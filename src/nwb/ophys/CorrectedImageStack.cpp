#include "nwb/ophys/CorrectedImageStack.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// CorrectedImageStack
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(CorrectedImageStack)

/** Constructor */
CorrectedImageStack::CorrectedImageStack(const std::string& path,
                                         std::shared_ptr<IO::BaseIO> io)
    : NWBDataInterface(path, io)
{
}

/** Destructor */
CorrectedImageStack::~CorrectedImageStack() {}

Status CorrectedImageStack::Spec::validate() const
{
  Status status = NWBDataInterface::Spec::validate();
  if (original == nullptr) {
    std::cerr << "CorrectedImageStack::Spec::validate: original must be set."
              << std::endl;
    status = Status::Failure;
  }
  status = status && corrected.validate();
  status = status && xyTranslation.validate();
  if (xyTranslation.data != nullptr && !xyTranslation.data->isLink()) {
    SizeArray shape, chunking;
    IO::BaseDataType dataType;
    if (xyTranslation.data->getProperties(nullptr, shape, chunking, dataType)
            == Status::Success
        && (shape.size() != 2 || shape[1] != 2))
    {
      std::cerr << "CorrectedImageStack::Spec::validate: the data of "
                   "xyTranslation must have the shape [frames, 2]."
                << std::endl;
      status = Status::Failure;
    }
  }
  return status;
}

Status CorrectedImageStack::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "CorrectedImageStack::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }
  Status status = NWBDataInterface::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }

  auto corrected = spec.corrected.createAndInitialize(
      AQNWB::mergePaths(m_path, "corrected"), ioPtr);
  auto xyTranslation = spec.xyTranslation.createAndInitialize(
      AQNWB::mergePaths(m_path, "xy_translation"), ioPtr);
  if (corrected == nullptr || xyTranslation == nullptr) {
    std::cerr << "CorrectedImageStack::initialize: Failed to create the "
                 "corrected or xy_translation series."
              << std::endl;
    return Status::Failure;
  }
  return ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "original"),
                           AQNWB::mergePaths("/", spec.original->getPath()));
}
