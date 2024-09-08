#include "nwb/file/ElectrodeGroup.hpp"

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

  io->createCommonNWBAttributes(path, "core", "ElectrodeGroup", description);
  io->createAttribute(location, path, "location");
  io->createLink("/" + path + "/device", "/" + device.getPath());
}
