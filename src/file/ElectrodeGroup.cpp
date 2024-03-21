#include "ElectrodeGroup.hpp"

// ElectrodeGroup

/** Constructor */
ElectrodeGroup::ElectrodeGroup(const std::string& path, std::shared_ptr<BaseIO> io, const std::string& description, const std::string& location, const Device& device)
  : Container(path, io), description(description), location(location), device(device)
{
}

/** Destructor */
ElectrodeGroup::~ElectrodeGroup() {}

void ElectrodeGroup::initialize()
{
  io->createCommonNWBAttributes(path, "core", "ElectrodeGroup", description);
  io->createAttribute(location, path, "location");
  io->createLink("/" + path + "/device", "/" + device.getPath());

}

// Getter for description
std::string ElectrodeGroup::getDescription() const
{
  return description;
}

// Getter for location
std::string ElectrodeGroup::getLocation() const
{
  return location;
}

// Getter for device
Device ElectrodeGroup::getDevice() const
{
  return device;
}
