#include "nwb/device/Device.hpp"

using namespace AQNWB::NWB;

// Device
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(Device)

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

  m_io->createCommonNWBAttributes(
      m_path, this->getNamespace(), this->getTypeName());
  if (description != "")
    m_io->createAttribute(description, m_path, "description");
  m_io->createAttribute(manufacturer, m_path, "manufacturer");
}
