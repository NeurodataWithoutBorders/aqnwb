#include "nwb/device/Device.hpp"

using namespace AQNWB::NWB;

// Device
/** Constructor */
Device::Device(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
Device::~Device() {}

void Device::initialize(const std::string& description,
                        const std::string& manufacturer)
{
  Container::initialize();

  m_io->createCommonNWBAttributes(m_path, "core", "Device", description);
  m_io->createAttribute(manufacturer, m_path, "manufacturer");
}
