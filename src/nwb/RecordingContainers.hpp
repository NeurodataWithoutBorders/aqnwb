#pragma once

#include "Channel.hpp"
#include "Types.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{

/**
 * @brief The RecordingContainers class provides an interface for managing
 * and holding groups of Containers acquired during a recording.
 */

class RecordingContainers
{
public:
  /**
   * @brief Constructor for RecordingContainer class.
   */
  RecordingContainers();

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
   * @brief Adds a Container object to the container.
   * @param data The Container object to add.
   */
  void addData(std::unique_ptr<Container> data);

  /**
   * @brief Gets the Container object from the recordingContainers
   * @param containerInd The index of the container dataset within the group.
   */
  Container* getContainer(const SizeType& containerInd);

  /**
   * @brief Write timeseries data to a recordingContainer dataset.
   * @param containerInd The index of the timeseries dataset within the
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
  Status writeTimeseriesData(const SizeType& containerInd,
                             const Channel& channel,
                             const std::vector<SizeType>& dataShape,
                             const std::vector<SizeType>& positionOffset,
                             const void* data,
                             const void* timestamps);

  /**
   * @brief Write ElectricalSereis data to a recordingContainer dataset.
   * @param containerInd The index of the electrical series dataset within the
   * electrical series group.
   * @param channel The channel index to use for writing timestamps.
   * @param numSamples Number of samples in the time, i.e., the size of the
   *                   first dimension of the data parameter
   * @param data A pointer to the data block.
   * @param timestamps A pointer to the timestamps block. May be null if
   * multidimensional TimeSeries and only need to write the timestamps once but
   * write data multiple times.
   * @return The status of the write operation.
   */
  Status writeElectricalSeriesData(const SizeType& containerInd,
                                   const Channel& channel,
                                   const SizeType& numSamples,
                                   const void* data,
                                   const void* timestamps);

  std::vector<std::unique_ptr<Container>> containers;
  std::string name;
};

}  // namespace AQNWB::NWB
