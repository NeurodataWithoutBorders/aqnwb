#include "nwb/image/OpticalSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// OpticalSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(OpticalSeries)

/** Constructor */
OpticalSeries::OpticalSeries(const std::string& path,
                             std::shared_ptr<IO::BaseIO> io)
    : ImageSeries(path, io)
{
}

/** Destructor */
OpticalSeries::~OpticalSeries() {}

Status OpticalSeries::Spec::validate() const
{
  Status status = ImageSeries::Spec::validate();
  if (fieldOfView.has_value() && fieldOfView->size() != 2
      && fieldOfView->size() != 3)
  {
    std::cerr << "OpticalSeries::Spec::validate: fieldOfView must have 2 or 3 "
                 "elements."
              << std::endl;
    status = Status::Failure;
  }
  if (data != nullptr && !data->isLink()) {
    SizeArray shape, chunking;
    IO::BaseDataType dataType;
    if (data->getProperties(nullptr, shape, chunking, dataType)
            == Status::Success
        && shape.size() == 4 && shape[3] != 3)
    {
      std::cerr << "OpticalSeries::Spec::validate: 4D data must have 3 color "
                   "channels [frame, x, y, 3]."
                << std::endl;
      status = Status::Failure;
    }
  }
  return status;
}

Status OpticalSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "OpticalSeries::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }
  Status status = ImageSeries::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }
  if (spec.distance.has_value()) {
    const float distance = spec.distance.value();
    status = status
        && ioPtr->createScalarDataSet(IO::BaseDataType::F32,
                                      &distance,
                                      AQNWB::mergePaths(m_path, "distance"));
  }
  if (spec.fieldOfView.has_value()) {
    const std::vector<float>& fieldOfView = spec.fieldOfView.value();
    auto recorder = ioPtr->createArrayDataSet(
        IO::ArrayDataSetConfig(IO::BaseDataType::F32,
                               SizeArray {fieldOfView.size()},
                               SizeArray {fieldOfView.size()}),
        AQNWB::mergePaths(m_path, "field_of_view"));
    if (recorder == nullptr) {
      status = Status::Failure;
    } else {
      status = status
          && recorder->writeDataBlock(SizeArray {fieldOfView.size()},
                                      IO::BaseDataType::F32,
                                      fieldOfView.data());
    }
  }
  if (spec.orientation.has_value()) {
    status = status
        && ioPtr->createStringDataSet(AQNWB::mergePaths(m_path, "orientation"),
                                      spec.orientation.value());
  }
  return status;
}
