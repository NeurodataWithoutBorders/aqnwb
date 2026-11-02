#include "nwb/device/DeviceModel.hpp"

using namespace AQNWB::NWB;

// DeviceModel
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(DeviceModel)

/** Constructor */
DeviceModel::DeviceModel(const std::string& path,
                         std::shared_ptr<IO::BaseIO> io)
    : NWBContainer(path, io)
{
}

/** Destructor */
DeviceModel::~DeviceModel() {}

Status DeviceModel::initialize(const DeviceModelSpec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "DeviceModel::initialize IO object has been deleted."
              << std::endl;
    return Status::Failure;
  }
  if (spec.manufacturer.empty()) {
    std::cerr << "DeviceModel::initialize the manufacturer is required."
              << std::endl;
    return Status::Failure;
  }

  Status status = NWBContainer::initialize();
  status = status
      && ioPtr->createAttribute(spec.manufacturer, m_path, "manufacturer");
  if (spec.modelNumber.has_value()) {
    status = status
        && ioPtr->createAttribute(
            spec.modelNumber.value(), m_path, "model_number");
  }
  if (spec.description.has_value()) {
    status = status
        && ioPtr->createAttribute(
            spec.description.value(), m_path, "description");
  }
  return status;
}
