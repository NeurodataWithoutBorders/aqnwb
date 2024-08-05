#include "nwb/ecephys/SpikeEventSeries.hpp"

using namespace AQNWB::NWB;

// SpikeEventSeries

/** Constructor */
SpikeEventSeries::SpikeEventSeries(const std::string& path,
                                   std::shared_ptr<BaseIO> io,
                                   const BaseDataType& dataType,
                                   const Types::ChannelVector& channelVector,
                                   const std::string& electrodesTablePath,
                                   const std::string& unit,
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
                 electrodesTablePath,
                 unit,
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
