#pragma once

#include <string>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Stores snapshots/snippets of recorded spike events (i.e., threshold crossings). 
 */
class SpikeEventSeries : public ElectricalSeries
{
public:
  /**
   * @brief Constructor.
   * @param path The location of the SpikeEventSeries in the file.
   * @param io A shared pointer to the IO object.
   * @param description The description of the SpikeEventSeries, should describe how events were detected.
   */
  SpikeEventSeries(const std::string& path,
                   std::shared_ptr<BaseIO> io,
                   const BaseDataType& dataType,
                   const Types::ChannelVector& channelVector,
                   const std::string& electrodesTablePath,
                   const std::string& unit = "volts",
                   const std::string& description = "no description",
                   const SizeArray& dsetSize = SizeArray {0},
                   const SizeArray& chunkSize = SizeArray {1},
                   const float& conversion = 1.0f,
                   const float& resolution = -1.0f,
                   const float& offset = 0.0f);

  /**
   * @brief Destructor
   */
  ~SpikeEventSeries();

private:
  /**
   * @brief The neurodataType of the SpikeEventSeries.
   */
  std::string neurodataType = "SpikeEventSeries";
};
}  // namespace AQNWB::NWB
