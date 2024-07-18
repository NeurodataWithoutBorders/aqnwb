#pragma once

#include "Types.hpp"
#include "nwb/NWBFile.hpp"

namespace AQNWB::NWB
{
/**
 * @brief The NWBRecording class manages the recording process
 */

class NWBRecording
{
public:
  /**
   * @brief Default constructor for NWBRecording.
   */
  NWBRecording();

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  NWBRecording(const NWBRecording&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  NWBRecording& operator=(const NWBRecording&) = delete;

  /**
   * @brief Destructor for NWBRecordingEngine.
   */
  ~NWBRecording();

  /**
   * @brief Opens the file for recording.
   * @param rootFolder The root folder where the file will be stored.
   * @param baseName The base name of the file (will be appended with
   * experiment number).
   * @param experimentNumber The experiment number.
   * @param recordingNumber The recording number.
   */
  Status openFile(const std::string& rootFolder,
                  const std::string& baseName,
                  int experimentNumber,
                  std::vector<Types::ChannelGroup> recordingArrays,
                  const std::string& IOType = "HDF5");

  /**
   * @brief Closes the file and performs necessary cleanup when recording
   * stops.
   */
  void closeFile();

  /**
   * @brief Writes data for a timeseries.
   */
  void writeTimeseriesData(SizeType timeSeriesID,
                           Channel systemChannel,
                           const float* dataBuffer,
                           const double* timestampBuffer,
                           SizeType size, 
                           std::vector<SizeType> positionOffset);

private:
  /**
   * @brief Pointer to the current NWB file.
   */
  std::unique_ptr<NWBFile> nwbfile;

  /**
   * @brief Holds scaled samples for writing.
   */
  std::unique_ptr<float[]> scaledData = nullptr;

  /**
   * @brief Holds integer samples for writing.
   */
  std::unique_ptr<int16_t[]> intData = nullptr;

};
}  // namespace AQNWB::NWB
