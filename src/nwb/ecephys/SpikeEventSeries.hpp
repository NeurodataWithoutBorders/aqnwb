#pragma once

#include <string>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Stores snapshots/snippets of recorded spike events (i.e., threshold
 * crossings).
 */
class SpikeEventSeries : public ElectricalSeries
{
public:
  /**
   * @brief Constructor.
   * @param path The location of the SpikeEventSeries in the file.
   * @param io A shared pointer to the IO object.
   * @param description The description of the SpikeEventSeries, should describe
   * how events were detected.
   */
  SpikeEventSeries(const std::string& path,
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
  ~SpikeEventSeries();

  /**
   * @brief Write a single spike series event
   * @param channelInd The channel index within the SpikeEventSeries
   * @param numSamples The number of samples in the event
   * @param numChannels The number of channels in the event
   * @param data The data of the event
   * @param timestamps The timestamps of the event
   * @param
   */
   Status writeSpike(SizeType channelInd,
                   const SizeType& numSamples,
                   const SizeType& numChannels,
                   const void* data,
                   const void* timestamps);

private:
  /**
   * @brief The neurodataType of the SpikeEventSeries.
   */
  std::string neurodataType = "SpikeEventSeries";

  /**
   * @brief The number of events already written.
   */
  SizeArray eventsRecorded;
};
}  // namespace AQNWB::NWB
