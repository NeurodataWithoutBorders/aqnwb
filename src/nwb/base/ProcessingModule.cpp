#include "nwb/base/ProcessingModule.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ProcessingModule)

ProcessingModule::ProcessingModule(const std::string& path,
                                   std::shared_ptr<AQNWB::IO::BaseIO> io)
    : NWBContainer(path, io)
{
}

Status ProcessingModule::initialize(const std::string& description)
{
  return initialize(Spec().withDescription(description));
}

Status ProcessingModule::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "ProcessingModule::initialize IO object has been deleted."
              << std::endl;
    return Status::Failure;
  }

  Status initStatus = NWBContainer::initialize(spec);
  if (initStatus != Status::Success) {
    return Status::Failure;
  }

  Status descrStatus =
      ioPtr->createAttribute(spec.description, m_path, "description");
  initStatus = initStatus && descrStatus;

  return initStatus;
}
