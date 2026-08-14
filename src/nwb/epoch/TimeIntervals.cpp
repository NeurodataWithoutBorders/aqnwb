#include "nwb/epoch/TimeIntervals.hpp"

#include "Utils.hpp"

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
    const SizeType rowChunkSize, const bool addTagsColumn)
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

    // TODO: add the tags_index column
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
  return checkRequiredColumnNames({"id", "start_time", "stop_time"}, dataSpecs);
  // TODO Also add support for checking dtype of named columns defined in the
  // schema
}

// Initialize the object
Status TimeIntervals::initialize(const std::string& description,
                                 const std::vector<DataSpecPtr>& columnSpecs)
{
  std::vector<DataSpecPtr> specsToUse = columnSpecs;
  if (specsToUse.empty()) {
    specsToUse = createDefaultDataSpecs();
  }

  // create group. This configures the "start_time" and "stop_time" columns
  // (among others) via the DataSpec mechanism, creating and registering the
  // corresponding VectorData recording objects exactly once.
  Status dtStatus = DynamicTable::initialize(description, specsToUse);

  return dtStatus;
}
