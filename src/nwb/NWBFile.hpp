#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
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
#include "nwb/file/Subject.hpp"
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

  /**
   * @brief Optional metadata of the experiment in `/general`.
   *
   * All fields are optional; only the fields that are set are written. Pass an
   * instance of this struct to NWBFile::initialize(). Field names follow
   * PyNWB's `NWBFile` arguments (e.g., `stimulusNotes` for
   * `/general/stimulus`). `data_collection` is set with the `dataCollection`
   * argument of NWBFile::initialize().
   */
  struct GeneralMetadataSpec
  {
    /// @brief General description of the experiment (`experiment_description`).
    std::optional<std::string> experimentDescription = std::nullopt;
    /// @brief Name of person(s) who performed the experiment (`experimenter`),
    /// e.g., "Last, First".
    std::optional<std::vector<std::string>> experimenter = std::nullopt;
    /// @brief Institution(s) where the experiment was performed
    /// (`institution`).
    std::optional<std::string> institution = std::nullopt;
    /// @brief Terms to search over (`keywords`).
    std::optional<std::vector<std::string>> keywords = std::nullopt;
    /// @brief Laboratory where the experiment was performed (`lab`).
    std::optional<std::string> lab = std::nullopt;
    /// @brief Notes about the experiment (`notes`).
    std::optional<std::string> notes = std::nullopt;
    /// @brief Description of drugs used, including how and when they were
    /// administered (`pharmacology`).
    std::optional<std::string> pharmacology = std::nullopt;
    /// @brief Experimental protocol, e.g., the IACUC protocol number
    /// (`protocol`).
    std::optional<std::string> protocol = std::nullopt;
    /// @brief Publication information, e.g., PMID, DOI, URL
    /// (`related_publications`).
    std::optional<std::vector<std::string>> relatedPublications = std::nullopt;
    /// @brief Lab-specific ID for the session (`session_id`).
    std::optional<std::string> sessionId = std::nullopt;
    /// @brief Description of slices, including preparation thickness,
    /// orientation, temperature, and bath solution (`slices`).
    std::optional<std::string> slices = std::nullopt;
    /// @brief Script file or link to public source code used to create this NWB
    /// file (`source_script`). Requires sourceScriptFileName.
    std::optional<std::string> sourceScript = std::nullopt;
    /// @brief Name of the script file (`file_name` attribute of
    /// `source_script`). Required if sourceScript is set.
    std::optional<std::string> sourceScriptFileName = std::nullopt;
    /// @brief Notes about stimuli, such as how and where they were presented
    /// (`/general/stimulus`).
    std::optional<std::string> stimulusNotes = std::nullopt;
    /// @brief Narrative description about surgery/surgeries, including date(s)
    /// and who performed surgery (`surgery`).
    std::optional<std::string> surgery = std::nullopt;
    /// @brief Information about virus(es) used in experiments, including virus
    /// ID, source, date made, injection location, volume (`virus`).
    std::optional<std::string> virus = std::nullopt;
    /// @brief Name and version of the software package(s) used to generate the
    /// data in this file (`was_generated_by`), e.g., {{"Open Ephys GUI",
    /// "1.1.0"}}.
    std::optional<std::vector<std::pair<std::string, std::string>>>
        wasGeneratedBy = std::nullopt;

    /**
     * @brief Set the experimentDescription field.
     * @param value General description of the experiment
     * (`experiment_description`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withExperimentDescription(std::string value)
    {
      experimentDescription = std::move(value);
      return *this;
    }

    /**
     * @brief Set the experimenter field.
     * @param value Name of person(s) who performed the experiment
     * (`experimenter`), e.g., "Last, First".
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withExperimenter(std::vector<std::string> value)
    {
      experimenter = std::move(value);
      return *this;
    }

    /**
     * @brief Set the institution field.
     * @param value Institution(s) where the experiment was performed
     * (`institution`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withInstitution(std::string value)
    {
      institution = std::move(value);
      return *this;
    }

    /**
     * @brief Set the keywords field.
     * @param value Terms to search over (`keywords`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withKeywords(std::vector<std::string> value)
    {
      keywords = std::move(value);
      return *this;
    }

    /**
     * @brief Set the lab field.
     * @param value Laboratory where the experiment was performed (`lab`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withLab(std::string value)
    {
      lab = std::move(value);
      return *this;
    }

    /**
     * @brief Set the notes field.
     * @param value Notes about the experiment (`notes`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withNotes(std::string value)
    {
      notes = std::move(value);
      return *this;
    }

    /**
     * @brief Set the pharmacology field.
     * @param value Description of drugs used, including how and when they were
     * administered (`pharmacology`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withPharmacology(std::string value)
    {
      pharmacology = std::move(value);
      return *this;
    }

    /**
     * @brief Set the protocol field.
     * @param value Experimental protocol, e.g., the IACUC protocol number
     * (`protocol`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withProtocol(std::string value)
    {
      protocol = std::move(value);
      return *this;
    }

    /**
     * @brief Set the relatedPublications field.
     * @param value Publication information, e.g., PMID, DOI, URL
     * (`related_publications`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withRelatedPublications(std::vector<std::string> value)
    {
      relatedPublications = std::move(value);
      return *this;
    }

    /**
     * @brief Set the sessionId field.
     * @param value Lab-specific ID for the session (`session_id`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withSessionId(std::string value)
    {
      sessionId = std::move(value);
      return *this;
    }

    /**
     * @brief Set the slices field.
     * @param value Description of slices, including preparation thickness,
     * orientation, temperature, and bath solution (`slices`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withSlices(std::string value)
    {
      slices = std::move(value);
      return *this;
    }

    /**
     * @brief Set the sourceScript field.
     * @param value Script file or link to public source code used to create
     * this NWB file (`source_script`). Requires sourceScriptFileName.
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withSourceScript(std::string value)
    {
      sourceScript = std::move(value);
      return *this;
    }

    /**
     * @brief Set the sourceScriptFileName field.
     * @param value Name of the script file (`file_name` attribute of
     * `source_script`). Required if sourceScript is set.
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withSourceScriptFileName(std::string value)
    {
      sourceScriptFileName = std::move(value);
      return *this;
    }

    /**
     * @brief Set the stimulusNotes field.
     * @param value Notes about stimuli, such as how and where they were
     * presented (`/general/stimulus`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withStimulusNotes(std::string value)
    {
      stimulusNotes = std::move(value);
      return *this;
    }

    /**
     * @brief Set the surgery field.
     * @param value Narrative description about surgery/surgeries, including
     * date(s) and who performed surgery (`surgery`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withSurgery(std::string value)
    {
      surgery = std::move(value);
      return *this;
    }

    /**
     * @brief Set the virus field.
     * @param value Information about virus(es) used in experiments, including
     * virus ID, source, date made, injection location, volume (`virus`).
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withVirus(std::string value)
    {
      virus = std::move(value);
      return *this;
    }

    /**
     * @brief Set the wasGeneratedBy field.
     * @param value Name and version of the software package(s) used to generate
     * the data in this file (`was_generated_by`), e.g., {{"Open Ephys GUI",
     * "1.1.0"}}.
     * @return Reference to this GeneralMetadataSpec, to allow method chaining.
     */
    GeneralMetadataSpec& withWasGeneratedBy(
        std::vector<std::pair<std::string, std::string>> value)
    {
      wasGeneratedBy = std::move(value);
      return *this;
    }
  };

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
   * @param subjectSpec Optional subject metadata. Defaults to @c std::nullopt,
   * so no Subject group is created unless a SubjectSpec is supplied. For DANDI
   * validation, the supplied SubjectSpec must include subjectId, species, sex,
   * and either age or dateOfBirth.
   * @param generalMetadataSpec Optional metadata of the experiment in
   * `/general` (e.g., experimenter, institution, keywords). Defaults to @c
   * std::nullopt. Fields whose dataset already exists in the file are not
   * changed. Fails if `sourceScript` is set without `sourceScriptFileName`.
   */
  Status initialize(
      const std::string& identifierText,
      const std::string& description = "a recording session",
      const std::string& dataCollection = "",
      const std::string& sessionStartTime = "",
      const std::string& timestampsReferenceTime = "",
      const std::optional<AQNWB::NWB::Subject::SubjectSpec>& subjectSpec =
          std::nullopt,
      const std::optional<GeneralMetadataSpec>& generalMetadataSpec =
          std::nullopt);

  /**
   * @brief Change the session start time of an initialized file.
   *
   * Overwrites the value of `/session_start_time`, so this is also possible
   * while recording (e.g., in SWMR mode), for applications that know the start
   * time only after the file was created. The new value must not be longer than
   * the value written by `initialize()` (e.g., a time from `formatTime()` if
   * the default value of `initialize()` was used).
   * @param sessionStartTime ISO 8601 formatted time string.
   * @return The status of the operation.
   */
  Status setSessionStartTime(const std::string& sessionStartTime);

  /**
   * @brief Change the timestamps reference time of an initialized file.
   *
   * Overwrites the value of `/timestamps_reference_time`, so this is also
   * possible while recording (e.g., in SWMR mode), for applications that know
   * time zero of their timestamps only after the file was created. The new
   * value must not be longer than the value written by `initialize()` (e.g., a
   * time from `formatTime()` if the default value of `initialize()` was used).
   * @param timestampsReferenceTime ISO 8601 formatted time string.
   * @return The status of the operation.
   */
  Status setTimestampsReferenceTime(const std::string& timestampsReferenceTime);

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
   * @param numSamples The number of samples per event (spike waveform). Must
   * be > 0. All events of a SpikeEventSeries span the same channels and store
   * snapshots of equal duration, so the `data` dataset has a fixed size in the
   * channel and sample dimensions, i.e., shape [num_events, num_channels,
   * numSamples] (or [num_events, numSamples] for single-channel series). `data`
   * is chunked by complete events, with the number of events per chunk chosen
   * such that a chunk is about 32 KiB.
   * @param useChannelConversion If true (default), the per-channel conversion
   * factors (Channel::getConversion()) are stored in the `channel_conversion`
   * dataset and the `conversion` attribute of `data` is set to 1. If false,
   * the conversion factor of the first channel of each ChannelVector is stored
   * in the `conversion` attribute of `data` and applies to all channels, and
   * no `channel_conversion` dataset is created. Use false when all channels
   * share the same conversion factor, e.g., for compatibility with readers
   * that do not support `channel_conversion` for SpikeEventSeries.
   * @return Status The status of the object creation operation.
   */
  Status createSpikeEventSeries(
      std::vector<Types::ChannelVector> recordingArrays,
      std::vector<std::string> recordingNames,
      const IO::BaseDataType& dataType,
      std::vector<SizeType>& containerIndexes,
      const SizeType numSamples,
      const bool useChannelConversion = true);

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

  DEFINE_DATASET_FIELD(readDataCollection,
                       recordDataCollection,
                       std::string,
                       "general/data_collection",
                       Notes about data collection and analysis)

  DEFINE_DATASET_FIELD(readExperimentDescription,
                       recordExperimentDescription,
                       std::string,
                       "general/experiment_description",
                       General description of the experiment)

  DEFINE_DATASET_FIELD(readExperimenter,
                       recordExperimenter,
                       std::string,
                       "general/experimenter",
                       Name of person(s) who performed the experiment)

  DEFINE_DATASET_FIELD(readInstitution,
                       recordInstitution,
                       std::string,
                       "general/institution",
                       Institution(s) where experiment was performed)

  DEFINE_DATASET_FIELD(readKeywords,
                       recordKeywords,
                       std::string,
                       "general/keywords",
                       Terms to search over)

  DEFINE_DATASET_FIELD(readLab,
                       recordLab,
                       std::string,
                       "general/lab",
                       Laboratory where experiment was performed)

  DEFINE_DATASET_FIELD(readNotes,
                       recordNotes,
                       std::string,
                       "general/notes",
                       Notes about the experiment)

  DEFINE_DATASET_FIELD(readPharmacology,
                       recordPharmacology,
                       std::string,
                       "general/pharmacology",
                       Description of drugs used)

  DEFINE_DATASET_FIELD(readProtocol,
                       recordProtocol,
                       std::string,
                       "general/protocol",
                       Experimental protocol)

  DEFINE_DATASET_FIELD(readRelatedPublications,
                       recordRelatedPublications,
                       std::string,
                       "general/related_publications",
                       Publication information)

  DEFINE_DATASET_FIELD(readSessionId,
                       recordSessionId,
                       std::string,
                       "general/session_id",
                       Lab-specific ID for the session)

  DEFINE_DATASET_FIELD(readSlices,
                       recordSlices,
                       std::string,
                       "general/slices",
                       Description of slices)

  DEFINE_DATASET_FIELD(
      readSourceScript,
      recordSourceScript,
      std::string,
      "general/source_script",
      Script file or link to public source code used to create this NWB file)

  DEFINE_DATASET_FIELD(readStimulusNotes,
                       recordStimulusNotes,
                       std::string,
                       "general/stimulus",
                       Notes about stimuli)

  DEFINE_DATASET_FIELD(readSurgery,
                       recordSurgery,
                       std::string,
                       "general/surgery",
                       Narrative description about surgery / surgeries)

  DEFINE_DATASET_FIELD(readVirus,
                       recordVirus,
                       std::string,
                       "general/virus",
                       Information about virus(es) used in experiments)

  DEFINE_DATASET_FIELD(readWasGeneratedBy,
                       recordWasGeneratedBy,
                       std::string,
                       "general/was_generated_by",
                       Name and version of software package(s) used to generate
                           data in this file(shape[num_sources, 2]))

  DEFINE_ATTRIBUTE_FIELD(readSourceScriptFileName,
                         std::string,
                         "general/source_script/file_name",
                         Name of script file)

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
                                  createTimeIntervalsInstance,
                                  TimeIntervals,
                                  "intervals",
                                  Get a TimeIntervals object stored in the
                                      intervals group)

  DEFINE_UNNAMED_REGISTERED_FIELD(readEventsTable,
                                  createEventsTableInstance,
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
   * @brief Writes the fields of a GeneralMetadataSpec to `/general`.
   *
   * Fields whose dataset already exists are skipped with a warning.
   * @param spec The metadata to write.
   * @return Status::Failure if a dataset could not be written or the spec is
   * invalid, Status::Success otherwise.
   */
  Status createGeneralMetadata(const GeneralMetadataSpec& spec);

  /**
   * @brief The ElectrodesTable for the file
   */
  std::unique_ptr<ElectrodesTable> m_electrodeTable;
};

}  // namespace AQNWB::NWB
