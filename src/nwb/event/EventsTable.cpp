#include "nwb/event/EventsTable.hpp"

#include "Utils.hpp"
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
    float timestampResolution,
    float durationResolution,
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
      "A 1-dimensional VectorData that stores timestamps in seconds from "
      "thesession start time. Timestamp are not required to be sorted in time.",
      timestampResolution));

  if (durationResolution >= 0.0f) {
    IO::ArrayDataSetConfig durationConfig(
        IO::BaseDataType::F32, SizeArray {0}, SizeArray {rowChunkSize});
    specs.push_back(std::make_shared<DurationVectorData::DataSpec>(
        "duration",
        durationConfig,
        "A 1-dimensional VectorData that stores the durations of the events in "
        "seconds. Durations are not required to be sorted in time.",
        durationResolution));
  }

  if (createAnnotationColumn) {
    IO::ArrayDataSetConfig annotationConfig(
        IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {rowChunkSize});
    specs.push_back(std::make_shared<VectorData::DataSpec>(
        "annotation",
        annotationConfig,
        "A 1-dimensional VectorData that stores annotations for the events. "
        "Annotations are not required to be sorted in time."));
  }

  return specs;
}

// Initialize the object
Status EventsTable::initialize(const std::string& description,
                               const std::string& sourceDescription,
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
    // If no specs provided, we can't create the default ones because we don't
    // have the resolutions. This is a limitation of the current API design
    // where we removed the resolutions from initialize.
    // For now, we'll just use the default DynamicTable specs.
    specsToUse = DynamicTable::createDefaultDataSpecs();
  }

  // Call parent initialize method.
  Status parentInitStatus = DynamicTable::initialize(description, specsToUse);

  // Initialize attributes, datasets, and groups
  // Create the source_description attribute if provided
  Status sourceDescStatus = Status::Success;
  if (!sourceDescription.empty()) {
    sourceDescStatus =
        ioPtr->createAttribute(sourceDescription, m_path, "source_description");
  }

  // Combine all statuses and return the final status
  initStatus = initStatus && parentInitStatus && sourceDescStatus;
  return initStatus;
}
