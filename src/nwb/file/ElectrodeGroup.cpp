#include "nwb/file/ElectrodeGroup.hpp"

using namespace AQNWB::NWB;

// ElectrodeGroup

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

  m_io->createCommonNWBAttributes(
      m_path, "core", "ElectrodeGroup", description);
  m_io->createAttribute(location, m_path, "location");
  m_io->createLink("/" + m_path + "/device", "/" + device.getPath());
}
