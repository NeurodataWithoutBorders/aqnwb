#pragma once

#include <string>

#include "Channel.hpp"
#include "io/BaseIO.hpp"
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
   */
  SpikeEventSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor
   */
  ~SpikeEventSeries();

  /**
   * @brief Initializes the SpikeEventSeries
   *
   * @param dataType The data type to use for storing the recorded voltage
   * @param channelVector The electrodes to use for recording
   * @param description The description of the SpikeEventSeries, should describe
   * how events were detected.
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
   * @brief Write a single spike series event
   *
   * @param numSamples The number of samples in the event
   * @param numChannels The number of channels in the event
   * @param data The data of the event
   * @param timestamps The timestamps of the event
   */
  Status writeSpike(const SizeType& numSamples,
                    const SizeType& numChannels,
                    const void* data,
                    const void* timestamps);

private:
  /**
   * @brief The number of events already written.
   */
  SizeType eventsRecorded;
};
}  // namespace AQNWB::NWB
