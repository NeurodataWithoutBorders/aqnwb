#include "nwb/device/Device.hpp"

using namespace AQNWB::NWB;

// Device
/** Constructor */
Device::Device(const std::string& path,
               std::shared_ptr<BaseIO> io,
               const std::string& description,
               const std::string& manufacturer)
    : Container(path, io)
    , description(description)
    , manufacturer(manufacturer)
{
}

/** Destructor */
Device::~Device() {}

void Device::initialize()
{
  Container::initialize();

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
