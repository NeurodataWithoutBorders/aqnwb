#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "BaseIO.hpp"
#include "Types.hpp"
#include "nwb/base/TimeSeries.hpp"

/*!
 * \namespace AQNWB::NWB
 * \brief Namespace for all classes related to the NWB data standard
 */
namespace AQNWB::NWB
{

class RecordingContainers;  // declare here because gets used in NWBFile class

/**
 * @brief The NWBFile class provides an interface for setting up and managing
 * the NWB file.
 */
class NWBFile : public Container
{
public:
  /**
   * @brief Constructor for NWBFile class.
   * @param io The shared pointer to the IO object.
   */
  NWBFile(std::shared_ptr<BaseIO> io);

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
   *
   *  @param idText The identifier text for the NWBFile.
   */
  Status initialize(const std::string& idText);

  /**
   * @brief Finalizes the NWB file by closing it.
   */
  Status finalize();

  /**
   * @brief Create ElectricalSeries objects to record data into.
   * Created objects are stored in recordingContainers.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param recordingArrays vector of ChannelVector indicating the electrodes to
   *                        record from. A separate ElectricalSeries will be
   *                        created for each ChannelVector.
   * @param dataType The data type of the elements in the data block.
   * @return Status The status of the object creation operation.
   */
  Status createElectricalSeries(
      std::vector<Types::ChannelVector> recordingArrays,
      const BaseDataType& dataType = BaseDataType::I16);

  /**
   * @brief Starts the recording.
   */
  Status startRecording();

  /**
   * @brief Stops the recording.
   */
  void stopRecording();

  /**
   * @brief Gets the TimeSeries object from the recordingContainers
   * @param timeseriesInd The index of the timeseries dataset within the group.
   */
  TimeSeries* getTimeSeries(const SizeType& timeseriesInd);

protected:
  /**
   * @brief Creates the default file structure.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
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
   * @param specVariables The contents of the specification files.
   * These values are generated from the nwb schema by
   * `resources/generate_spec_files.py`
   */
  template<SizeType N>
  void cacheSpecifications(
      const std::string& specPath,
      const std::string& versionNumber,
      const std::array<std::pair<std::string_view, std::string_view>, N>&
          specVariables);

  /**
   * @brief Holds the Container (usually TimeSeries) objects that have been
   * created in the nwb file for recording.
   */
  std::unique_ptr<RecordingContainers> recordingContainers =
      std::make_unique<RecordingContainers>("RecordingContainers");

  std::string identifierText;  // TODO Remove this for read
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
