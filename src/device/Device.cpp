#include "Device.hpp"

// Device
/** Constructor */
Device::Device(std::string path, std::shared_ptr<BaseIO> io, std::string description, std::string manufacturer)
  : Container(path, io), description(description), manufacturer(manufacturer)
{
}

/** Destructor */
Device::~Device() {}

void Device::initialize()
{
  io->createCommonNWBAttributes(path, "core", "Device", description);
  io->createAttribute(manufacturer, path, "manufacturer");
}

// Getter for manufacturer
std::string Device::getManufacturer() const
{
  return manufacturer;
}

// Getter for description
std::string Device::getDescription() const
{
  return description;
}
