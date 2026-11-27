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
    const float& offset,
    const bool useChannelConversion)
{
  Spec spec;
  // Non-owning pointer to the dataConfig, which outlives the call to
  // initialize
  spec.data = Spec::DataConfigPtr(std::shared_ptr<void>(), &dataConfig);
  spec.channelVector = channelVector;
  spec.description = description;
  if (!channelVector.empty()) {
    spec.comments = channelVector[0].getComments();
  }
  spec.conversion = conversion;
  spec.resolution = resolution;
  spec.offset = offset;
  spec.useChannelConversion = useChannelConversion;
  return initialize(spec);
}

Status SpikeEventSeries::initialize(const Spec& spec)
{
  auto esInitStatus = ElectricalSeries::initialize(spec);
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
  SizeArray dataShape;
  SizeArray positionOffset;
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
