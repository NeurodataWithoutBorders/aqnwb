#include "nwb/ecephys/SpikeEventSeries.hpp"

using namespace AQNWB::NWB;

// SpikeEventSeries

/** Constructor */
SpikeEventSeries::SpikeEventSeries(const std::string& path,
                                   std::shared_ptr<BaseIO> io,
                                   const BaseDataType& dataType,
                                   const Types::ChannelVector& channelVector,
                                   const std::string& description,
                                   const SizeArray& dsetSize,
                                   const SizeArray& chunkSize,
                                   const float& conversion,
                                   const float& resolution,
                                   const float& offset)
    : ElectricalSeries(path,
                       io,
                       dataType,
                       channelVector,
                       description,
                       dsetSize,
                       chunkSize,
                       conversion,
                       resolution,
                       offset)
{
}

/** Destructor */
SpikeEventSeries::~SpikeEventSeries() {}

void SpikeEventSeries::initialize()
{
  ElectricalSeries::initialize();

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
  }
  else {
    dataShape = {1, numChannels, numSamples};
    positionOffset = {this->eventsRecorded, 0, 0};
  }
  this->eventsRecorded += 1;

  // write channel data
  return writeData(dataShape, positionOffset, data, timestamps);

}