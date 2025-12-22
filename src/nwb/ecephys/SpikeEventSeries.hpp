#pragma once

#include <string>

#include "Channel.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Stores snapshots/snippets of recorded spike events (i.e., threshold
 * crossings).
 */
class SpikeEventSeries : public ElectricalSeries
{
public:
  // Register the TimeSeries as a subclass of Container
  REGISTER_SUBCLASS(SpikeEventSeries,
                    ElectricalSeries,
                    AQNWB::SPEC::CORE::namespaceName)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the SpikeEventSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  SpikeEventSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Destructor
   */
  ~SpikeEventSeries();

  /**
   * @brief Initializes the SpikeEventSeries
   *
   * @param dataConfig Configuration for the dataset including data type, shape
   * and chunking. The shape must be a vector with two elements. The first
   * element specifies the length in time and the second element must be equal
   * to the length of channelVector. The chunking must also be a vector with two
   * elements to specify the size of a chunk in the time and electrode
   * dimension.
   * @param channelVector The electrodes to use for recording
   * @param description The description of the SpikeEventSeries, should describe
   *                   how events were detected.
   * @param conversion Scalar to multiply each element in data to convert it to
   *                   the specified 'unit'
   * @param resolution Smallest meaningful difference between values in data,
   *                   stored in the specified by unit
   * @param offset Scalar to add to the data after scaling by 'conversion' to
   *               finalize its coercion to the specified 'unit'
   */
  Status initialize(const IO::ArrayDataSetConfig& dataConfig,
                    const Types::ChannelVector& channelVector,
                    const std::string& description,
                    const float& conversion = 1.0f,
                    const float& resolution = -1.0f,
                    const float& offset = 0.0f);

  /**
   * @brief Write a single spike series event
   *
   * Timestamp and controlInput values are only written if the channel index is
   * 0.
   *
   * @param numSamples The number of samples in the event
   * @param numChannels The number of channels in the event
   * @param dataInput The data of the event
   * @param timestampsInput The timestamps of the event
   * @param controlInput A pointer to the control block data (optional)
   */
  Status writeSpike(const SizeType& numSamples,
                    const SizeType& numChannels,
                    const void* dataInput,
                    const void* timestampsInput,
                    const void* controlInput = nullptr);

  DEFINE_DATASET_FIELD(readData, recordData, std::any, "data", Spike waveforms)

  DEFINE_ATTRIBUTE_FIELD(readDataUnit,
                        std::string,
                        "data/unit",
                        Unit of measurement for waveforms.
                        This is fixed to volts)

private:
  /**
   * @brief The number of events already written.
   */
  SizeType m_eventsRecorded;
};
}  // namespace AQNWB::NWB
