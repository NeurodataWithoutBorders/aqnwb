#pragma once

#include <cstdint>

#include "io/BaseIO.hpp"

using namespace AQNWBIO;

/**
 * @brief The NWBFile class provides an interface for setting up and managing the NWB file.
 */
class NWBFile
{
public:
  /**
   * @brief Constructor for NWBFile class.
   * @param idText The identifier text for the NWBFile.
   * @param io The shared pointer to the IO object.
   */
  NWBFile(std::string idText, std::shared_ptr<BaseIO> io);

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
   * @brief Initializes the NWB file by opening and setting up the file structure.
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
  Status startRecording();

  /**
   * @brief Closes the relevant datasets.
   */
  void stopRecording();

  /**
   * @brief Writes continuous data for a particular channel.
   * @param datasetID The ID of the dataset.
   * @param channel The channel number.
   * @param nSamples The number of samples.
   * @param data The pointer to the data array.
   * @param bitVolts The bit to volts conversion factor.
   */
  void writeData(int datasetID,
                 int channel,
                 int nSamples,
                 const float* data,
                 float bitVolts);

  /**
   * @brief Indicates the NWB schema version.
   */
  const std::string NWBVersion = "2.7.0";

  /**
   * @brief Indicates the HDMF schema version.
   */
  const std::string HDMFVersion = "1.8.0";

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
   * @return std::unique_ptr<BaseRecordingData> The unique pointer to the created recording data.
   */
  std::unique_ptr<BaseRecordingData> createRecordingData(
    BaseDataType type, const SizeArray& size, const SizeArray& chunking, const std::string& path);

  /**
   * @brief Saves the specification files for the schema.
   * @param specPath The location in the file to store the spec information.
   * @param versionNumber The version number of the specification files.
   */
  void cacheSpecifications(std::string specPath, std::string versionNumber);

  /**
   * @brief Creates a new dataset to hold text data (messages).
   * @param path The location in the file for the dataset.
   * @param name The name of the dataset.
   * @param text The text data to be stored in the dataset.
   */
  void createTextDataSet(std::string path, std::string name, std::string text);

  const std::string identifierText;
  std::shared_ptr<BaseIO> io;
  std::vector<float> scaledBuffer;
  std::vector<int16_t> intBuffer;
  SizeType bufferSize;
};


/**
 * @brief The NWBRecordingEngine class manages the recording process
 */
class NWBRecordingEngine
{
public:
  /**
   * @brief Default constructor for NWBRecordingEngine.
   */
  NWBRecordingEngine();

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  NWBRecordingEngine(const NWBRecordingEngine&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  NWBRecordingEngine& operator=(const NWBRecordingEngine&) = delete;

  /**
   * @brief Destructor for NWBRecordingEngine.
   */
  ~NWBRecordingEngine();

  /**
   * @brief Opens all the necessary files for recording.
   * @param rootFolder The root folder where the files will be stored.
   * @param experimentNumber The experiment number.
   * @param recordingNumber The recording number.
   */
  void openFiles(std::string rootFolder,
                 int experimentNumber,
                 int recordingNumber);

  /**
   * @brief Closes all the files and performs necessary cleanup when recording stops.
   */
  void closeFiles();

private:
  /**
   * @brief Pointer to the current NWB file.
   */
  std::unique_ptr<NWBFile> nwb;

  /**
   * @brief Holds integer sample numbers for writing.
   */
  std::vector<int64_t> smpBuffer;
};
