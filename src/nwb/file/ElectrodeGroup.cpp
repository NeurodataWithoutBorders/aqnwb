#include "nwb/file/ElectrodeGroup.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// ElectrodeGroup
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ElectrodeGroup)

/** Constructor */
ElectrodeGroup::ElectrodeGroup(const std::string& path,
                               std::shared_ptr<IO::BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
ElectrodeGroup::~ElectrodeGroup() {}

void ElectrodeGroup::initialize(const std::string& description,
                                const std::string& location,
                                const Device& device)
{
  Container::initialize();
  if (description != "")
    m_io->createAttribute(description, m_path, "description");
  m_io->createAttribute(location, m_path, "location");
  m_io->createLink(AQNWB::mergePaths("/" + m_path, "device"),
                   AQNWB::mergePaths("/", device.getPath()));
}
