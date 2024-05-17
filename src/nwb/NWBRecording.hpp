#pragma once

#include "BaseIO.hpp"
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
   * @brief Opens all the necessary files for recording.
   * @param rootFolder The root folder where the files will be stored.
   * @param baseName The base name of the files (will be appended with experiment number).
   * @param experimentNumber The experiment number.
   * @param recordingNumber The recording number.
   */
  Status openFiles(const std::string& rootFolder,
                   const std::string& baseName,
                   int experimentNumber);

  /**
   * @brief Closes all the files and performs necessary cleanup when recording
   * stops.
   */
  void closeFiles();

  /**
   * @brief Writes data for a timeseries.
   */
  void writeTimeseriesData(int timeSeriesID,
                           Channel systemChannel,
                           const float *dataBuffer,
                           const double *timestampBuffer,
                           int size);

private:
  /**
   * @brief Pointer to the current NWB file.
   */
  std::unique_ptr<NWBFile> nwbfile;

  /**
   * @brief Holds integer sample numbers for writing. 
   */
  std::unique_ptr<int[]> sampleBuffer = nullptr;

  /**
   * @brief Holds scaled samples for writing. 
   */
  std::unique_ptr<float[]> scaledBuffer = nullptr;

  /**
   * @brief Holds integer samples for writing. 
   */
  std::unique_ptr<int16_t[]> intBuffer = nullptr;

};
}  // namespace AQNWB::NWB
