#include "nwb/misc/AnnotationSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// AnnotationSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(AnnotationSeries)

/** Constructor */
AnnotationSeries::AnnotationSeries(const std::string& path,
                                   std::shared_ptr<IO::BaseIO> io)
    : TimeSeries(path, io)
{
}

/** Destructor */
AnnotationSeries::~AnnotationSeries() {}

/** Initialization function*/
Status AnnotationSeries::initialize(
    const std::string& description,
    const std::string& comments,
    const IO::BaseArrayDataSetConfig& dataConfig)
{
  Spec spec;
  // Non-owning pointer to the dataConfig, which outlives the call to
  // initialize
  spec.data = Spec::DataConfigPtr(std::shared_ptr<void>(), &dataConfig);
  spec.description = description;
  spec.comments = comments;
  return initialize(spec);
}

Status AnnotationSeries::Spec::validate() const
{
  Status status = TimeSeries::Spec::validate();
  if (unit != "n/a") {
    std::cerr << "AnnotationSeries::Spec::validate: unit must be 'n/a' but is '"
              << unit << "'." << std::endl;
    status = Status::Failure;
  }
  // The schema fixes these values, since the unit is n/a
  auto differs = [](float value, float fixed)
  { return value < fixed || value > fixed; };
  if (differs(conversion, 1.0f) || differs(resolution, -1.0f)
      || differs(offset, 0.0f))
  {
    std::cerr << "AnnotationSeries::Spec::validate: conversion, resolution, "
                 "and offset must be 1.0, -1.0, and 0.0."
              << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status AnnotationSeries::initialize(const Spec& spec)
{
  return TimeSeries::initialize(spec);
}

Status AnnotationSeries::writeAnnotation(
    const SizeType& numSamples,
    const std::vector<std::string>& dataInput,
    const void* timestampsInput,
    const void* controlInput)
{
  SizeArray dataShape = {numSamples};
  SizeArray positionOffset = {this->m_samplesRecorded};

  // Write timestamps
  Status tsStatus = Status::Success;
  auto timestampsRecorder = this->recordTimestamps();
  tsStatus = timestampsRecorder->writeDataBlock(
      dataShape, positionOffset, this->timestampsType, timestampsInput);

  // Write the data
  auto dataRecorder = this->recordData();
  Status dataStatus = dataRecorder->writeDataBlock(
      dataShape, positionOffset, this->m_dataType, dataInput);

  // Write the control data if it exists
  if (controlInput != nullptr) {
    auto controlRecorder = this->recordControl();
    tsStatus = controlRecorder->writeDataBlock(
        dataShape, positionOffset, this->controlType, controlInput);
  }

  // track samples recorded
  m_samplesRecorded += numSamples;

  return dataStatus && tsStatus;
}
