#include "nwb/ophys/RoiResponseSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// RoiResponseSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(RoiResponseSeries)

/** Constructor */
RoiResponseSeries::RoiResponseSeries(const std::string& path,
                                     std::shared_ptr<IO::BaseIO> io)
    : TimeSeries(path, io)
{
}

/** Destructor */
RoiResponseSeries::~RoiResponseSeries() {}

Status RoiResponseSeries::Spec::validate() const
{
  Status status = TimeSeries::Spec::validate();
  if (planeSegmentation == nullptr) {
    std::cerr << "RoiResponseSeries::Spec::validate: planeSegmentation must "
                 "be set."
              << std::endl;
    status = Status::Failure;
  }
  if (roiIndices.empty()) {
    std::cerr << "RoiResponseSeries::Spec::validate: roiIndices must not be "
                 "empty."
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
      const bool validShape = (shape.size() == 1 && roiIndices.size() == 1)
          || (shape.size() == 2 && shape[1] == roiIndices.size());
      if (!validShape) {
        std::cerr << "RoiResponseSeries::Spec::validate: data must be a 1D "
                     "array for a single ROI or a 2D array with one column "
                     "per ROI."
                  << std::endl;
        status = Status::Failure;
      }
      if (dataType.type == IO::BaseDataType::Type::T_STR
          || dataType.type == IO::BaseDataType::Type::V_STR)
      {
        std::cerr << "RoiResponseSeries::Spec::validate: data must be numeric."
                  << std::endl;
        status = Status::Failure;
      }
    }
  }
  return status;
}

Status RoiResponseSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "RoiResponseSeries::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  // Validate the spec and initialize the TimeSeries
  Status status = TimeSeries::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }

  // Remember the shape of a sample for writeSamples
  SizeArray shape, chunking;
  IO::BaseDataType dataType;
  if (spec.data->getProperties(ioPtr.get(), shape, chunking, dataType)
          == Status::Success
      && !shape.empty())
  {
    m_sampleShape = SizeArray(shape.begin() + 1, shape.end());
  }
  m_samplesRecorded = 0;

  // Create the rois DynamicTableRegion
  const SizeType numRois = spec.roiIndices.size();
  auto rois =
      DynamicTableRegion::create(AQNWB::mergePaths(m_path, "rois"), ioPtr);
  if (rois == nullptr
      || rois->initialize(IO::ArrayDataSetConfig(IO::BaseDataType::I32,
                                                 SizeArray {numRois},
                                                 SizeArray {numRois}),
                          spec.roisDescription,
                          spec.planeSegmentation->getPath())
          != Status::Success)
  {
    std::cerr << "RoiResponseSeries::initialize: Failed to create rois."
              << std::endl;
    return Status::Failure;
  }
  status = status
      && rois->recordData()->writeDataBlock(
          SizeArray {numRois}, IO::BaseDataType::I32, spec.roiIndices.data());
  return status;
}

Status RoiResponseSeries::writeSamples(const SizeType& numSamples,
                                       const void* dataInput,
                                       const void* timestampsInput,
                                       const void* controlInput)
{
  SizeArray dataShape = {numSamples};
  dataShape.insert(dataShape.end(), m_sampleShape.begin(), m_sampleShape.end());
  SizeArray positionOffset(dataShape.size(), 0);
  positionOffset[0] = m_samplesRecorded;

  Status status = this->writeData(
      dataShape, positionOffset, dataInput, timestampsInput, controlInput);
  if (status == Status::Success) {
    m_samplesRecorded += numSamples;
  }
  return status;
}
