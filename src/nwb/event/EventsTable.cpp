#include "nwb/event/EventsTable.hpp"

#include "Utils.hpp"
#include "nwb/NWBFile.hpp"
// Includes for referenced types
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/MeaningsTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(EventsTable)

// Constructor
EventsTable::EventsTable(const std::string& path,
                         std::shared_ptr<AQNWB::IO::BaseIO> io)
    : DynamicTable(path, io)
{
}

Status EventsTable::validateDataSpecs(
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  return checkRequiredColumnNames({"id", "timestamp"}, dataSpecs);
}

std::vector<DynamicTable::DataSpecPtr> EventsTable::createDefaultDataSpecs(
    std::optional<float> timestampResolution,
    bool createDurationColumn,
    std::optional<float> durationResolution,
    const bool createAnnotationColumn,
    const SizeType rowChunkSize)
{
  std::vector<DataSpecPtr> specs =
      DynamicTable::createDefaultDataSpecs(rowChunkSize);

  IO::ArrayDataSetConfig timestampConfig(
      IO::BaseDataType::F32, SizeArray {0}, SizeArray {rowChunkSize});
  specs.push_back(std::make_shared<TimestampVectorData::DataSpec>(
      "timestamp",
      timestampConfig,
      "Column containing the time that each event occurred "
      "- in seconds - from the session start time.",
      timestampResolution));

  if (createDurationColumn) {
    IO::ArrayDataSetConfig durationConfig(
        IO::BaseDataType::F32, SizeArray {0}, SizeArray {rowChunkSize});
    specs.push_back(std::make_shared<DurationVectorData::DataSpec>(
        "duration",
        durationConfig,
        "Optional column containing the duration of each event - in seconds. A "
        "value of NaN can be used for events without a duration or with a "
        "duration that is not yet specified.",
        durationResolution));
  }

  if (createAnnotationColumn) {
    IO::ArrayDataSetConfig annotationConfig(
        IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {rowChunkSize});
    specs.push_back(std::make_shared<VectorData::DataSpec>(
        "annotation",
        annotationConfig,
        "Column containing user annotations about events."));
  }

  return specs;
}

// Initialize the object
Status EventsTable::initialize(
    const std::string& description,
    const std::optional<std::string>& sourceDescription,
    const std::vector<DataSpecPtr>& columnSpecs)
{
  Status initStatus = Status::Success;

  // Retrieve the IO object
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr << "IO object has been deleted. Can't initialize EventsTable: "
              << m_path << std::endl;
    return Status::Failure;
  }

  std::vector<DataSpecPtr> specsToUse = columnSpecs;
  if (specsToUse.empty()) {
    specsToUse = createDefaultDataSpecs();
  }

  // Ensure the events group exists if this table is being created in the events
  // group
  if (m_path.find(NWBFile::EVENTS_PATH) == 0) {
    Status requireStatus = NWBFile::requireEventsGroup(ioPtr);
    if (requireStatus != Status::Success) {
      std::cerr << "Failed to create or verify events group for EventsTable: "
                << m_path << std::endl;
      return Status::Failure;
    }
  }

  // Call parent initialize method.
  Status parentInitStatus = DynamicTable::initialize(description, specsToUse);

  // Initialize attributes, datasets, and groups
  // Create the source_description attribute if provided
  Status sourceDescStatus = Status::Success;
  if (sourceDescription.has_value() && !sourceDescription.value().empty()) {
    sourceDescStatus = ioPtr->createAttribute(
        sourceDescription.value(), m_path, "source_description");
  }

  // Combine all statuses and return the final status
  initStatus = initStatus && parentInitStatus && sourceDescStatus;
  return initStatus;
}
