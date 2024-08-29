#pragma once

#include "Channel.hpp"
#include "Types.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{

/**
 * @brief The RecordingContainers class provides an interface for managing
 * and holding groups of TimeSeries acquired during a recording.
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
   * @brief Adds a TimeSeries object to the container.
   * @param data The TimeSeries object to add.
   */
  void addData(std::unique_ptr<TimeSeries> data);

  /**
   * @brief Gets the TimeSeries object from the recordingContainers
   * @param timeseriesInd The index of the timeseries dataset within the group.
   */
  TimeSeries* getTimeSeries(const SizeType& timeseriesInd);

  /**
   * @brief Write timeseries data to a recordingContainer dataset.
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
  Status writeTimeseriesData(const SizeType& timeseriesInd,
                             const Channel& channel,
                             const std::vector<SizeType>& dataShape,
                             const std::vector<SizeType>& positionOffset,
                             const void* data,
                             const void* timestamps);

  std::vector<std::unique_ptr<TimeSeries>> containers;
  std::string name;
};

}  // namespace AQNWB::NWB
