#pragma once

#include <string>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{
/**
 * @brief General purpose time series.
 */
class ElectricalSeries : public TimeSeries
{
public:
  /**
   * @brief Constructor.
   * @param path The location of the ElectricalSeries in the file.
   * @param io A shared pointer to the IO object.
   * @param dataType The data type to use for storing the recorded voltage
   * @param channelVector The electrodes to use for recording
   * @param description The description of the TimeSeries.
   * @param dsetSize Initial size of the main dataset. This must be a vector
   *                 with two elements. The first element specifies the length
   *                 in time and the second element must be equal to the
   *                 length of channelVector
   * @param chunkSize Chunk size to use. The number of elements must be two to
   *                  specify the size of a chunk in the time and electrode
   *                  dimension
   * @param conversion Scalar to multiply each element in data to convert it to
   *                   the specified ‘unit’
   * @param resolution Smallest meaningful difference between values in data,
   *                   stored in the specified by unit
   * @param offset Scalar to add to the data after scaling by ‘conversion’ to
   *               finalize its coercion to the specified ‘unit'
   */
  ElectricalSeries(const std::string& path,
                   std::shared_ptr<BaseIO> io,
                   const BaseDataType& dataType,
                   const Types::ChannelVector& channelVector,
                   const std::string& description,
                   const SizeArray& dsetSize,
                   const SizeArray& chunkSize,
                   const float& conversion = 1.0f,
                   const float& resolution = -1.0f,
                   const float& offset = 0.0f);

  /**
   * @brief Destructor
   */
  ~ElectricalSeries();

  /**
   * @brief Initializes the Electrical Series
   */
  void initialize();

  /**
   * @brief Writes a channel to an ElectricalSeries dataset.
   * @param channelInd The channel index within the ElectricalSeries
   * @param numSamples The number of samples to write (length in time).
   * @param data A pointer to the data block.
   * @param timestamps A pointer to the timestamps block.
   * @return The status of the write operation.
   */
  Status writeChannel(SizeType channelInd,
                      const SizeType& numSamples,
                      const void* data,
                      const void* timestamps);

  /**
   * @brief Channel group that this time series is associated with.
   */
  Types::ChannelVector channelVector;

  /**
   * @brief Pointer to channel-specific conversion factor dataset.
   */
  std::unique_ptr<BaseRecordingData> channelConversion;

  /**
   * @brief Pointer to electrodes dataset.
   */
  std::unique_ptr<BaseRecordingData> electrodesDataset;

private:
  /**
   * @brief The neurodataType of the TimeSeries.
   */
  std::string neurodataType = "ElectricalSeries";

  /**
   * @brief The number of samples already written per channel.
   */
  SizeArray samplesRecorded;
};
}  // namespace AQNWB::NWB
