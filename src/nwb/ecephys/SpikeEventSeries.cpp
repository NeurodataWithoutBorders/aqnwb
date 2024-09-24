#include "nwb/ecephys/SpikeEventSeries.hpp"

using namespace AQNWB::NWB;

// SpikeEventSeries

/** Constructor */
SpikeEventSeries::SpikeEventSeries(const std::string& path,
                                   std::shared_ptr<IO::BaseIO> io)
    : ElectricalSeries(path, io)
{
}

/** Destructor */
SpikeEventSeries::~SpikeEventSeries() {}

void SpikeEventSeries::initialize(const IO::BaseDataType& dataType,
                                  const Types::ChannelVector& channelVector,
                                  const std::string& description,
                                  const SizeArray& dsetSize,
                                  const SizeArray& chunkSize,
                                  const float& conversion,
                                  const float& resolution,
                                  const float& offset)
{
  ElectricalSeries::initialize(dataType,
                               channelVector,
                               description,
                               dsetSize,
                               chunkSize,
                               conversion,
                               resolution,
                               offset);
  this->eventsRecorded = 0;
}

Status SpikeEventSeries::writeSpike(const SizeType& numSamples,
                                    const SizeType& numChannels,
                                    const void* data,
                                    const void* timestamps)
{
  // get offsets and datashape
  std::vector<SizeType> dataShape;
  std::vector<SizeType> positionOffset;
  if (numChannels == 1) {
    dataShape = {1, numSamples};
    positionOffset = {this->eventsRecorded, 0};
  } else {
    dataShape = {1, numChannels, numSamples};
    positionOffset = {this->eventsRecorded, 0, 0};
  }
  this->eventsRecorded += 1;

  // write channel data
  return writeData(dataShape, positionOffset, data, timestamps);
}