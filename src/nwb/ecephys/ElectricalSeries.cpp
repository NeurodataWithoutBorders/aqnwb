#include "nwb/ecephys/ElectricalSeries.hpp"

#include "nwb/hdmf/table/DynamicTableRegion.hpp"

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

  // std::unique_ptr<DynamicTableRegion> electrodes =
  // std::make_unique<DynamicTableRegion>(getPath() + "/electrodes", io,
  // electrodesTablePath, "Electrode index for each channel");
  // electrodes->initialize();
}
