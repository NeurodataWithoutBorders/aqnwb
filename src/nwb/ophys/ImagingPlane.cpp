#include <set>

#include "nwb/ophys/ImagingPlane.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// ImagingPlane
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ImagingPlane)

namespace
{
/**
 * @brief Create a 1D float32 dataset with the given values.
 */
Status createFloatArrayDataSet(AQNWB::IO::BaseIO& io,
                               const std::string& path,
                               const std::vector<float>& values)
{
  AQNWB::IO::ArrayDataSetConfig config(AQNWB::IO::BaseDataType::F32,
                                       SizeArray {values.size()},
                                       SizeArray {values.size()});
  auto recorder = io.createArrayDataSet(config, path);
  if (recorder == nullptr) {
    return Status::Failure;
  }
  return recorder->writeDataBlock(
      SizeArray {values.size()}, AQNWB::IO::BaseDataType::F32, values.data());
}
}  // namespace

/** Constructor */
ImagingPlane::ImagingPlane(const std::string& path,
                           std::shared_ptr<IO::BaseIO> io)
    : NWBContainer(path, io)
{
}

/** Destructor */
ImagingPlane::~ImagingPlane() {}

Status ImagingPlane::Spec::validate() const
{
  Status status = NWBContainer::Spec::validate();
  if (!(excitationLambda > 0.0f)) {
    std::cerr << "ImagingPlane::Spec::validate: excitationLambda must be > 0."
              << std::endl;
    status = Status::Failure;
  }
  if (indicator.empty()) {
    std::cerr << "ImagingPlane::Spec::validate: indicator must not be empty."
              << std::endl;
    status = Status::Failure;
  }
  if (location.empty()) {
    std::cerr << "ImagingPlane::Spec::validate: location must not be empty."
              << std::endl;
    status = Status::Failure;
  }
  if (device == nullptr) {
    std::cerr << "ImagingPlane::Spec::validate: device must be set."
              << std::endl;
    status = Status::Failure;
  }
  if (originCoords.has_value() && originCoords->size() != 2
      && originCoords->size() != 3)
  {
    std::cerr << "ImagingPlane::Spec::validate: originCoords must have 2 or 3 "
                 "elements."
              << std::endl;
    status = Status::Failure;
  }
  if (gridSpacing.has_value() && gridSpacing->size() != 2
      && gridSpacing->size() != 3)
  {
    std::cerr << "ImagingPlane::Spec::validate: gridSpacing must have 2 or 3 "
                 "elements."
              << std::endl;
    status = Status::Failure;
  }
  if (opticalChannels.empty()) {
    std::cerr << "ImagingPlane::Spec::validate: at least one optical channel "
                 "is required."
              << std::endl;
    status = Status::Failure;
  }
  std::set<std::string> channelNames;
  for (const auto& [name, channelSpec] : opticalChannels) {
    if (name.empty() || name.find('/') != std::string::npos) {
      std::cerr << "ImagingPlane::Spec::validate: the name of an optical "
                   "channel must not be empty or contain '/'."
                << std::endl;
      status = Status::Failure;
    }
    if (!channelNames.insert(name).second) {
      std::cerr << "ImagingPlane::Spec::validate: duplicate optical channel "
                   "name '"
                << name << "'." << std::endl;
      status = Status::Failure;
    }
    // Validate all channels to report all problems
    const Status channelStatus = channelSpec.validate();
    status = status && channelStatus;
  }
  return status;
}

Status ImagingPlane::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "ImagingPlane::initialize: IO object is not valid."
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

  if (spec.description.has_value()) {
    status = status
        && ioPtr->createStringDataSet(AQNWB::mergePaths(m_path, "description"),
                                      spec.description.value());
  }
  status = status
      && ioPtr->createScalarDataSet(
          IO::BaseDataType::F32,
          &spec.excitationLambda,
          AQNWB::mergePaths(m_path, "excitation_lambda"));
  if (spec.imagingRate.has_value()) {
    const float imagingRate = spec.imagingRate.value();
    status = status
        && ioPtr->createScalarDataSet(
            IO::BaseDataType::F32,
            &imagingRate,
            AQNWB::mergePaths(m_path, "imaging_rate"));
  }
  status = status
      && ioPtr->createStringDataSet(AQNWB::mergePaths(m_path, "indicator"),
                                    spec.indicator);
  status = status
      && ioPtr->createStringDataSet(AQNWB::mergePaths(m_path, "location"),
                                    spec.location);
  if (spec.originCoords.has_value()) {
    const std::string originCoordsPath =
        AQNWB::mergePaths(m_path, "origin_coords");
    status = status
        && createFloatArrayDataSet(
                 *ioPtr, originCoordsPath, spec.originCoords.value());
    status = status
        && ioPtr->createAttribute(
            spec.originCoordsUnit, originCoordsPath, "unit");
  }
  if (spec.gridSpacing.has_value()) {
    const std::string gridSpacingPath =
        AQNWB::mergePaths(m_path, "grid_spacing");
    status = status
        && createFloatArrayDataSet(
                 *ioPtr, gridSpacingPath, spec.gridSpacing.value());
    status = status
        && ioPtr->createAttribute(
            spec.gridSpacingUnit, gridSpacingPath, "unit");
  }
  if (spec.referenceFrame.has_value()) {
    status = status
        && ioPtr->createStringDataSet(
            AQNWB::mergePaths(m_path, "reference_frame"),
            spec.referenceFrame.value());
  }

  // Create the optical channels
  for (const auto& [name, channelSpec] : spec.opticalChannels) {
    auto opticalChannel = createOpticalChannel(name);
    if (opticalChannel == nullptr) {
      status = Status::Failure;
    } else {
      status = status && opticalChannel->initialize(channelSpec);
    }
  }

  // Link to the device
  status = status
      && ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "device"),
                           AQNWB::mergePaths("/", spec.device->getPath()));
  return status;
}
