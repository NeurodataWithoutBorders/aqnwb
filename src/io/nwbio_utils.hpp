/**
 * @brief Utility functions for NWB IO operations.
 *
 * This namespace contains utility functions for helping with NWB IO operations,
 * such as writing timeseries data, electrical series data, spike event data,
 * and annotation series data to recordingContainer datasets.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Types.hpp"
#include "io/RecordingObjects.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/misc/AnnotationSeries.hpp"

/*!
 * \namespace AQNWB::IO
 * \brief The namespace for IO components of AqNWB
 */
namespace AQNWB::IO
{

/**
 * @brief Write timeseries data to a recordingContainer dataset.
 * @param recording_objects A shared pointer to the RecordingObjects instance.
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
static inline AQNWB::Types::Status writeTimeseriesData(
    std::shared_ptr<RecordingObjects> recording_objects,
    const AQNWB::Types::SizeType& containerInd,
    const AQNWB::Channel& channel,
    const std::vector<AQNWB::Types::SizeType>& dataShape,
    const std::vector<AQNWB::Types::SizeType>& positionOffset,
    const void* data,
    const void* timestamps,
    const void* controlInput = nullptr)
{
  auto registeredObject = recording_objects->getRecordingObject(containerInd);
  // Cast to TimeSeries
  // This assumes that the object at containerInd is a TimeSeries
  auto ts = std::dynamic_pointer_cast<AQNWB::NWB::TimeSeries>(registeredObject);
  if (ts == nullptr) {
    return AQNWB::Types::Status::Failure;
  }

  // write data and timestamps to datasets
  if (channel.getLocalIndex() == 0) {
    // write with timestamps if it's the first channel
    return ts->writeData(
        dataShape, positionOffset, data, timestamps, controlInput);
  } else {
    // write without timestamps and controlInput if its another channel in the
    // same timeseries
    return ts->writeData(dataShape, positionOffset, data);
  }
}

/**
 * @brief Write ElectricalSeries data to a recordingContainer dataset.
 * @param recording_objects A shared pointer to the RecordingObjects instance.
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
static inline AQNWB::Types::Status writeElectricalSeriesData(
    std::shared_ptr<RecordingObjects> recording_objects,
    const AQNWB::Types::SizeType& containerInd,
    const AQNWB::Channel& channel,
    const AQNWB::Types::SizeType& numSamples,
    const void* data,
    const void* timestamps,
    const void* controlInput = nullptr)
{
  auto registeredObject = recording_objects->getRecordingObject(containerInd);
  auto es =
      std::dynamic_pointer_cast<AQNWB::NWB::ElectricalSeries>(registeredObject);

  if (es == nullptr)
    return AQNWB::Types::Status::Failure;

  return es->writeChannel(
      channel.getLocalIndex(), numSamples, data, timestamps, controlInput);
}

/**
 * @brief Write SpikeEventSeries data to a recordingContainer dataset.
 * @param recording_objects A shared pointer to the RecordingObjects instance.
 * @param containerInd The index of the SpikeEventSeries dataset within the
 * SpikeEventSeries containers.
 * @param numSamples Number of samples in the time for the single event.
 * @param numChannels Number of channels in the time for the single event.
 * @param data A pointer to the data block.
 * @param timestamps A pointer to the timestamps block
 * @param controlInput A pointer to the control block data (optional)
 * @return The status of the write operation.
 */
static inline AQNWB::Types::Status writeSpikeEventData(
    std::shared_ptr<RecordingObjects> recording_objects,
    const AQNWB::Types::SizeType& containerInd,
    const AQNWB::Types::SizeType& numSamples,
    const AQNWB::Types::SizeType& numChannels,
    const void* data,
    const void* timestamps,
    const void* controlInput = nullptr)
{
  auto registeredObject = recording_objects->getRecordingObject(containerInd);
  auto ses =
      std::dynamic_pointer_cast<AQNWB::NWB::SpikeEventSeries>(registeredObject);

  if (ses == nullptr)
    return AQNWB::Types::Status::Failure;

  return ses->writeSpike(
      numSamples, numChannels, data, timestamps, controlInput);
}

/**
 * @brief Write AnnotationSeries data to a recordingContainer dataset.
 * @param recording_objects A shared pointer to the RecordingObjects instance.
 * @param containerInd The index of the AnnotationSeries dataset within the
 * AnnotationSeries containers.
 * @param numSamples Number of samples in the time for the single event.
 * @param data A vector of strings of data to write.
 * @param timestamps A pointer to the timestamps block
 * @param controlInput A pointer to the control block data (optional)
 * @return The status of the write operation.
 */
static inline AQNWB::Types::Status writeAnnotationSeriesData(
    std::shared_ptr<RecordingObjects> recording_objects,
    const AQNWB::Types::SizeType& containerInd,
    const AQNWB::Types::SizeType& numSamples,
    const std::vector<std::string> data,
    const void* timestamps,
    const void* controlInput = nullptr)
{
  auto registeredObject = recording_objects->getRecordingObject(containerInd);
  auto as =
      std::dynamic_pointer_cast<AQNWB::NWB::AnnotationSeries>(registeredObject);

  if (as == nullptr)
    return AQNWB::Types::Status::Failure;

  return as->writeAnnotation(numSamples, data, timestamps, controlInput);
}

}  // namespace AQNWB::IO
