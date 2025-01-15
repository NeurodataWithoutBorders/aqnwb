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
   * @brief Adds a Container object to the container. Note that this function
   * transfers ownership of the Container object to the RecordingContainers
   * object, and should be called with the pattern
   * recordingContainers.addContainer(std::move(container)).
   * @param container The Container object to add.
   */
  void addContainer(std::unique_ptr<Container> container);

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
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeTimeseriesData(const SizeType& containerInd,
                             const Channel& channel,
                             const std::vector<SizeType>& dataShape,
                             const std::vector<SizeType>& positionOffset,
                             const void* data,
                             const void* timestamps,
                             const void* controlInput = nullptr);

  /**
   * @brief Write ElectricalSeries data to a recordingContainer dataset.
   * @param containerInd The index of the electrical series dataset within the
   * electrical series group.
   * @param channel The channel index to use for writing timestamps.
   * @param numSamples Number of samples in the time, i.e., the size of the
   *                   first dimension of the data parameter
   * @param data A pointer to the data block.
   * @param timestamps A pointer to the timestamps block. May be null if
   * multidimensional TimeSeries and only need to write the timestamps once but
   * write data multiple times.
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeElectricalSeriesData(const SizeType& containerInd,
                                   const Channel& channel,
                                   const SizeType& numSamples,
                                   const void* data,
                                   const void* timestamps,
                                   const void* controlInput = nullptr);

  /**
   * @brief Write SpikeEventSeries data to a recordingContainer dataset.
   * @param containerInd The index of the SpikeEventSeries dataset within the
   * SpikeEventSeries containers.
   * @param numSamples Number of samples in the time for the single event.
   * @param numChannels Number of channels in the time for the single event.
   * @param data A pointer to the data block.
   * @param timestamps A pointer to the timestamps block
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeSpikeEventData(const SizeType& containerInd,
                             const SizeType& numSamples,
                             const SizeType& numChannels,
                             const void* data,
                             const void* timestamps,
                             const void* controlInput = nullptr);

  /**
   * @brief Write AnnotationSeries data to a recordingContainer dataset.
   * @param containerInd The index of the AnnotationSeries dataset within the
   * AnnotationSeries containers.
   * @param numSamples Number of samples in the time for the single event.
   * @param data A vector of strings of data to write.
   * @param timestamps A pointer to the timestamps block
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeAnnotationSeriesData(const SizeType& containerInd,
                                   const SizeType& numSamples,
                                   const std::vector<std::string> data,
                                   const void* timestamps,
                                   const void* controlInput = nullptr);

  /**
   * @brief Get the number of recording containers
   */
  inline SizeType size() const { return m_containers.size(); }

private:
  /**
   * @brief The Containers used for recording
   */
  std::vector<std::unique_ptr<Container>> m_containers;

  /**
   * @brief The name of the collection of recording containers
   */
  std::string m_name;
};

}  // namespace AQNWB::NWB
