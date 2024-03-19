#include "ElectrodeGroup.hpp"

// ElectrodeGroup
// /** Constructor */
ElectrodeGroup::ElectrodeGroup(std::string path, std::shared_ptr<BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
ElectrodeGroup::~ElectrodeGroup() {}

void ElectrodeGroup::initialize()
{
  io->createCommonNWBAttributes(path, "core", "ElectrodeGroup", description);
  io->createAttribute(location, path, "location");
}

void ElectrodeGroup::linkDevice()
{
  io->createLink("/" + path + "/device", "/" + device);
}
