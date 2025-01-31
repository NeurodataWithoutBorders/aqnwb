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
Status Container::initialize()
{
  m_io->createGroup(m_path);
  // setup common attributes
  m_io->createCommonNWBAttributes(
      m_path, this->getNamespace(), this->getTypeName());
  return Status::Success;
}
