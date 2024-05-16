#include "nwb/hdmf/table/DynamicTableRegion.hpp"

using namespace AQNWB::NWB;

// DynamicTable

/** Constructor */
DynamicTableRegion::DynamicTableRegion(const std::string& path,
                                       std::shared_ptr<BaseIO> io,
                                       const std::string& tablePath,
                                       const std::string& description)
    : path(path)
    , io(io)
    , tablePath(tablePath)
    , description(description)
{
}

/** Destructor */
DynamicTableRegion::~DynamicTableRegion() {}

/** Initialization function*/
void DynamicTableRegion::initialize()
{
  // io->createGroup(path);  // TODO - this should maybe be on VectorData
  // initialization? But I'm not sure
  io->createCommonNWBAttributes(
      path, "hdmf-common", neurodataType, description);
  io->createReferenceAttribute(tablePath, path, "table");
}
