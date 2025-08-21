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

Status Device::initialize(const std::string& description,
                        const std::string& manufacturer)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Device::initialize IO object has been deleted." << std::endl;
    return Status::Failure;
  }
  auto ctrInitStatus = Container::initialize();

  ioPtr->createCommonNWBAttributes(
      m_path, this->getNamespace(), this->getTypeName());
  if (description != "")
    ioPtr->createAttribute(description, m_path, "description");
  ioPtr->createAttribute(manufacturer, m_path, "manufacturer");
  return ctrInitStatus;
}
