#include "nwb/hdmf/base/Container.hpp"

using namespace AQNWB::NWB;

// Container
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(Container)

/** Constructor */
Container::Container(const std::string& path,
                     std::shared_ptr<AQNWB::IO::BaseIO> io)
    : RegisteredType(path, io)
{
}

/** Destructor */
Container::~Container() {}

/** Initialize */
void Container::initialize()
{
  io->createGroup(path);
}
