#include "nwb/ophys/TwoPhotonSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// TwoPhotonSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(TwoPhotonSeries)

/** Constructor */
TwoPhotonSeries::TwoPhotonSeries(const std::string& path,
                                 std::shared_ptr<IO::BaseIO> io)
    : ImageSeries(path, io)
{
}

/** Destructor */
TwoPhotonSeries::~TwoPhotonSeries() {}

Status TwoPhotonSeries::Spec::validate() const
{
  Status status = ImageSeries::Spec::validate();
  if (imagingPlane == nullptr) {
    std::cerr << "TwoPhotonSeries::Spec::validate: imagingPlane must be set."
              << std::endl;
    status = Status::Failure;
  }
  if (fieldOfView.has_value() && fieldOfView->size() != 2
      && fieldOfView->size() != 3)
  {
    std::cerr << "TwoPhotonSeries::Spec::validate: fieldOfView must have 2 or "
                 "3 elements."
              << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status TwoPhotonSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "TwoPhotonSeries::initialize: IO object is not valid."
              << std::endl;
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
  if (spec.fieldOfView.has_value()) {
    const std::vector<float>& fieldOfView = spec.fieldOfView.value();
    IO::ArrayDataSetConfig fieldOfViewConfig(IO::BaseDataType::F32,
                                             SizeArray {fieldOfView.size()},
                                             SizeArray {fieldOfView.size()});
    auto fieldOfViewRecorder = ioPtr->createArrayDataSet(
        fieldOfViewConfig, AQNWB::mergePaths(m_path, "field_of_view"));
    if (fieldOfViewRecorder == nullptr) {
      status = Status::Failure;
    } else {
      status = status
          && fieldOfViewRecorder->writeDataBlock(SizeArray {fieldOfView.size()},
                                                 IO::BaseDataType::F32,
                                                 fieldOfView.data());
    }
  }
  status = status
      && ioPtr->createLink(
          AQNWB::mergePaths("/" + m_path, "imaging_plane"),
          AQNWB::mergePaths("/", spec.imagingPlane->getPath()));
  return status;
}
