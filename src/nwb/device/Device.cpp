#include "nwb/device/Device.hpp"

using namespace AQNWB::NWB;

// Device
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(Device)

/** Constructor */
Device::Device(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : NWBContainer(path, io)
{
}

/** Destructor */
Device::~Device() {}

Status Device::initialize(const DeviceSpec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Device::initialize IO object has been deleted." << std::endl;
    return Status::Failure;
  }

  Status status = NWBContainer::initialize();
  if (spec.description.has_value()) {
    status = status
        && ioPtr->createAttribute(
            spec.description.value(), m_path, "description");
  }
  if (spec.serialNumber.has_value()) {
    status = status
        && ioPtr->createAttribute(
            spec.serialNumber.value(), m_path, "serial_number");
  }
  if (spec.model != nullptr) {
    status = status
        && ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "model"),
                             AQNWB::mergePaths("/", spec.model->getPath()));
  }
  return status;
}

Status Device::initialize(const std::string& description,
                          const std::string& manufacturer)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Device::initialize IO object has been deleted." << std::endl;
    return Status::Failure;
  }

  DeviceSpec spec;
  if (description != "") {
    spec.withDescription(description);
  }
  Status status = initialize(spec);
  return status && ioPtr->createAttribute(manufacturer, m_path, "manufacturer");
}
