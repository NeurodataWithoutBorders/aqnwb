#include "nwb/event/EventsTable.hpp"

#include "Utils.hpp"
// Includes for referenced types
#include "nwb/event/DurationVectorData.hpp"
#include "nwb/event/TimestampVectorData.hpp"
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

// Initialize the object
Status EventsTable::initialize(const std::string& description,
                               const std::string& sourceDescription,
                               float timestampResolution,
                               float durationResolution,
                               const bool createAnnotationColumn,
                               const SizeType rowChunkSize)
{
  Status initStatus = Status::Success;

  // Retrieve the IO object
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr << "IO object has been deleted. Can't initialize EventsTable: "
              << m_path << std::endl;
    return Status::Failure;
  }

  // Call parent initialize method.
  Status parentInitStatus = DynamicTable::initialize(description);

  // Initialize attributes, datasets, and groups
  // Create the source_description attribute if provided
  Status sourceDescStatus = Status::Success;
  if (!sourceDescription.empty()) {
    sourceDescStatus =
        ioPtr->createAttribute(sourceDescription, m_path, "source_description");
  }

  // Initialize the required TimestampVectorData dataset
  auto timestampPath = AQNWB::mergePaths(m_path, "timestamp");
  IO::ArrayDataSetConfig timestampConfig(
      IO::BaseDataType::F32, SizeArray {0}, SizeArray {rowChunkSize});
  auto timestampColumn = TimestampVectorData::create(timestampPath, ioPtr);
  Status timestampStatus = timestampColumn->initialize(
      timestampConfig,
      "A 1-dimensional VectorData that stores timestamps in seconds from "
      "thesession start time. Timestamp are not required to be sorted in time.",
      timestampResolution);
  Status addTimestampColumnStatus = addColumn(timestampColumn);
  timestampStatus = timestampStatus && addTimestampColumnStatus;

  // Initialize the optional DurationVectorData dataset if requested
  Status durationStatus = Status::Success;
  if (durationResolution >= 0.0f) {
    auto durationPath = AQNWB::mergePaths(m_path, "duration");
    IO::ArrayDataSetConfig durationConfig(
        IO::BaseDataType::F32, SizeArray {0}, SizeArray {rowChunkSize});
    auto durationColumn = DurationVectorData::create(durationPath, ioPtr);
    durationStatus = durationColumn->initialize(
        durationConfig,
        "A 1-dimensional VectorData that stores the durations of the events in "
        "seconds. Durations are not required to be sorted in time.",
        durationResolution);
    Status addDurationColumnStatus = addColumn(durationColumn);
    durationStatus = durationStatus && addDurationColumnStatus;
  }

  // Initialize the optional annotation dataset
  Status annotationStatus = Status::Success;
  if (createAnnotationColumn) {
    auto annotationPath = AQNWB::mergePaths(m_path, "annotation");
    IO::ArrayDataSetConfig annotationConfig(
        IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {rowChunkSize});
    auto annotationColumn = VectorData::create(annotationPath, ioPtr);
    annotationStatus = annotationColumn->initialize(
        annotationConfig,
        "A 1-dimensional VectorData that stores annotations for the events. "
        "Annotations are not required to be sorted in time.");
    Status addAnnotationColumnStatus = addColumn(annotationColumn);
    annotationStatus = annotationStatus && addAnnotationColumnStatus;
  }

  // Combine all statuses and return the final status
  initStatus = initStatus && parentInitStatus && sourceDescStatus
      && timestampStatus && durationStatus && annotationStatus;
  return initStatus;
}
