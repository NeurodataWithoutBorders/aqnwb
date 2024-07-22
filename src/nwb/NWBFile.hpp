#pragma once

#include <cstdint>

#include "BaseIO.hpp"
#include "Types.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{

using TimeSeriesData = std::vector<std::unique_ptr<TimeSeries>>;

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
  Status startRecording(std::vector<Types::ChannelGroup> recordingArrays,
                        const BaseDataType& dataType = BaseDataType::I16);

  /**
   * @brief Closes the relevant datasets.
   */
  void stopRecording();

  /**
   * @brief Write timeseries to an NWB file.
   * @param datasetInd The index of the timeseries dataset.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param dataType The data type of the elements in the data block.
   * @param data A pointer to the data block.
   * @param timestamps A pointer to the timestamps block. May be null if
   * multidimensional TimeSeries and only need to write the timestamps once but
   * write data multiple times.
   * @return The status of the write operation.
   */
  Status writeTimeseries(SizeType datasetInd,
                         const std::vector<SizeType>& dataShape,
                         const std::vector<SizeType>& positionOffset,
                         const void* data,
                         const BaseDataType& dataType = BaseDataType::I16,
                         const void* timestamps = nullptr);

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

  /**
   * @brief Creates a new dataset to hold text data (messages).
   * @param path The location in the file for the dataset.
   * @param name The name of the dataset.
   * @param text The text data to be stored in the dataset.
   */
  void createTextDataSet(const std::string& path,
                         const std::string& name,
                         const std::string& text);

  const std::string identifierText;
  std::shared_ptr<BaseIO> io;
  TimeSeriesData timeseriesData;
};
}  // namespace AQNWB::NWB
