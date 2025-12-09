#include "NWBContainer.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(NWBContainer)

NWBContainer::NWBContainer(const std::string& path,
                           std::shared_ptr<AQNWB::IO::BaseIO> io)
    : Container(path, io)
{
}

Status NWBContainer::initialize()
{
  return AQNWB::NWB::Container::initialize();
}
