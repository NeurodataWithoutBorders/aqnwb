#include "nwb/file/ElectrodeGroup.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// ElectrodeGroup
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ElectrodeGroup)

/** Constructor */
ElectrodeGroup::ElectrodeGroup(const std::string& path,
                               std::shared_ptr<IO::BaseIO> io)
    : NWBContainer(path, io)
{
}

/** Destructor */
ElectrodeGroup::~ElectrodeGroup() {}

Status ElectrodeGroup::Spec::validate() const
{
  Status status = NWBContainer::Spec::validate();
  if (location.empty()) {
    std::cerr << "ElectrodeGroup::Spec::validate: location must not be empty."
              << std::endl;
    status = Status::Failure;
  }
  if (device == nullptr) {
    std::cerr << "ElectrodeGroup::Spec::validate: device must be set."
              << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status ElectrodeGroup::initialize(const std::string& description,
                                  const std::string& location,
                                  const std::shared_ptr<Device>& device)
{
  return initialize(Spec()
                        .withDescription(description)
                        .withLocation(location)
                        .withDevice(device));
}

Status ElectrodeGroup::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "ElectrodeGroup::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  auto ctrInitStatus = NWBContainer::initialize(spec);
  if (ctrInitStatus != Status::Success) {
    return Status::Failure;
  }
  if (spec.description != "")
    ioPtr->createAttribute(spec.description, m_path, "description");
  ioPtr->createAttribute(spec.location, m_path, "location");
  ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "device"),
                    AQNWB::mergePaths("/", spec.device->getPath()));
  return ctrInitStatus;
}
