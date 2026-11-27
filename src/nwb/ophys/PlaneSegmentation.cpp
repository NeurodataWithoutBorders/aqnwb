#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <type_traits>
#include <variant>

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
  // The schema requires at least one of the mask columns
  const bool hasMaskColumn = pixelMask || voxelMask
      || std::any_of(columns.begin(),
                     columns.end(),
                     [](const DataSpecPtr& column)
                     {
                       return column != nullptr
                           && (column->name == "image_mask"
                               || column->name == "pixel_mask"
                               || column->name == "voxel_mask");
                     });
  if (!hasMaskColumn) {
    std::cerr << "PlaneSegmentation::Spec::validate: At least one of the "
                 "image_mask, pixel_mask, or voxel_mask columns is required."
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

AQNWB::IO::BaseDataType PlaneSegmentation::getPixelMaskType()
{
  return IO::BaseDataType::Compound(
      {{"x", IO::BaseDataType::T_U32, offsetof(PixelMaskEntry, x)},
       {"y", IO::BaseDataType::T_U32, offsetof(PixelMaskEntry, y)},
       {"weight", IO::BaseDataType::T_F32, offsetof(PixelMaskEntry, weight)}},
      sizeof(PixelMaskEntry));
}

AQNWB::IO::BaseDataType PlaneSegmentation::getVoxelMaskType()
{
  return IO::BaseDataType::Compound(
      {{"x", IO::BaseDataType::T_U32, offsetof(VoxelMaskEntry, x)},
       {"y", IO::BaseDataType::T_U32, offsetof(VoxelMaskEntry, y)},
       {"z", IO::BaseDataType::T_U32, offsetof(VoxelMaskEntry, z)},
       {"weight", IO::BaseDataType::T_F32, offsetof(VoxelMaskEntry, weight)}},
      sizeof(VoxelMaskEntry));
}

namespace
{
/**
 * @brief Create the specs of a ragged mask column and its index column.
 */
std::vector<PlaneSegmentation::DataSpecPtr> createRaggedMaskDataSpecs(
    const std::string& tablePath,
    const std::string& maskName,
    const AQNWB::IO::BaseDataType& type,
    const std::string& description,
    const SizeType chunkSize)
{
  return {VectorData::createDataSpec(
              maskName,
              AQNWB::IO::ArrayDataSetConfig(type, {0}, {chunkSize}),
              description),
          std::make_shared<VectorIndex::DataSpec>(
              maskName + "_index",
              AQNWB::IO::ArrayDataSetConfig(
                  AQNWB::IO::BaseDataType::U32, {0}, {100}),
              "Index into " + maskName + ".",
              AQNWB::mergePaths(tablePath, maskName))};
}
}  // namespace

std::vector<PlaneSegmentation::DataSpecPtr>
PlaneSegmentation::createPixelMaskDataSpecs(const std::string& tablePath,
                                            const SizeType chunkSize)
{
  return createRaggedMaskDataSpecs(
      tablePath,
      "pixel_mask",
      getPixelMaskType(),
      "Pixel masks for each ROI: a list of indices and weights for the ROI.",
      chunkSize);
}

std::vector<PlaneSegmentation::DataSpecPtr>
PlaneSegmentation::createVoxelMaskDataSpecs(const std::string& tablePath,
                                            const SizeType chunkSize)
{
  return createRaggedMaskDataSpecs(
      tablePath,
      "voxel_mask",
      getVoxelMaskType(),
      "Voxel masks for each ROI: a list of indices and weights for the ROI.",
      chunkSize);
}

Status PlaneSegmentation::validateDataSpecs(
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  Status status = DynamicTable::validateDataSpecs(dataSpecs);
  for (const auto& dataSpec : dataSpecs) {
    if (dataSpec != nullptr
        && (dataSpec->name == "pixel_mask" || dataSpec->name == "voxel_mask"))
    {
      const IO::BaseDataType expectedType = dataSpec->name == "pixel_mask"
          ? getPixelMaskType()
          : getVoxelMaskType();
      if (!(dataSpec->getType() == expectedType)
          || dataSpec->getShape().size() != 1)
      {
        std::cerr << "PlaneSegmentation::validateDataSpecs: " << dataSpec->name
                  << " must be a 1D column of the compound type of "
                     "getPixelMaskType() or getVoxelMaskType()."
                  << std::endl;
        status = Status::Failure;
      }
      continue;
    }
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
  // Add the pixel_mask and voxel_mask columns if requested
  auto hasColumn = [&tableSpec](const std::string& name)
  {
    return std::any_of(tableSpec.columns.begin(),
                       tableSpec.columns.end(),
                       [&name](const DataSpecPtr& column)
                       { return column != nullptr && column->name == name; });
  };
  if (spec.pixelMask && !hasColumn("pixel_mask")) {
    auto maskSpecs = createPixelMaskDataSpecs(m_path);
    tableSpec.columns.insert(
        tableSpec.columns.end(), maskSpecs.begin(), maskSpecs.end());
  }
  if (spec.voxelMask && !hasColumn("voxel_mask")) {
    auto maskSpecs = createVoxelMaskDataSpecs(m_path);
    tableSpec.columns.insert(
        tableSpec.columns.end(), maskSpecs.begin(), maskSpecs.end());
  }
  // Check the link targets before anything is written
  if (checkLinkTarget(spec.imagingPlane, "imagingPlane") != Status::Success) {
    return Status::Failure;
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

Status PlaneSegmentation::addPixelMasks(
    const std::vector<std::vector<PixelMaskEntry>>& masks,
    const std::vector<int>& ids)
{
  std::vector<RoiData> rois(masks.size());
  for (size_t i = 0; i < masks.size(); ++i) {
    rois[i].pixelMask = masks[i];
  }
  return addRois(rois, ids);
}

Status PlaneSegmentation::addVoxelMasks(
    const std::vector<std::vector<VoxelMaskEntry>>& masks,
    const std::vector<int>& ids)
{
  std::vector<RoiData> rois(masks.size());
  for (size_t i = 0; i < masks.size(); ++i) {
    rois[i].voxelMask = masks[i];
  }
  return addRois(rois, ids);
}

namespace
{
/**
 * @brief Get the number of entries of a ragged mask column (or 0 if the table
 * has no such column).
 */
AQNWB::Types::SizeType getNumMaskEntries(
    const std::shared_ptr<AQNWB::NWB::VectorData>& maskColumn)
{
  if (maskColumn == nullptr) {
    return 0;
  }
  const auto shape = maskColumn->recordData()->getShape();
  return shape.empty() ? 0 : shape[0];
}
}  // namespace

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

  // Check the masks of all ROIs before anything is written. Each ROI must have
  // a mask for each mask column of the table, so that all columns keep the
  // same number of rows.
  auto imageMask = getConfiguredColumn("image_mask");
  auto pixelMask = getConfiguredColumn("pixel_mask");
  auto voxelMask = getConfiguredColumn("voxel_mask");
  SizeType numPixelEntries = 0;
  SizeType numVoxelEntries = 0;
  for (const auto& roi : rois) {
    if ((imageMask != nullptr) != (roi.imageMask != nullptr)
        || (pixelMask != nullptr) != roi.pixelMask.has_value()
        || (voxelMask != nullptr) != roi.voxelMask.has_value())
    {
      std::cerr << "PlaneSegmentation::addRois: Each ROI must have a mask for "
                   "each mask column of the table (image_mask, pixel_mask, "
                   "voxel_mask), and no other masks."
                << std::endl;
      return Status::Failure;
    }
    numPixelEntries += roi.pixelMask.has_value() ? roi.pixelMask->size() : 0;
    numVoxelEntries += roi.voxelMask.has_value() ? roi.voxelMask->size() : 0;
  }
  // The index of the ragged masks is uint32
  const SizeType maxEntries = std::numeric_limits<uint32_t>::max();
  if (getNumMaskEntries(pixelMask) + numPixelEntries > maxEntries
      || getNumMaskEntries(voxelMask) + numVoxelEntries > maxEntries)
  {
    std::cerr << "PlaneSegmentation::addRois: The number of mask entries "
                 "exceeds the range of the uint32 index."
              << std::endl;
    return Status::Failure;
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
  if (addRowsExcept(rows,
                    ids,
                    {"image_mask",
                     "pixel_mask",
                     "pixel_mask_index",
                     "voxel_mask",
                     "voxel_mask_index"})
      != Status::Success)
  {
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
  if (pixelMask != nullptr) {
    std::vector<PixelMaskEntry> entries;
    std::vector<SizeType> maskSizes;
    for (const auto& roi : rois) {
      entries.insert(
          entries.end(), roi.pixelMask->begin(), roi.pixelMask->end());
      maskSizes.push_back(roi.pixelMask->size());
    }
    status = status
        && writeRaggedMasks(
                 "pixel_mask", getPixelMaskType(), entries.data(), maskSizes);
  }
  if (voxelMask != nullptr) {
    std::vector<VoxelMaskEntry> entries;
    std::vector<SizeType> maskSizes;
    for (const auto& roi : rois) {
      entries.insert(
          entries.end(), roi.voxelMask->begin(), roi.voxelMask->end());
      maskSizes.push_back(roi.voxelMask->size());
    }
    status = status
        && writeRaggedMasks(
                 "voxel_mask", getVoxelMaskType(), entries.data(), maskSizes);
  }
  return status;
}

Status PlaneSegmentation::writeRaggedMasks(
    const std::string& maskName,
    const IO::BaseDataType& type,
    const void* entries,
    const std::vector<SizeType>& maskSizes)
{
  auto maskColumn = getConfiguredColumn(maskName);
  auto indexColumn = getConfiguredColumn(maskName + "_index");
  if (maskColumn == nullptr || indexColumn == nullptr) {
    std::cerr << "PlaneSegmentation: The table has no " << maskName
              << " column." << std::endl;
    return Status::Failure;
  }
  const SizeType numRois = maskSizes.size();
  if (numRois == 0) {
    return Status::Success;
  }

  // Append the entries of the masks
  auto maskRecorder = maskColumn->recordData();
  const SizeArray maskShape = maskRecorder->getShape();
  const SizeType numEntries = maskShape.empty() ? 0 : maskShape[0];
  const SizeType numNewEntries =
      std::accumulate(maskSizes.begin(), maskSizes.end(), SizeType {0});
  Status status = Status::Success;
  if (numNewEntries > 0) {
    status = maskRecorder->writeDataBlock(
        {numNewEntries}, {numEntries}, type, entries);
  }

  // Append the index, i.e., the end of each mask in the mask column (the
  // range of the uint32 index is checked by addRois)
  std::vector<uint32_t> indexValues(numRois);
  SizeType end = numEntries;
  for (SizeType i = 0; i < numRois; ++i) {
    end += maskSizes[i];
    indexValues[i] = static_cast<uint32_t>(end);
  }
  auto indexRecorder = indexColumn->recordData();
  const SizeArray indexShape = indexRecorder->getShape();
  const SizeType numIndexRows = indexShape.empty() ? 0 : indexShape[0];
  status = status
      && indexRecorder->writeDataBlock(
          {numRois}, {numIndexRows}, IO::BaseDataType::U32, indexValues.data());
  return status;
}
