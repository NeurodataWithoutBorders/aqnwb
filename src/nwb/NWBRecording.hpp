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
   * @param timeseriesInd The index of the timeseries.
   * @param channel The channel information.
   * @param data The data to write.
   * @param dataType The data type of the data.
   * @param timestamps The timestamps for the data.
   * @param numSamples The number of samples to write.
   * @param positionOffset The position offset for the data.
   */
  void writeTimeseriesData(SizeType timeseriesInd,
                           Channel channel,
                           const void* data,
                           const BaseDataType& dataType,
                           const void* timestamps,
                           SizeType numSamples,
                           std::vector<SizeType> positionOffset);

private:
  /**
   * @brief Pointer to the current NWB file.
   */
  std::unique_ptr<NWBFile> nwbfile;
};
}  // namespace AQNWB::NWB
