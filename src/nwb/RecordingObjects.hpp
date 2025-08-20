#pragma once

#include "Channel.hpp"
#include "Types.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{

// Forward declaration
class RegisteredType;

/**
 * @brief The RecordingObjects class provides an interface for managing
 * and holding groups of RegisteredType objects used for recording
 * during data acquisition.
 */

class RecordingObjects
{
public:
  /**
   * @brief Constructor for RecordingObjects class.
   */
  RecordingObjects();

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  RecordingObjects(const RecordingObjects&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  RecordingObjects& operator=(const RecordingObjects&) = delete;

  /**
   * @brief Destructor for RecordingObjects class.
   */
  ~RecordingObjects();

  /**
   * @brief Adds a RegisteredType object to the recording objects collection.
   * @param object The RegisteredType object to add as a shared pointer.
   */
  void addRecordingObject(std::shared_ptr<RegisteredType> object);

  /**
   * @brief Gets the RegisteredType object from the recording objects collection
   * @param objectInd The index of the object within the collection.
   */
  std::shared_ptr<RegisteredType> getRecordingObject(const SizeType& objectInd);

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
   * @brief Finalize all RegisteredType objects managed by this RecordingObjects instance.
   * This method calls finalize() on all objects in the collection.
   * @return The status of the finalize operation.
   */
  Status finalize();

  /**
   * @brief Get the number of recording objects
   */
  inline SizeType size() const { return m_recording_objects.size(); }

private:
  /**
   * @brief The RegisteredType objects used for recording
   */
  std::vector<std::shared_ptr<RegisteredType>> m_recording_objects;

  /**
   * @brief The name of the collection of recording objects
   */
  std::string m_name;
};

}  // namespace AQNWB::NWB
