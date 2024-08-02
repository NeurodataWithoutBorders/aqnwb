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
   * @param recordingArrays ChannelVector objets indicating the electrodes to use for 
   *                        ElectricalSeries recordings 
   * @param IOType Type of backend IO to use
   */
  Status openFile(const std::string& rootFolder,
                  const std::string& baseName,
                  int experimentNumber,
                  std::vector<Types::ChannelVector> recordingArrays,
                  const std::string& IOType = "HDF5");

  /**
   * @brief Closes the file and performs necessary cleanup when recording
   * stops.
   */
  void closeFile();

  /**
   * @brief Write timeseries to an NWB file.
   * @param containerName The name of the timeseries group to write to.
   * @param timeseriesInd The index of the timeseries dataset within the
   * timeseries group.
   * @param channel The channel index to use for writing timestamps.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param data A pointer to the data block.
   * @param timestamps A pointer to the timestamps block. May be null if
   * multidimensional TimeSeries and only need to write the timestamps once but
   * write data multiple times.
   * @return The status of the write operation.
   */
  Status writeTimeseriesData(const std::string& containerName,
                             const SizeType& timeseriesInd,
                             const Channel& channel,
                             const std::vector<SizeType>& dataShape,
                             const std::vector<SizeType>& positionOffset,
                             const void* data,
                             const void* timestamps);

private:
  /**
   * @brief Pointer to the current NWB file.
   */
  std::unique_ptr<NWBFile> nwbfile;
};
}  // namespace AQNWB::NWB
