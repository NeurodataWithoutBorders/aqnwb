#include "nwb/ophys/OpticalChannel.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// OpticalChannel
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(OpticalChannel)

/** Constructor */
OpticalChannel::OpticalChannel(const std::string& path,
                               std::shared_ptr<IO::BaseIO> io)
    : NWBContainer(path, io)
{
}

/** Destructor */
OpticalChannel::~OpticalChannel() {}

Status OpticalChannel::Spec::validate() const
{
  Status status = NWBContainer::Spec::validate();
  if (description.empty()) {
    std::cerr << "OpticalChannel::Spec::validate: description must not be "
                 "empty."
              << std::endl;
    status = Status::Failure;
  }
  if (!(emissionLambda > 0.0f)) {
    std::cerr << "OpticalChannel::Spec::validate: emissionLambda must be > 0."
              << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status OpticalChannel::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "OpticalChannel::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  // Validate the spec and create the group
  Status status = NWBContainer::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }

  status = status
      && ioPtr->createStringDataSet(AQNWB::mergePaths(m_path, "description"),
                                    spec.description);
  status = status
      && ioPtr->createScalarDataSet(
          IO::BaseDataType::F32,
          &spec.emissionLambda,
          AQNWB::mergePaths(m_path, "emission_lambda"));
  return status;
}
