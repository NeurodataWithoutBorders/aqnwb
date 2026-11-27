#include "nwb/ogen/OptogeneticStimulusSite.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// OptogeneticStimulusSite
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(OptogeneticStimulusSite)

/** Constructor */
OptogeneticStimulusSite::OptogeneticStimulusSite(const std::string& path,
                                                 std::shared_ptr<IO::BaseIO> io)
    : NWBContainer(path, io)
{
}

/** Destructor */
OptogeneticStimulusSite::~OptogeneticStimulusSite() {}

Status OptogeneticStimulusSite::Spec::validate() const
{
  Status status = NWBContainer::Spec::validate();
  if (description.empty()) {
    std::cerr << "OptogeneticStimulusSite::Spec::validate: description must "
                 "not be empty."
              << std::endl;
    status = Status::Failure;
  }
  if (!(excitationLambda > 0.0f)) {
    std::cerr << "OptogeneticStimulusSite::Spec::validate: excitationLambda "
                 "must be > 0."
              << std::endl;
    status = Status::Failure;
  }
  if (location.empty()) {
    std::cerr << "OptogeneticStimulusSite::Spec::validate: location must not "
                 "be empty."
              << std::endl;
    status = Status::Failure;
  }
  if (device == nullptr) {
    std::cerr << "OptogeneticStimulusSite::Spec::validate: device must be set."
              << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status OptogeneticStimulusSite::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "OptogeneticStimulusSite::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  // Check the link targets before anything is written
  if (checkLinkTarget(spec.device, "device") != Status::Success) {
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
          &spec.excitationLambda,
          AQNWB::mergePaths(m_path, "excitation_lambda"));
  status = status
      && ioPtr->createStringDataSet(AQNWB::mergePaths(m_path, "location"),
                                    spec.location);
  status = status
      && ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "device"),
                           AQNWB::mergePaths("/", spec.device->getPath()));
  return status;
}
