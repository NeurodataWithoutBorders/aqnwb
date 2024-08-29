#pragma once

#include "Types.hpp"
#include "nwb/RecordingContainers.hpp"
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
   * @param filename The name of the file to open.
   * @param recordingArrays ChannelVector objects indicating the electrodes to
   *                        use for ElectricalSeries recordings
   * @param IOType Type of backend IO to use
   */
  Status openFile(const std::string& filename,
                  std::vector<Types::ChannelVector> recordingArrays,
                  const std::string& IOType = "HDF5",
                  RecordingContainers* recordingContainers = nullptr);

  /**
   * @brief Closes the file and performs necessary cleanup when recording
   * stops.
   */
  void closeFile();

private:
  /**
   * @brief Pointer to the current NWB file.
   */
  std::unique_ptr<NWBFile> nwbfile;
};
}  // namespace AQNWB::NWB
