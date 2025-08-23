#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "io/RecordingObjects.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "nwb/hdmf/base/Container.hpp"
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
 */
class NWBFile : public Container
{
public:
  // Register the ElectrodeTable as a subclass of Container
  REGISTER_SUBCLASS(NWBFile, Container, AQNWB::SPEC::CORE::namespaceName)

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
  NWBFile(std::shared_ptr<IO::BaseIO> io);

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
  ~NWBFile();

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
   * @return The generated ElectrodeTable or nullptr if failed.
   */
  std::shared_ptr<ElectrodeTable> createElectrodesTable(
      std::vector<Types::ChannelVector> recordingArrays,
      bool finalizeTable = true);

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
  Status createAnnotationSeries(std::vector<std::string> recordingNames,
                                std::vector<SizeType>& containerIndexes);

  DEFINE_REGISTERED_FIELD(readElectrodeTable,
                          ElectrodeTable,
                          ElectrodeTable::electrodeTablePath,
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

  inline const static std::string m_acquisitionPath = "/acquisition";

  inline const static std::string m_specificationsPath = "/specifications";

  /**
   * @brief The ElectrodeTable for the file
   */
  std::unique_ptr<ElectrodeTable> m_electrodeTable;
};

}  // namespace AQNWB::NWB
