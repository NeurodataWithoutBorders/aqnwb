#include "nwb/epoch/TimeIntervals.hpp"

#include "Utils.hpp"
#include "nwb/NWBFile.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(TimeIntervals)

// Constructor
TimeIntervals::TimeIntervals(const std::string& path,
                             std::shared_ptr<AQNWB::IO::BaseIO> io)
    : DynamicTable(path, io)
{
}

// TODO add addTimeseries parameter to also support construction of the
// timeseries and timeseries_index columns
std::vector<DynamicTable::DataSpecPtr> TimeIntervals::createDefaultDataSpecs(
    const std::string& timeIntervalsPath,
    const SizeType rowChunkSize,
    const bool addTagsColumn)
{
  std::vector<DataSpecPtr> specs =
      DynamicTable::createDefaultDataSpecs(rowChunkSize);

  // Add the required start_time column
  IO::ArrayDataSetConfig startTimeConfig(
      IO::BaseDataType::T_F32, SizeArray {0}, SizeArray {rowChunkSize});
  specs.push_back(std::make_shared<VectorData::DataSpec>(
      "start_time", startTimeConfig, "Start time of epoch, in seconds"));

  // Add the required stop_time column
  IO::ArrayDataSetConfig stopTimeConfig(
      IO::BaseDataType::T_F32, SizeArray {0}, SizeArray {rowChunkSize});
  specs.push_back(std::make_shared<VectorData::DataSpec>(
      "stop_time", stopTimeConfig, "Stop time of epoch, in seconds"));

  // add the tags and tags_index columns if requested
  if (addTagsColumn) {
    // add the tags column spec
    IO::ArrayDataSetConfig tagsConfig(
        IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {rowChunkSize});
    specs.push_back(std::make_shared<VectorData::DataSpec>(
        "tags",
        tagsConfig,
        "User-defined tags that identify or categorize events"));

    // add the tags_index column
    IO::ArrayDataSetConfig tagsIndexConfig(
        IO::BaseDataType::T_U32, SizeArray {0}, SizeArray {rowChunkSize});
    specs.push_back(std::make_shared<VectorIndex::DataSpec>(
        "tags_index",
        tagsIndexConfig,
        "Index for user-defined tags that identify or categorize events",
        AQNWB::mergePaths(timeIntervalsPath, "tags")));
  }

  // TODO add the timeseries and timeseries_index columns
  // if (addTimeseriesColumn)
  // {
  //    // TODO Add the timeseries TimeSeriesReferenceVectorData column
  //    // TODO Add the timesries_index VectorIndex column
  // }

  return specs;
}

Status TimeIntervals::validateDataSpecs(
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  Status status = DynamicTable::validateDataSpecs(dataSpecs);
  if (status != Status::Success) {
    return status;
  }

  status = checkRequiredColumnSpec<VectorData::DataSpec>(
      "start_time", dataSpecs, IO::BaseDataType::F32);
  if (status != Status::Success) {
    return status;
  }

  return checkRequiredColumnSpec<VectorData::DataSpec>(
      "stop_time", dataSpecs, IO::BaseDataType::F32);
}

// Initialize the object
Status TimeIntervals::initialize(const std::string& description,
                                 const std::vector<DataSpecPtr>& columnSpecs)
{
  std::vector<DataSpecPtr> specsToUse = columnSpecs;
  if (specsToUse.empty()) {
    specsToUse = createDefaultDataSpecs(this->getPath());
  }

  // Retrieve the IO object
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr << "IO object has been deleted. Can't initialize TimeIntervals: "
              << m_path << std::endl;
    return Status::Failure;
  }

  // Ensure the intervals group exists if this table is being created in the
  // intervals group
  if (AQNWB::isPathOrDescendant(m_path, NWBFile::INTERVALS_PATH)) {
    Status requireStatus = NWBFile::requireIntervalsGroup(ioPtr);
    if (requireStatus != Status::Success) {
      std::cerr
          << "Failed to create or verify intervals group for TimeIntervals: "
          << m_path << std::endl;
      return Status::Failure;
    }
  }

  // create group. This configures the "start_time" and "stop_time" columns
  // (among others) via the DataSpec mechanism, creating and registering the
  // corresponding VectorData recording objects exactly once.
  Status dtStatus = DynamicTable::initialize(description, specsToUse);

  return dtStatus;
}
