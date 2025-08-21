#include "nwb/hdmf/base/Container.hpp"

using namespace AQNWB::NWB;

// Container
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(Container)

/** Constructor */
Container::Container(const std::string& path,
                     std::shared_ptr<AQNWB::IO::BaseIO> io)
    : RegisteredType(path, io)
{
}

/** Destructor */
Container::~Container() {}

/** Initialize */
Status Container::initialize()
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Container::initialize IO object has been deleted." << std::endl;
    return AQNWB::Types::Status::Failure;
  }

  // Call RegisteredType::initialize() to add this object to RecordingObjects
  auto registerStatus = registerRecordingObject();

  auto createGroupStatus = ioPtr->createGroup(m_path);
  // setup common attributes
  auto createAttrsStatus = ioPtr->createCommonNWBAttributes(
      m_path, this->getNamespace(), this->getTypeName());

  return createGroupStatus && createAttrsStatus && registerStatus;
}
