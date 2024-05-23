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
   * @return Status The status of the recording operation.
   */
  Status startRecording(
      std::vector<Types::ChannelGroup>
          recordingArrays);  // TODO - add recording number to stop and restart
                             // recording to same file

  /**
   * @brief Closes the relevant datasets.
   */
  void stopRecording();

  /**
   * @brief Write continuous data to the NWB file.
   * @param datasetID The index of the continuous dataset.
   * @param numSamples The number of samples to write.
   * @param type The base data type.
   * @param data The data to write.
   */
  Status writeContinuousData(int datasetInd,
                             int numSamples,
                             BaseDataType type,
                             const void* data);

  /**
   * @brief Write a row of continuous data to the NWB file.
   * @param datasetID The index of the continuous dataset.
   * @param rowID The index of the row to write.
   * @param numSamples The number of samples to write.
   * @param type The base data type.
   * @param data The data to write.
   */
  Status writeContinuousData(int datasetInd,
                             int rowInd,
                             int numSamples,
                             BaseDataType type,
                             const void* data);

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
  std::vector<float> scaledBuffer;
  std::vector<int16_t> intBuffer;
  SizeType bufferSize;
  TimeSeriesData timeseriesData;
};
}  // namespace AQNWB::NWB
