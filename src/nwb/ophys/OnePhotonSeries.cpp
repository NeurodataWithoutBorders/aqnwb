#include "nwb/ophys/OnePhotonSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// OnePhotonSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(OnePhotonSeries)

/** Constructor */
OnePhotonSeries::OnePhotonSeries(const std::string& path,
                                 std::shared_ptr<IO::BaseIO> io)
    : ImageSeries(path, io)
{
}

/** Destructor */
OnePhotonSeries::~OnePhotonSeries() {}

Status OnePhotonSeries::Spec::validate() const
{
  Status status = ImageSeries::Spec::validate();
  if (imagingPlane == nullptr) {
    std::cerr << "OnePhotonSeries::Spec::validate: imagingPlane must be set."
              << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status OnePhotonSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "OnePhotonSeries::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  // Check the link targets before anything is written
  if (checkLinkTarget(spec.imagingPlane, "imagingPlane") != Status::Success) {
    return Status::Failure;
  }
  // Validate the spec and initialize the ImageSeries
  Status status = ImageSeries::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }

  if (spec.pmtGain.has_value()) {
    const float pmtGain = spec.pmtGain.value();
    status = status
        && ioPtr->createAttribute(
            IO::BaseDataType::F32, &pmtGain, m_path, "pmt_gain");
  }
  if (spec.scanLineRate.has_value()) {
    const float scanLineRate = spec.scanLineRate.value();
    status = status
        && ioPtr->createAttribute(
            IO::BaseDataType::F32, &scanLineRate, m_path, "scan_line_rate");
  }
  if (spec.exposureTime.has_value()) {
    const float exposureTime = spec.exposureTime.value();
    status = status
        && ioPtr->createAttribute(
            IO::BaseDataType::F32, &exposureTime, m_path, "exposure_time");
  }
  if (spec.binning.has_value()) {
    const uint8_t binning = spec.binning.value();
    status = status
        && ioPtr->createAttribute(
            IO::BaseDataType::U8, &binning, m_path, "binning");
  }
  if (spec.power.has_value()) {
    const float power = spec.power.value();
    status = status
        && ioPtr->createAttribute(
            IO::BaseDataType::F32, &power, m_path, "power");
  }
  if (spec.intensity.has_value()) {
    const float intensity = spec.intensity.value();
    status = status
        && ioPtr->createAttribute(
            IO::BaseDataType::F32, &intensity, m_path, "intensity");
  }
  status = status
      && ioPtr->createLink(
          AQNWB::mergePaths("/" + m_path, "imaging_plane"),
          AQNWB::mergePaths("/", spec.imagingPlane->getPath()));
  return status;
}
