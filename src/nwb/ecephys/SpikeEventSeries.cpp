#include "nwb/ecephys/SpikeEventSeries.hpp"

using namespace AQNWB::NWB;

// SpikeEventSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(SpikeEventSeries)

/** Constructor */
SpikeEventSeries::SpikeEventSeries(const std::string& path,
                                   std::shared_ptr<IO::BaseIO> io)
    : ElectricalSeries(path, io)
{
}

/** Destructor */
SpikeEventSeries::~SpikeEventSeries() {}

Status SpikeEventSeries::initialize(
    const IO::BaseArrayDataSetConfig& dataConfig,
    const Types::ChannelVector& channelVector,
    const std::string& description,
    const float& conversion,
    const float& resolution,
    const float& offset)
{
  auto esInitStatus = ElectricalSeries::initialize(
      dataConfig, channelVector, description, conversion, resolution, offset);
  this->m_eventsRecorded = 0;
  return esInitStatus;
}

Status SpikeEventSeries::writeSpike(const SizeType& numSamples,
                                    const SizeType& numChannels,
                                    const void* dataInput,
                                    const void* timestampsInput,
                                    const void* controlInput)
{
  // get offsets and datashape
  std::vector<SizeType> dataShape;
  std::vector<SizeType> positionOffset;
  if (numChannels == 1) {
    dataShape = {1, numSamples};
    positionOffset = {this->m_eventsRecorded, 0};
  } else {
    dataShape = {1, numChannels, numSamples};
    positionOffset = {this->m_eventsRecorded, 0, 0};
  }
  this->m_eventsRecorded += 1;

  // write channel data
  return writeData(
      dataShape, positionOffset, dataInput, timestampsInput, controlInput);
}
