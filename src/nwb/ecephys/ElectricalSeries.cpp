#include "nwb/ecephys/ElectricalSeries.hpp"

using namespace AQNWB::NWB;

// ElectricalSeries

/** Constructor */
ElectricalSeries::ElectricalSeries(const std::string& path,
                                   std::shared_ptr<BaseIO> io,
                                   const std::string& description,
                                   const std::string& electrodesTablePath)
    : TimeSeries(path, io, description)
    , electrodesTablePath(electrodesTablePath)
{
}

/** Destructor */
ElectricalSeries::~ElectricalSeries() {}

/** Initialization function*/
void ElectricalSeries::initialize()
{
  TimeSeries::initialize();
}
