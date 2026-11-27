#include "nwb/image/IndexSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// IndexSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(IndexSeries)

/** Constructor */
IndexSeries::IndexSeries(const std::string& path,
                         std::shared_ptr<IO::BaseIO> io)
    : TimeSeries(path, io)
{
}

/** Destructor */
IndexSeries::~IndexSeries() {}

Status IndexSeries::Spec::validate() const
{
  Status status = TimeSeries::Spec::validate();
  if (unit != "N/A") {
    std::cerr << "IndexSeries::Spec::validate: unit must be 'N/A' but is '"
              << unit << "'." << std::endl;
    status = Status::Failure;
  }
  if (indexedImages == nullptr && indexedTimeseries == nullptr) {
    std::cerr << "IndexSeries::Spec::validate: indexedImages (or the "
                 "deprecated indexedTimeseries) must be set."
              << std::endl;
    status = Status::Failure;
  }
  if (data != nullptr && !data->isLink()) {
    SizeArray shape, chunking;
    IO::BaseDataType dataType;
    if (data->getProperties(nullptr, shape, chunking, dataType)
            == Status::Success
        && (shape.size() != 1 || !(dataType == IO::BaseDataType::U32)))
    {
      std::cerr << "IndexSeries::Spec::validate: data must be a 1D uint32 "
                   "array."
                << std::endl;
      status = Status::Failure;
    }
  }
  return status;
}

Status IndexSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "IndexSeries::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }
  // Check the link targets before anything is written
  if (checkLinkTarget(spec.indexedImages, "indexedImages") != Status::Success) {
    return Status::Failure;
  }
  if (checkLinkTarget(spec.indexedTimeseries, "indexedTimeseries")
      != Status::Success)
  {
    return Status::Failure;
  }
  // The schema requires that the indexed Images define the order of the
  // images, which the indices refer to
  if (spec.indexedImages != nullptr
      && !ioPtr->objectExists(
          AQNWB::mergePaths(spec.indexedImages->getPath(), "order_of_images")))
  {
    std::cerr << "IndexSeries::initialize: The indexed Images "
              << spec.indexedImages->getPath()
              << " must define the order of the images (see "
                 "Images::setOrderOfImages)."
              << std::endl;
    return Status::Failure;
  }
  Status status = TimeSeries::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }
  if (spec.indexedImages != nullptr) {
    status = status
        && ioPtr->createLink(
            AQNWB::mergePaths("/" + m_path, "indexed_images"),
            AQNWB::mergePaths("/", spec.indexedImages->getPath()));
  }
  if (spec.indexedTimeseries != nullptr) {
    status = status
        && ioPtr->createLink(
            AQNWB::mergePaths("/" + m_path, "indexed_timeseries"),
            AQNWB::mergePaths("/", spec.indexedTimeseries->getPath()));
  }
  return status;
}

Status IndexSeries::writeIndices(const SizeType& numSamples,
                                 const uint32_t* indices,
                                 const void* timestampsInput)
{
  return appendSamples(numSamples, indices, timestampsInput);
}
