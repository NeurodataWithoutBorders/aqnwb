#include "nwb/file/ElectrodeGroup.hpp"

using namespace AQNWB::NWB;

// ElectrodeGroup

/** Constructor */
ElectrodeGroup::ElectrodeGroup(const std::string& path,
                               std::shared_ptr<BaseIO> io,
                               const std::string& description,
                               const std::string& location,
                               const Device& device)
    : Container(path, io)
    , description(description)
    , location(location)
    , device(device)
{
}

/** Destructor */
ElectrodeGroup::~ElectrodeGroup() {}

void ElectrodeGroup::initialize()
{
  Container::initialize();

  this->m_io->createCommonNWBAttributes(this->m_path, "core", "ElectrodeGroup", description);
  this->m_io->createAttribute(location, this->m_path, "location");
  this->m_io->createLink("/" + this->m_path + "/device", "/" + device.getPath());
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
const Device& ElectrodeGroup::getDevice() const
{
  return device;
}
