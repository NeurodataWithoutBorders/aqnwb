#include "nwb/ogen/OptogeneticSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// OptogeneticSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(OptogeneticSeries)

/** Constructor */
OptogeneticSeries::OptogeneticSeries(const std::string& path,
                                     std::shared_ptr<IO::BaseIO> io)
    : TimeSeries(path, io)
{
}

/** Destructor */
OptogeneticSeries::~OptogeneticSeries() {}

Status OptogeneticSeries::Spec::validate() const
{
  Status status = TimeSeries::Spec::validate();
  if (unit != "watts") {
    std::cerr << "OptogeneticSeries::Spec::validate: unit must be 'watts' but "
                 "is '"
              << unit << "'." << std::endl;
    status = Status::Failure;
  }
  if (site == nullptr) {
    std::cerr << "OptogeneticSeries::Spec::validate: site must be set."
              << std::endl;
    status = Status::Failure;
  }
  // Check the shape and data type of the data (unless the data is a link)
  if (data != nullptr && !data->isLink()) {
    SizeArray shape, chunking;
    IO::BaseDataType dataType;
    if (data->getProperties(nullptr, shape, chunking, dataType)
        == Status::Success)
    {
      if (shape.size() != 1 && shape.size() != 2) {
        std::cerr << "OptogeneticSeries::Spec::validate: data must be a 1D or "
                     "2D array but has "
                  << shape.size() << " dimensions." << std::endl;
        status = Status::Failure;
      }
      if (dataType.type == IO::BaseDataType::Type::T_STR
          || dataType.type == IO::BaseDataType::Type::V_STR)
      {
        std::cerr << "OptogeneticSeries::Spec::validate: data must be numeric."
                  << std::endl;
        status = Status::Failure;
      }
    }
  }
  return status;
}

Status OptogeneticSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "OptogeneticSeries::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  // Validate the spec and initialize the TimeSeries
  Status status = TimeSeries::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }

  status = status
      && ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "site"),
                           AQNWB::mergePaths("/", spec.site->getPath()));
  return status;
}

Status OptogeneticSeries::writeSamples(const SizeType& numSamples,
                                       const void* dataInput,
                                       const void* timestampsInput,
                                       const void* controlInput)
{
  return appendSamples(numSamples, dataInput, timestampsInput, controlInput);
}
