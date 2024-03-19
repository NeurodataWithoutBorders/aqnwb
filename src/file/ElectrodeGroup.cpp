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
  io->setGroupAttributes(path, "core", "ElectrodeGroup", description);
  io->setAttribute(location, path, "location");
}

void ElectrodeGroup::linkDevice()
{
  io->createLink("/" + path + "/device", "/" + device);
}
