#include "nwb/hdmf/base/Container.hpp"

using namespace AQNWB::NWB;

// Container

/** Constructor */
Container::Container(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : m_path(path)
    , m_io(io)
{
}

/** Destructor */
Container::~Container() {}

/** Initialize */
void Container::initialize()
{
  m_io->createGroup(m_path);
}
