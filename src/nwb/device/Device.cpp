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

Status Device::initialize(const std::string& description,
                          const std::string& manufacturer)
{
  return initialize(
      Spec().withDescription(description).withManufacturer(manufacturer));
}

Status Device::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Device::initialize IO object has been deleted." << std::endl;
    return Status::Failure;
  }
  auto ctrInitStatus = NWBContainer::initialize(spec);
  if (ctrInitStatus != Status::Success) {
    return Status::Failure;
  }

  if (spec.description != "")
    ctrInitStatus = ctrInitStatus
        && ioPtr->createAttribute(spec.description, m_path, "description");
  if (spec.manufacturer.has_value())
    ctrInitStatus = ctrInitStatus
        && ioPtr->createAttribute(
            spec.manufacturer.value(), m_path, "manufacturer");
  return ctrInitStatus;
}
