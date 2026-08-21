#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "nwb/base/ProcessingModule.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/epoch/TimeIntervals.hpp"
#include "nwb/event/EventsTable.hpp"
#include "nwb/file/ElectrodesTable.hpp"
#include "spec/core.hpp"

/*!
 * \namespace AQNWB::NWB
 * \brief Namespace for all classes related to the NWB data standard
 */
namespace AQNWB::NWB
{

/**
 * @brief The NWBFile class provides an interface for setting up and managing
 * the NWB file.
 *
 * \note
 * **Handling of Optional Paths:**
 * Some groups in the NWB file structure (e.g., `/events`, `/intervals`) are
 * optional and are not created by default during `initialize()`. These groups
 * are created automatically on-demand when objects are added to them (e.g., via
 * `createEventsTable` or `createTimeIntervals`). If you are creating objects in
 * these paths manually, you can use helper methods like `requireEventsGroup` or
 * `requireIntervalsGroup` to ensure the parent groups exist.
 */
class NWBFile : public NWBContainer
{
public:
  // Register the NWBFile as a subclass of NWBContainer
  REGISTER_SUBCLASS(NWBFile, NWBContainer, AQNWB::SPEC::CORE::namespaceName)

  // Static paths for NWB file structure
  /// @brief The path to the acquisition group in the NWB file
  inline const static std::string ACQUISITION_PATH = "/acquisition";
  /// @brief The path to the specification group in the NWB file
  inline const static std::string SPECIFICATIONS_PATH = "/specifications";
  /// @brief The path to the processing group in the NWB file
  inline const static std::string PROCESSING_PATH = "/processing";
  /// @brief The path to the stimulus group in the NWB file
  inline const static std::string STIMULUS_PATH = "/stimulus";
  /// @brief The path to the general group in the NWB file
  inline const static std::string GENERAL_PATH = "/general";
  /// @brief The path to the analysis group in the NWB file
  inline const static std::string ANALYSIS_PATH = "/analysis";
  /// @brief The path to the root events group in the NWB file
  inline const static std::string EVENTS_PATH = "/events";
  /// @brief The path to the root intervals group in the NWB file
  inline const static std::string INTERVALS_PATH = "/intervals";

  /** \brief Convenience factor method since the path is fixed to '/'
   * @param io A shared pointer to the IO object.
   * @return A shared pointer to the created NWBFile object, or nullptr if
   * creation failed.
   */
  static std::shared_ptr<NWBFile> create(std::shared_ptr<IO::BaseIO> io)
  {
    return RegisteredType::create<NWBFile>("/", io);
  }

protected:
  /**
   * @brief Constructor for NWBFile class.
   * @param io The shared pointer to the IO object.
   */
  explicit NWBFile(std::shared_ptr<IO::BaseIO> io);

  /** @brief Required constructor so we can call RegisteredType::create but the
   * path cannot be set
   */
  NWBFile(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  NWBFile(const NWBFile&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  NWBFile& operator=(const NWBFile&) = delete;

  /**
   * @brief Destructor for NWBFile class.
   */
  ~NWBFile() override;

  /**
   * @brief Initializes the NWB file by setting up the file structure.
   *
   * If the file is already initialized then no action will be performed.
   *
   * @param identifierText The identifier text for the NWBFile.
   * @param description A description of the NWBFile session.
   * @param dataCollection Information about the data collection methods.
   * @param sessionStartTime ISO formatted time string with the session start
   * time. If empty (default), then the getCurrentTime() will be used.
   * @param timestampsReferenceTime ISO formatted time string with the timestamp
   * reference time. If empty (default), then the getCurrentTime() will be used.
   */
  Status initialize(const std::string& identifierText,
                    const std::string& description = "a recording session",
                    const std::string& dataCollection = "",
                    const std::string& sessionStartTime = "",
                    const std::string& timestampsReferenceTime = "");

  /**
   * @brief Check if the NWB file is initialized.
   *
   * The function simply checks if the top-level group structure exists.
   *
   * @return bool True if the file is initialized, false otherwise.
   */
  bool isInitialized() const;

  /**
   * @brief Create ElectrodesTable.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param recordingArrays vector of ChannelVector indicating the electrodes to
   *                        add to the table. This vector should contain all the
   *                        electrodes that are detected by the acquisition
   *                        system, not only those being actively recorded from.
   * @param finalizeTable If true (default) then the table will be finalized
   *                      after creation to write it to the file. If false, the
   *                      caller must call finalize() on the returned table
   *                      object to write it to the file.
   * @param rowChunkSize The chunk size to use for the rows of the table.
   * @return The generated ElectrodesTable or nullptr if failed.
   */
  std::shared_ptr<ElectrodesTable> createElectrodesTable(
      std::vector<Types::ChannelVector> recordingArrays,
      bool finalizeTable = true,
      const SizeType rowChunkSize = 100);

  /**
   * @brief Ensure that the events group exists in the NWB file.
   *
   * This function checks if the events group exists and creates it if it does
   * not.
   *
   * @param io The shared pointer to the IO object.
   * @return Status::Success if the group exists or was successfully created,
   * Status::Failure otherwise.
   */
  static Status requireEventsGroup(std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Ensure that the intervals group exists in the NWB file.
   *
   * This function checks if the intervals group exists and creates it if it
   * does not.
   *
   * @param io The shared pointer to the IO object.
   * @return Status::Success if the group exists or was successfully created,
   * Status::Failure otherwise.
   */
  static Status requireIntervalsGroup(std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Create an EventsTable in the EVENTS_PATH group.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param name The name of the EventsTable to create.
   * @param description Description of the table.
   * @param sourceDescription Optional short text description of where the
   * events came from.
   * @param timestampResolution The temporal resolution of the timestamps in
   * seconds.
   * @param createDurationColumn Whether to create the optional duration column
   * (default: false). If true, the duration column will be created with the
   * specified `durationResolution`. If false, the duration column will not be
   * created.
   * @param durationResolution The temporal resolution of the optional duration
   * column in seconds. See `DurationVectorData::initialize()` for more details.
   * If a std::nullopt is provided, the duration.resolution attribute of the
   * duration column will not be created. Note, if `createDurationColumn` is
   * false, this parameter is ignored.
   * @param createAnnotationColumn Whether to create the annotation column.
   * @param rowChunkSize The chunk size for the rows of the table.
   * @return The generated EventsTable or nullptr if failed.
   */
  std::shared_ptr<EventsTable> createEventsTable(
      const std::string& name,
      const std::string& description,
      const std::optional<std::string>& sourceDescription = std::nullopt,
      std::optional<float> timestampResolution = std::nullopt,
      bool createDurationColumn = false,
      std::optional<float> durationResolution = std::nullopt,
      const bool createAnnotationColumn = false,
      const SizeType rowChunkSize = 100);

  /**
   * @brief Create an EventsTable in the EVENTS_PATH group using a pre-built
   * column spec list.
   *
   * This overload is useful when the caller has already constructed a column
   * spec vector (e.g. via EventsTable::createDefaultDataSpecs() followed by
   * push_back() calls to add custom columns) and wants to pass it directly
   * instead of using the individual resolution/flag parameters.
   *
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param name The name of the EventsTable to create.
   * @param description Description of the table.
   * @param sourceDescription Optional short text description of where the
   * events came from.
   * @param columnSpecs Pre-built vector of column specs (e.g. from
   * EventsTable::createDefaultDataSpecs() with additional custom specs
   * appended).
   * @return The generated EventsTable or nullptr if failed.
   */
  std::shared_ptr<EventsTable> createEventsTable(
      const std::string& name,
      const std::string& description,
      const std::string& sourceDescription,
      const std::vector<NWB::DynamicTable::DataSpecPtr>& columnSpecs);

  /**
   * @brief Create the Epochs table in the INTERVALS_PATH group.
   * @param createTagsColumn Whether to create the tags column.
   * @param rowChunkSize The chunk size for the rows of the table.
   * @return The generated TimeIntervals or nullptr if failed.
   */
  std::shared_ptr<TimeIntervals> createEpochs(
      const bool createTagsColumn = false, const SizeType rowChunkSize = 100);

  /**
   * @brief Create the Trials table in the INTERVALS_PATH group.
   * @param createTagsColumn Whether to create the tags column.
   * @param rowChunkSize The chunk size for the rows of the table.
   * @return The generated TimeIntervals or nullptr if failed.
   */
  std::shared_ptr<TimeIntervals> createTrials(
      const bool createTagsColumn = false, const SizeType rowChunkSize = 100);

  /**
   * @brief Create the Invalid Times table in the INTERVALS_PATH group.
   * @param createTagsColumn Whether to create the tags column.
   * @param rowChunkSize The chunk size for the rows of the table.
   * @return The generated TimeIntervals or nullptr if failed.
   */
  std::shared_ptr<TimeIntervals> createInvalidTimes(
      const bool createTagsColumn = false, const SizeType rowChunkSize = 100);

  /**
   * @brief Create a TimeIntervals table in the INTERVALS_PATH group.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param name The name of the TimeIntervals table to create (e.g., "epochs",
   * "trials", "invalid_times").
   * @param description Description of the table.
   * @param createTagsColumn Whether to create the tags column.
   * @param rowChunkSize The chunk size for the rows of the table.
   * @return The generated TimeIntervals or nullptr if failed.
   */
  std::shared_ptr<TimeIntervals> createTimeIntervals(
      const std::string& name,
      const std::string& description,
      const bool createTagsColumn = false,
      const SizeType rowChunkSize = 100);

  /**
   * @brief Create a TimeIntervals table in the INTERVALS_PATH group using a
   * pre-built column spec list.
   *
   * This overload is useful when the caller has already constructed a column
   * spec vector (e.g. via TimeIntervals::createDefaultDataSpecs() followed by
   * push_back() calls to add custom columns) and wants to pass it directly.
   *
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param name The name of the TimeIntervals table to create.
   * @param description Description of the table.
   * @param columnSpecs Pre-built vector of column specs.
   * @return The generated TimeIntervals or nullptr if failed.
   */
  std::shared_ptr<TimeIntervals> createTimeIntervals(
      const std::string& name,
      const std::string& description,
      const std::vector<NWB::DynamicTable::DataSpecPtr>& columnSpecs);

  /**
   * @brief Create ElectricalSeries objects to record data into.
   * Created objects are automatically added to the I/O's RecordingObjects.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param recordingArrays vector of ChannelVector indicating the electrodes to
   *                        record from. A separate ElectricalSeries will be
   *                        created for each ChannelVector.
   * @param recordingNames vector indicating the names of the ElectricalSeries
   * within the acquisition group
   * @param dataType The data type of the elements in the data block.
   * @param containerIndexes This vector is updated with the indexes of the
   * created containers.
   * @return Status The status of the object creation operation.
   */
  Status createElectricalSeries(
      std::vector<Types::ChannelVector> recordingArrays,
      std::vector<std::string> recordingNames,
      const IO::BaseDataType& dataType,
      std::vector<SizeType>& containerIndexes);

  /**
   * @brief Create SpikeEventSeries objects to record data into.
   * Created objects are automatically added to the I/O's RecordingObjects.
   * @param recordingArrays vector of ChannelVector indicating the electrodes to
   *                        record from. A separate ElectricalSeries will be
   *                        created for each ChannelVector.
   * @param recordingNames vector indicating the names of the SpikeEventSeries
   * within the acquisition group
   * @param dataType The data type of the elements in the data block.
   * @param containerIndexes This vector is updated with the indexes of the
   * created containers.
   * @return Status The status of the object creation operation.
   */
  Status createSpikeEventSeries(
      std::vector<Types::ChannelVector> recordingArrays,
      std::vector<std::string> recordingNames,
      const IO::BaseDataType& dataType,
      std::vector<SizeType>& containerIndexes);

  /** @brief Create AnnotationSeries objects to record data into.
   * Created objects are automatically added to the I/O's RecordingObjects.
   * @param recordingNames vector indicating the names of the AnnotationSeries
   * within the acquisition group
   * @param containerIndexes This vector is updated with the indexes of the
   * created containers.
   * @return Status The status of the object creation operation.
   */
  Status createAnnotationSeries(const std::vector<std::string>& recordingNames,
                                std::vector<SizeType>& containerIndexes);

  DEFINE_REGISTERED_FIELD(readEpochs,
                          TimeIntervals,
                          "intervals/epochs",
                          "Table of experimental epochs.")

  DEFINE_REGISTERED_FIELD(readTrials,
                          TimeIntervals,
                          "intervals/trials",
                          "Table of experimental trials.")

  DEFINE_REGISTERED_FIELD(readInvalidTimes,
                          TimeIntervals,
                          "intervals/invalid_times",
                          "Table of invalid times.")

  DEFINE_REGISTERED_FIELD(readElectrodesTable,
                          ElectrodesTable,
                          ElectrodesTable::electrodesTablePath,
                          "table with the extracellular electrodes")

  DEFINE_ATTRIBUTE_FIELD(readNWBVersion,
                         std::string,
                         "nwb_version",
                         File version string)

  DEFINE_DATASET_FIELD(readFileCreateDate,
                       recordFileCreateDate,
                       std::string,
                       "file_create_date",
                       A record of the date the file was created and of
                           subsequent modifications)

  DEFINE_DATASET_FIELD(readIdentifier,
                      recordIdentifier,
                      std::string,
                      "identifier",
                      A unique text identifier for the file)

  DEFINE_DATASET_FIELD(readSessionDescription,
                       recordSessionDescription,
                       std::string,
                       "session_description",
                       A description of the experimental session and data in the
                           file)

  DEFINE_DATASET_FIELD(readSessionStartTime,
                       recordSessionStartTime,
                       std::string,
                       "session_start_time",
                       Date and time of the experiment or session start)

  DEFINE_DATASET_FIELD(readTimestampsReferenceTime,
                       recordTimestampsReferenceTime,
                       std::string,
                       "timestamps_reference_time",
                       Date and time corresponding to time zero of all
                           timestamps)

  DEFINE_UNNAMED_REGISTERED_FIELD(readAquisitionSeries,
                                  createAquisitionSeries,
                                  TimeSeries,
                                  "acquisition",
                                  Get a TimeSeries stored in the acquisition
                                      group)

  DEFINE_UNNAMED_REGISTERED_FIELD(readProcessingModule,
                                  createProcessingModule,
                                  ProcessingModule,
                                  "processing",
                                  Get a ProcessingModule stored in the
                                      processing group)

  DEFINE_UNNAMED_REGISTERED_FIELD(readTimeIntervals,
                                  createTimeIntervals,
                                  TimeIntervals,
                                  "intervals",
                                  Get a TimeIntervals object stored in the
                                      intervals group)

  DEFINE_UNNAMED_REGISTERED_FIELD(readEventsTable,
                                  createEventsTable,
                                  EventsTable,
                                  "events",
                                  Get an EventsTable stored in the events group)

protected:
  /**
   * @brief Creates the default file structure.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   *
   * @param identifierText The identifier text for the NWBFile.
   * @param description A description of the NWBFile session.
   * @param dataCollection Information about the data collection methods.
   * @param sessionStartTime ISO formatted time string with the session start
   * time
   * @param timestampsReferenceTime ISO formatted time string with the timestamp
   * reference time
   * @return Status The status of the file structure creation.
   */
  Status createFileStructure(const std::string& identifierText,
                             const std::string& description,
                             const std::string& dataCollection,
                             const std::string& sessionStartTime,
                             const std::string& timestampsReferenceTime);

private:
  /**
   * @brief Saves the specification files for the schema.
   *
   * @param namespaceInfo The NamespaceInfo object with the namespace
   * specification
   */
  void cacheSpecifications(const Types::NamespaceInfo& namespaceInfo);

  /**
   * @brief The ElectrodesTable for the file
   */
  std::unique_ptr<ElectrodesTable> m_electrodeTable;
};

}  // namespace AQNWB::NWB
