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
  auto imageMask = getConfiguredColumn("image_mask");
  if (imageMask == nullptr) {
    std::cerr << "PlaneSegmentation::addImageMasks: The table has no "
                 "image_mask column."
              << std::endl;
    return Status::Failure;
  }
  if (!ids.empty() && ids.size() != numRois) {
    std::cerr << "PlaneSegmentation::addImageMasks: The number of ids must "
                 "match the number of ROIs."
              << std::endl;
    return Status::Failure;
  }

  // Append the masks after the existing rows
  auto maskRecorder = imageMask->recordData();
  SizeArray maskShape = maskRecorder->getShape();
  if (maskShape.empty()) {
    return Status::Failure;
  }
  const SizeType numRows = getNumberOfRows();
  SizeArray blockShape = maskShape;
  blockShape[0] = numRois;
  SizeArray positionOffset(maskShape.size(), 0);
  positionOffset[0] = numRows;
  Status status = maskRecorder->writeDataBlock(
      blockShape, positionOffset, IO::BaseDataType::F32, masks);

  // Append the ids of the ROIs
  std::vector<int> roiIds = ids;
  if (roiIds.empty()) {
    roiIds.resize(numRois);
    std::iota(roiIds.begin(), roiIds.end(), static_cast<int>(numRows));
  }
  status = status && setRowIDs(roiIds);
  return status;
}
