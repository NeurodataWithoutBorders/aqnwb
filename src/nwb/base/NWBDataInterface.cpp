#include "NWBDataInterface.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(NWBDataInterface)

NWBDataInterface::NWBDataInterface(const std::string& path,
                                   std::shared_ptr<AQNWB::IO::BaseIO> io)
    : NWBContainer(path, io)
{
}

Status NWBDataInterface::initialize()
{
  return NWBContainer::initialize();
}
