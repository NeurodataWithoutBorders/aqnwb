#pragma once

#include <string>

#include "Channel.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{
/**
 * @brief General purpose time series.
 */
class ElectricalSeries : public TimeSeries
{
public:
  // Register the TimeSeries as a subclass of Container
  REGISTER_SUBCLASS(ElectricalSeries, "core")

  /**
   * @brief Constructor.
   * @param path The location of the ElectricalSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  ElectricalSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor
   */
  ~ElectricalSeries();

  /**
   * @brief Initializes the Electrical Series
   *
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
  void initialize(const IO::BaseDataType& dataType,
                  const Types::ChannelVector& channelVector,
                  const std::string& description,
                  const SizeArray& dsetSize,
                  const SizeArray& chunkSize,
                  const float& conversion = 1.0f,
                  const float& resolution = -1.0f,
                  const float& offset = 0.0f);

  /**
   * @brief Writes a channel to an ElectricalSeries dataset.
   *
   * Timestamp and controlInput values are only written if the channel index is
   * 0.
   *
   * @param channelInd The channel index within the ElectricalSeries
   * @param numSamples The number of samples to write (length in time).
   * @param dataInput A pointer to the data block.
   * @param timestampsInput A pointer to the timestamps block.
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeChannel(SizeType channelInd,
                      const SizeType& numSamples,
                      const void* dataInput,
                      const void* timestampsInput,
                      const void* controlInput = nullptr);

  /**
   * @brief Channel group that this time series is associated with.
   */
  Types::ChannelVector channelVector;

  /**
   * @brief Pointer to channel-specific conversion factor dataset.
   */
  std::unique_ptr<IO::BaseRecordingData> channelConversion;

  /**
   * @brief Pointer to electrodes dataset.
   */
  std::unique_ptr<IO::BaseRecordingData> electrodesDataset;

  DEFINE_FIELD(readChannelConversion,
               AttributeField,
               float,
               "data/channel_conversion",
               Channel - specific conversion factor)

  DEFINE_FIELD(readData, DatasetField, std::any, "data", Recorded voltage data)

  DEFINE_FIELD(readDataUnit,
               AttributeField,
               std::string,
               "data/unit",
               Base unit of measurement for working with the data. 
               This value is fixed to volts)

private:
  /**
   * @brief The number of samples already written per channel.
   */
  SizeArray m_samplesRecorded;
};
}  // namespace AQNWB::NWB
