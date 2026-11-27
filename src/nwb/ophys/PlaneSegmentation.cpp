#include <numeric>

#include "nwb/ophys/PlaneSegmentation.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// PlaneSegmentation
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(PlaneSegmentation)

/** Constructor */
PlaneSegmentation::PlaneSegmentation(const std::string& path,
                                     std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(path, io)
{
}

/** Destructor */
PlaneSegmentation::~PlaneSegmentation() {}

Status PlaneSegmentation::Spec::validate() const
{
  Status status = DynamicTable::Spec::validate();
  if (description.empty()) {
    std::cerr << "PlaneSegmentation::Spec::validate: description must not be "
                 "empty."
              << std::endl;
    status = Status::Failure;
  }
  if (imagingPlane == nullptr) {
    std::cerr << "PlaneSegmentation::Spec::validate: imagingPlane must be set."
              << std::endl;
    status = Status::Failure;
  }
  // The schema requires a mask column
  const bool hasMaskColumn = std::any_of(
      columns.begin(),
      columns.end(),
      [](const DataSpecPtr& column)
      { return column != nullptr && column->name == "image_mask"; });
  if (!hasMaskColumn) {
    std::cerr << "PlaneSegmentation::Spec::validate: The image_mask column is "
                 "required (see createImageMaskDataSpec)."
              << std::endl;
    status = Status::Failure;
  }
  return status;
}

std::vector<PlaneSegmentation::DataSpecPtr>
PlaneSegmentation::createDefaultDataSpecs(const SizeType rowChunkSize)
{
  return DynamicTable::createDefaultDataSpecs(rowChunkSize);
}

PlaneSegmentation::DataSpecPtr PlaneSegmentation::createImageMaskDataSpec(
    const SizeArray& maskShape)
{
  SizeArray shape = {0};
  shape.insert(shape.end(), maskShape.begin(), maskShape.end());
  SizeArray chunking = {1};
  chunking.insert(chunking.end(), maskShape.begin(), maskShape.end());
  return VectorData::createDataSpec(
      "image_mask",
      IO::ArrayDataSetConfig(IO::BaseDataType::F32, shape, chunking),
      "ROI masks for each ROI. Each image mask is the size of the original "
      "imaging plane (or volume) and members of the ROI are finite non-zero.");
}

Status PlaneSegmentation::validateDataSpecs(
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  Status status = DynamicTable::validateDataSpecs(dataSpecs);
  for (const auto& dataSpec : dataSpecs) {
    if (dataSpec == nullptr || dataSpec->name != "image_mask") {
      continue;
    }
    const SizeArray& shape = dataSpec->getShape();
    if (std::dynamic_pointer_cast<VectorData::DataSpec>(dataSpec) == nullptr
        || (shape.size() != 3 && shape.size() != 4)
        || dataSpec->getType().type != IO::BaseDataType::Type::T_F32)
    {
      std::cerr << "PlaneSegmentation::validateDataSpecs: image_mask must be a "
                   "3D or 4D float32 VectorData column."
                << std::endl;
      status = Status::Failure;
    }
  }
  return status;
}

Status PlaneSegmentation::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "PlaneSegmentation::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  // Validate the spec and initialize the table with the default columns if no
  // columns are specified
  Spec tableSpec = spec;
  if (tableSpec.columns.empty()) {
    tableSpec.columns = createDefaultDataSpecs();
  }
  Status status = DynamicTable::initialize(tableSpec);
  if (status != Status::Success) {
    return Status::Failure;
  }

  // The reference_images group is required by the schema. It stores the
  // ImageSeries used as references for segmentation (if any).
  status = status
      && ioPtr->createGroup(AQNWB::mergePaths(m_path, "reference_images"));
  status = status
      && ioPtr->createLink(
          AQNWB::mergePaths("/" + m_path, "imaging_plane"),
          AQNWB::mergePaths("/", spec.imagingPlane->getPath()));
  return status;
}

Status PlaneSegmentation::addImageMasks(const SizeType& numRois,
                                        const float* masks,
                                        const std::vector<int>& ids)
{
  if (numRois > 0 && masks == nullptr) {
    std::cerr << "PlaneSegmentation::addImageMasks: masks must not be null."
              << std::endl;
    return Status::Failure;
  }
  auto imageMask = getConfiguredColumn("image_mask");
  if (imageMask == nullptr) {
    std::cerr << "PlaneSegmentation::addImageMasks: The table has no "
                 "image_mask column."
              << std::endl;
    return Status::Failure;
  }
  // Number of weights of a mask, i.e., the product of the mask shape
  const SizeArray maskShape = imageMask->recordData()->getShape();
  SizeType maskSize = 1;
  for (size_t i = 1; i < maskShape.size(); ++i) {
    maskSize *= maskShape[i];
  }
  std::vector<RoiData> rois(numRois);
  for (SizeType i = 0; i < numRois; ++i) {
    rois[i].imageMask = masks + i * maskSize;
  }
  return addRois(rois, ids);
}

Status PlaneSegmentation::addRois(const std::vector<RoiData>& rois,
                                  const std::vector<int>& ids)
{
  if (rois.empty()) {
    return Status::Success;
  }
  if (!ids.empty() && ids.size() != rois.size()) {
    std::cerr << "PlaneSegmentation::addRois: The number of ids must match "
                 "the number of ROIs."
              << std::endl;
    return Status::Failure;
  }
  // Check the masks of all ROIs before anything is written
  auto imageMask = getConfiguredColumn("image_mask");
  for (const auto& roi : rois) {
    if ((imageMask != nullptr) != (roi.imageMask != nullptr)) {
      std::cerr << "PlaneSegmentation::addRois: Each ROI must have an image "
                   "mask if (and only if) the table has an image_mask column."
                << std::endl;
      return Status::Failure;
    }
  }
  SizeArray maskShape;
  if (imageMask != nullptr) {
    maskShape = imageMask->recordData()->getShape();
    if (maskShape.empty()) {
      return Status::Failure;
    }
  }

  // Write the ids and the other columns (which also checks their values), and
  // then the masks after the existing rows
  const SizeType numRows = getNumberOfRows();
  std::vector<Types::RowData> rows;
  rows.reserve(rois.size());
  for (const auto& roi : rois) {
    rows.push_back(roi.columns);
  }
  if (addRowsExcept(rows, ids, {"image_mask"}) != Status::Success) {
    return Status::Failure;
  }
  Status status = Status::Success;
  if (imageMask != nullptr) {
    // The masks may not be contiguous, so write them one ROI at a time
    auto maskRecorder = imageMask->recordData();
    SizeArray blockShape = maskShape;
    blockShape[0] = 1;
    SizeArray positionOffset(maskShape.size(), 0);
    for (size_t i = 0; i < rois.size(); ++i) {
      positionOffset[0] = numRows + i;
      status = status
          && maskRecorder->writeDataBlock(blockShape,
                                          positionOffset,
                                          IO::BaseDataType::F32,
                                          rois[i].imageMask);
    }
  }
  return status;
}
