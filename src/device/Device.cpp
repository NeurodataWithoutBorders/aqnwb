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
  io->setCommonNWBAttributes(path, "core", "Device", description);
  io->setAttribute(manufacturer, path, "manufacturer");
}
