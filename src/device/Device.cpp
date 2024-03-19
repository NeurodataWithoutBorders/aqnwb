#include "Device.hpp"

// Device
/** Constructor */
Device::Device(std::string path, std::shared_ptr<BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
Device::~Device() {}

void Device::initialize()
{
  io->createCommonNWBAttributes(path, "core", "Device", description);
  io->createAttribute(manufacturer, path, "manufacturer");
}
