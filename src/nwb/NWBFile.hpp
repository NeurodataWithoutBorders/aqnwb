#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "BaseIO.hpp"
#include "Types.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{

class RecordingContainers;  // declare here because gets used in NWBFile class

/**
 * @brief The NWBFile class provides an interface for setting up and managing
 * the NWB file.
 */
class NWBFile
{
public:
  /**
   * @brief Constructor for NWBFile class.
   * @param idText The identifier text for the NWBFile.
   * @param io The shared pointer to the IO object.
   */
  NWBFile(const std::string& idText, std::shared_ptr<BaseIO> io);

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
   * @brief Initializes the NWB file by opening and setting up the file
   * structure.
   */
  void initialize();

  /**
   * @brief Finalizes the NWB file by closing it.
   */
  void finalize();

  /**
   * @brief Starts a recording.
   * @param dataType The data type of the elements in the data block.
   * @return Status The status of the recording operation.
   */
  Status startElectricalSeriesRecording(
      std::vector<Types::ChannelVector> recordingArrays,
      const BaseDataType& dataType = BaseDataType::I16);

  /**
   * @brief Closes the relevant datasets.
   */
  void stopRecording();

  /**
   * @brief Indicates the NWB schema version.
   */
  const std::string NWBVersion = "2.7.0";

  /**
   * @brief Indicates the HDMF schema version.
   */
  const std::string HDMFVersion = "1.8.0";

  /**
   * @brief Indicates the HDMF experimental version.
   */
  const std::string HDMFExperimentalVersion = "0.5.0";

  /**
   * @brief Gets the TimeSeries object from the recording containers
   * @param containerName The name of the timeseries group.
   * @param timeseriesInd The index of the timeseries dataset within the group.
   */
  TimeSeries* getTimeSeries(const SizeType& timeseriesInd);

protected:
  /**
   * @brief Creates the default file structure.
   * @return Status The status of the file structure creation.
   */
  Status createFileStructure();

private:
  /**
   * @brief Factory method for creating recording data.
   * @param type The base data type.
   * @param size The size of the dataset.
   * @param chunking The chunking size of the dataset.
   * @param path The location in the file of the new dataset.
   * @return std::unique_ptr<BaseRecordingData> The unique pointer to the
   * created recording data.
   */
  std::unique_ptr<BaseRecordingData> createRecordingData(
      BaseDataType type,
      const SizeArray& size,
      const SizeArray& chunking,
      const std::string& path);

  /**
   * @brief Saves the specification files for the schema.
   * @param specPath The location in the file to store the spec information.
   * @param versionNumber The version number of the specification files.
   */
  void cacheSpecifications(const std::string& specPath,
                           const std::string& versionNumber);

  const std::string identifierText;
  std::shared_ptr<BaseIO> io;
  std::unique_ptr<RecordingContainers> recordingContainers =
      std::make_unique<RecordingContainers>("RecordingContainers");
};

/**
 * @brief The RecordingContainers class provides an interface for managing
 * groups of TimeSeries acquired during a recording.
 */
class RecordingContainers
{
public:
  /**
   * @brief Constructor for RecordingContainer class.
   * @param name The name of the group of time series
   */
  RecordingContainers(const std::string& name);

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  RecordingContainers(const RecordingContainers&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  RecordingContainers& operator=(const RecordingContainers&) = delete;

  /**
   * @brief Destructor for RecordingContainer class.
   */
  ~RecordingContainers();

  /**
   * @brief Adds a TimeSeries object to the container.
   * @param data The TimeSeries object to add.
   */
  void addData(std::unique_ptr<TimeSeries> data);

  std::vector<std::unique_ptr<TimeSeries>> containers;
  std::string name;
};

}  // namespace AQNWB::NWB
