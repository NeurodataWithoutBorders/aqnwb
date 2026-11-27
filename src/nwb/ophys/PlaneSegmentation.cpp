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

Status PlaneSegmentation::checkSingleMaskType(const std::string& maskName) const
{
  // The add*Masks methods append rows to one mask column and to the id
  // column. With several mask columns, the other mask columns would get out
  // of sync with the id column.
  for (const std::string& name : {"image_mask", "pixel_mask", "voxel_mask"}) {
    if (name != maskName && getConfiguredColumn(name) != nullptr) {
      std::cerr << "PlaneSegmentation: Adding " << maskName
                << " rows to a table that also has a " << name
                << " column is not supported, since all columns must have "
                   "the same number of rows."
                << std::endl;
      return Status::Failure;
    }
  }
  return Status::Success;
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
  if (numRois > 0 && masks == nullptr) {
    std::cerr << "PlaneSegmentation::addImageMasks: masks must not be null."
              << std::endl;
    return Status::Failure;
  }
  if (checkSingleMaskType("image_mask") != Status::Success) {
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

Status PlaneSegmentation::addPixelMasks(
    const std::vector<std::vector<PixelMaskEntry>>& masks,
    const std::vector<int>& ids)
{
  std::vector<PixelMaskEntry> entries;
  std::vector<SizeType> maskSizes;
  for (const auto& mask : masks) {
    entries.insert(entries.end(), mask.begin(), mask.end());
    maskSizes.push_back(mask.size());
  }
  return addRaggedMasks(
      "pixel_mask", getPixelMaskType(), entries.data(), maskSizes, ids);
}

Status PlaneSegmentation::addVoxelMasks(
    const std::vector<std::vector<VoxelMaskEntry>>& masks,
    const std::vector<int>& ids)
{
  std::vector<VoxelMaskEntry> entries;
  std::vector<SizeType> maskSizes;
  for (const auto& mask : masks) {
    entries.insert(entries.end(), mask.begin(), mask.end());
    maskSizes.push_back(mask.size());
  }
  return addRaggedMasks(
      "voxel_mask", getVoxelMaskType(), entries.data(), maskSizes, ids);
}

Status PlaneSegmentation::addRaggedMasks(const std::string& maskName,
                                         const IO::BaseDataType& type,
                                         const void* entries,
                                         const std::vector<SizeType>& maskSizes,
                                         const std::vector<int>& ids)
{
  auto maskColumn = getConfiguredColumn(maskName);
  auto indexColumn = getConfiguredColumn(maskName + "_index");
  if (maskColumn == nullptr || indexColumn == nullptr) {
    std::cerr << "PlaneSegmentation::add" << maskName << "s: The table has no "
              << maskName << " column." << std::endl;
    return Status::Failure;
  }
  if (checkSingleMaskType(maskName) != Status::Success) {
    return Status::Failure;
  }
  const SizeType numRois = maskSizes.size();
  if (!ids.empty() && ids.size() != numRois) {
    std::cerr << "PlaneSegmentation: The number of ids must match the number "
                 "of ROIs."
              << std::endl;
    return Status::Failure;
  }
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

  // Append the index, i.e., the end of each mask in the mask column
  std::vector<uint32_t> indexValues(numRois);
  SizeType end = numEntries;
  for (SizeType i = 0; i < numRois; ++i) {
    end += maskSizes[i];
    if (end > std::numeric_limits<uint32_t>::max()) {
      std::cerr << "PlaneSegmentation: The number of " << maskName
                << " entries exceeds the range of the uint32 index."
                << std::endl;
      return Status::Failure;
    }
    indexValues[i] = static_cast<uint32_t>(end);
  }
  auto indexRecorder = indexColumn->recordData();
  const SizeArray indexShape = indexRecorder->getShape();
  const SizeType numIndexRows = indexShape.empty() ? 0 : indexShape[0];
  // Use the number of rows of the table for the default ids
  const SizeType numRows = getNumberOfRows();
  status = status
      && indexRecorder->writeDataBlock(
          {numRois}, {numIndexRows}, IO::BaseDataType::U32, indexValues.data());

  // Append the ids of the ROIs
  std::vector<int> roiIds = ids;
  if (roiIds.empty()) {
    roiIds.resize(numRois);
    std::iota(roiIds.begin(), roiIds.end(), static_cast<int>(numRows));
  }
  status = status && setRowIDs(roiIds);
  return status;
}

namespace
{
/**
 * @brief Read a ragged compound mask column and split it into one list of
 * entries per ROI.
 */
template<typename EntryT>
std::vector<std::vector<EntryT>> readRaggedMasks(
    const std::shared_ptr<AQNWB::IO::BaseIO>& io,
    const std::string& tablePath,
    const std::string& maskName,
    const AQNWB::IO::BaseDataType& entryType)
{
  std::vector<std::vector<EntryT>> masks;
  const std::string maskPath = AQNWB::mergePaths(tablePath, maskName);
  const std::string indexPath = maskPath + "_index";
  if (io == nullptr || !io->objectExists(maskPath)
      || !io->objectExists(indexPath))
  {
    return masks;
  }
  auto entries = AQNWB::IO::DataBlock<EntryT>::fromCompound(
      io->readDataset(maskPath), entryType);
  // The index may use any unsigned integer type (e.g., uint8 in files
  // written by hdmf), so convert it to SizeType
  std::vector<SizeType> ends;
  std::visit(
      [&ends](const auto& values)
      {
        using VectorT = std::decay_t<decltype(values)>;
        if constexpr (std::is_same_v<VectorT, std::monostate>) {
          throw std::invalid_argument(
              "PlaneSegmentation: unsupported data type of the mask index");
        } else if constexpr (std::is_integral_v<typename VectorT::value_type>) {
          for (const auto& value : values) {
            ends.push_back(static_cast<SizeType>(value));
          }
        } else {
          throw std::invalid_argument(
              "PlaneSegmentation: the mask index must "
              "use an integer data type");
        }
      },
      io->readDataset(indexPath).as_variant());
  SizeType start = 0;
  for (const SizeType end : ends) {
    if (end < start || end > entries.data.size()) {
      throw std::invalid_argument("PlaneSegmentation: invalid " + maskName
                                  + "_index");
    }
    masks.emplace_back(
        entries.data.begin() + static_cast<std::ptrdiff_t>(start),
        entries.data.begin() + static_cast<std::ptrdiff_t>(end));
    start = end;
  }
  return masks;
}
}  // namespace

std::vector<std::vector<PlaneSegmentation::PixelMaskEntry>>
PlaneSegmentation::readPixelMasks() const
{
  return readRaggedMasks<PixelMaskEntry>(
      getIO(), m_path, "pixel_mask", getPixelMaskType());
}

std::vector<std::vector<PlaneSegmentation::VoxelMaskEntry>>
PlaneSegmentation::readVoxelMasks() const
{
  return readRaggedMasks<VoxelMaskEntry>(
      getIO(), m_path, "voxel_mask", getVoxelMaskType());
}
