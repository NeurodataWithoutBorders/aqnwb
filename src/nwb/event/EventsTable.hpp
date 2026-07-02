#pragma once

// Common STL includes
#include <memory>
#include <optional>
#include <string>
#include <vector>
// Base AqNWB includes for IO and RegisteredType
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/RegisteredType.hpp"
// Include for parent type
#include "nwb/event/DurationVectorData.hpp"
#include "nwb/event/TimestampVectorData.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"

// Include for the namespace schema header
#include "spec/core.hpp"

namespace AQNWB::NWB
{

/**
 * @brief A column-based table to store information about events, one event per
 * row. Use `EventsTable` when each row is anchored at a single timestamp and
 * duration is absent, optional, or mixed across rows. Additional columns may be
 * added to store metadata about each event, such as the duration of the event.
 * Examples include TTL pulses, licks, rewards, stimulus onsets, and detected
 * ripples. Each `EventsTable` should hold events of a single type, so that all
 * rows share the same set of per-event metadata columns. Events of different
 * types (e.g., licks and stimulus presentations) should be stored in separate
 * `EventsTable` instances.
 */
class EventsTable : public AQNWB::NWB::DynamicTable
{
public:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  EventsTable(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~EventsTable() override {}

  /**
   * @brief Initialize the object
   * @param description Description of the table
   * @param sourceDescription Optional short text description of where the
   * events came from, applying to every row in the table. If empty string is
   * provided (default), then the attribute will not be created.
   * @param timestampResolution The temporal resolution of the timestamps - in
   * seconds. See `TimestampVectorData::initialize()` for more details.
   * @param durationResolution The temporal resolution of the optional duration
   * column in seconds. See `DurationVectorData::initialize()` for more details.
   * If a negative value is provided (default), then the duration column will
   * not be created.
   * @param createAnnotationColumn Whether to create the annotation column
   * (default: false)
   * @param rowChunkSize The chunk size for the rows of the table (optional,
   * default: 100)
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::string& description,
                    const std::string& sourceDescription,
                    float timestampResolution,
                    float durationResolution = -1.0f,
                    const bool createAnnotationColumn = false,
                    const SizeType rowChunkSize = 100);

  // Define read methods
  // description overrides inherited field from parent neurodata_type
  DEFINE_ATTRIBUTE_FIELD(
      readDescription,
      std::string,
      "description",
      "A description of the events stored in the table - including information "
      "about how the event times were computed - especially if the times are "
      "the result of processing or filtering raw data. For example - if the "
      "experimenter is encoding different types of events using a strobed or "
      "N-bit encoding - then the description should describe which channels "
      "were used and how the event time is computed - e.g. - as the rise time "
      "of the first bit.")

  DEFINE_ATTRIBUTE_FIELD(
      readSourceDescription,
      std::string,
      "source_description",
      "Optional short text description of where the events came from - "
      "applying to every row in the table. For example - Acquisition system "
      "for events emitted directly by the acquisition system (e.g. - TTL edges "
      "or hardware event channels); Thresholding of analog signal ANALOG1 at 3 "
      "V for events produced by a detection algorithm run on acquired data; or "
      "Manual video review for events added by a human annotator. This is a "
      "free-text label of origin only; use `description` for the longer "
      "narrative of how the event times were computed (channels used - "
      "encoding scheme - algorithm parameters - etc.).")

  DEFINE_REGISTERED_FIELD(readTimestampColumn,
                          AQNWB::NWB::TimestampVectorData,
                          "timestamp",
                          "Column containing the time that each event occurred "
                          "- in seconds - from the session start time.")

  DEFINE_REGISTERED_FIELD(
      readDurationColumn,
      AQNWB::NWB::DurationVectorData,
      "duration",
      "Optional column containing the duration of each event - in seconds. A "
      "value of NaN can be used for events without a duration or with a "
      "duration that is not yet specified.")

  DEFINE_REGISTERED_FIELD(readAnnotationColumn,
                          AQNWB::NWB::VectorData,
                          "annotation",
                          "Column containing user annotations about events.")

  REGISTER_SUBCLASS(EventsTable, DynamicTable, AQNWB::SPEC::CORE::namespaceName)
};

}  // namespace AQNWB::NWB
