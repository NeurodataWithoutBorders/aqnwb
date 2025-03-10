#pragma once

#include <string>

#include "Channel.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/file/ElectrodeTable.hpp"

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
   * @param dataConfig Configuration for the dataset including data type, shape
   * and chunking. The shape must be a vector with two elements. The first
   * element specifies the length in time and the second element must be equal
   * to the length of channelVector. The chunking must also be a vector with two
   * elements to specify the size of a chunk in the time and electrode
   * dimension.
   * @param channelVector The electrodes to use for recording
   * @param description The description of the TimeSeries.
   * @param conversion Scalar to multiply each element in data to convert it to
   *                   the specified 'unit'
   * @param resolution Smallest meaningful difference between values in data,
   *                   stored in the specified by unit
   * @param offset Scalar to add to the data after scaling by 'conversion' to
   *               finalize its coercion to the specified 'unit'
   * @return The status of the initialization operation.
   */
  Status initialize(const IO::ArrayDataSetConfig& dataConfig,
                  const Types::ChannelVector& channelVector,
                  const std::string& description,
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
  Types::ChannelVector m_channelVector;

  /**
   * @brief Pointer to channel-specific conversion factor dataset.
   */
  std::unique_ptr<IO::BaseRecordingData> m_channelConversion;

  /**
   * @brief Pointer to electrodes dataset.
   */
  std::unique_ptr<IO::BaseRecordingData> m_electrodesDataset;

  DEFINE_FIELD(readChannelConversion,
               AttributeField,
               float,
               "data/channel_conversion",
               Channel - specific conversion factor)

  DEFINE_FIELD(readData, DatasetField, float, "data", Recorded voltage data)

  DEFINE_FIELD(readDataUnit,
               AttributeField,
               std::string,
               "data/unit",
               Base unit of measurement for working with the data. 
               This value is fixed to volts)

  DEFINE_FIELD(
      readElectrodes,
      DatasetField,
      int,
      "electrodes",
      The indices of the electrodes that generated this electrical series.)

  DEFINE_FIELD(readElectrodesDescription,
               AttributeField,
               std::string,
               "electrodes/description",
               The electrodes that generated this electrical series.)

  DEFINE_REFERENCED_REGISTERED_FIELD(
      readElectrodesTable,
      ElectrodeTable,
      "electrodes/table",
      The electrodes table retrieved from the object referenced in the 
      `electrodes / table` attribute.)

private:
  /**
   * @brief The number of samples already written per channel.
   */
  SizeArray m_samplesRecorded;
};
}  // namespace AQNWB::NWB
