#include "nwb/file/ElectrodeGroup.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// ElectrodeGroup
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ElectrodeGroup)

/** Constructor */
ElectrodeGroup::ElectrodeGroup(const std::string& path,
                               std::shared_ptr<IO::BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
ElectrodeGroup::~ElectrodeGroup() {}

Status ElectrodeGroup::initialize(const std::string& description,
                                  const std::string& location,
                                  const std::shared_ptr<Device>& device)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "ElectrodeGroup::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  auto ctrInitStatus = Container::initialize();
  if (description != "")
    ioPtr->createAttribute(description, m_path, "description");
  ioPtr->createAttribute(location, m_path, "location");
  ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "device"),
                    AQNWB::mergePaths("/", device->getPath()));
  return ctrInitStatus;
}
