#include <iostream>

#include "nwb/base/TimeSeries.hpp"

#include "Utils.hpp"
#include "io/BaseIO.hpp"

using namespace AQNWB::NWB;

// TimeSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(TimeSeries)

// Initialize the static member
std::map<TimeSeries::ContinuityType, std::string>
    TimeSeries::ContinuityTypeNames = {
        {TimeSeries::Continuous, "continuous"},
        {TimeSeries::Instantaneous, "instantaneous"},
        {TimeSeries::Step, "step"}};

/** Constructor */
TimeSeries::TimeSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : NWBDataInterface(path, io)
{
}

/** Destructor */
TimeSeries::~TimeSeries() {}

Status TimeSeries::createDataAttributes(const std::string& path,
                                        const float& conversion,
                                        const float& resolution,
                                        const float& offset,
                                        const std::string& unit,
                                        const ContinuityType& continuity)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "TimeSeries::createDataAttributes: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  ioPtr->createAttribute(AQNWB::IO::BaseDataType::F32,
                         &conversion,
                         AQNWB::mergePaths(path, "data"),
                         "conversion");
  ioPtr->createAttribute(AQNWB::IO::BaseDataType::F32,
                         &resolution,
                         AQNWB::mergePaths(path, "data"),
                         "resolution");
  ioPtr->createAttribute(AQNWB::IO::BaseDataType::F32,
                         &offset,
                         AQNWB::mergePaths(path, "data"),
                         "offset");
  ioPtr->createAttribute(unit, path + "/data", "unit");
  if (continuity != ContinuityType::Undefined) {
    ioPtr->createAttribute(
        ContinuityTypeNames[continuity], path + "/data", "continuity");
  }
  return Status::Success;
}

Status TimeSeries::createTimestampsAttributes(const std::string& path)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr
        << "TimeSeries::createTimestampsAttributes: IO object is not valid."
        << std::endl;
    return Status::Failure;
  }

  int interval = 1;
  ioPtr->createAttribute(AQNWB::IO::BaseDataType::I32,
                         static_cast<const void*>(&interval),
                         path + "/timestamps",
                         "interval");
  ioPtr->createAttribute("seconds", path + "/timestamps", "unit");

  return Status::Success;
}

Status TimeSeries::Spec::validate() const
{
  Status status = NWBDataInterface::Spec::validate();
  if (data == nullptr) {
    std::cerr << "TimeSeries::Spec::validate: data configuration is not set."
              << std::endl;
    status = Status::Failure;
  }
  if (unit.empty()) {
    std::cerr << "TimeSeries::Spec::validate: unit must not be empty."
              << std::endl;
    status = Status::Failure;
  }
  if (startingTime.has_value()) {
    if (startingTime.value() < 0.0) {
      std::cerr << "TimeSeries::Spec::validate: startingTime must be >= 0."
                << std::endl;
      status = Status::Failure;
    }
    if (!(startingTimeRate > 0.0f)) {
      std::cerr << "TimeSeries::Spec::validate: startingTimeRate must be > 0."
                << std::endl;
      status = Status::Failure;
    }
  }
  return status;
}

Status TimeSeries::initialize(
    const IO::BaseArrayDataSetConfig& dataConfig,
    const std::string& unit,
    const std::string& description,
    const std::string& comments,
    const float& conversion,
    const float& resolution,
    const float& offset,
    const ContinuityType& continuity,
    const double& startingTime,
    const float& startingTimeRate,
    const std::vector<std::string>& controlDescription)
{
  Spec spec;
  // Non-owning pointer to the dataConfig, which outlives the call to
  // initialize
  spec.data = Spec::DataConfigPtr(std::shared_ptr<void>(), &dataConfig);
  spec.unit = unit;
  spec.description = description;
  spec.comments = comments;
  spec.conversion = conversion;
  spec.resolution = resolution;
  spec.offset = offset;
  spec.continuity = continuity;
  if (startingTime >= 0) {
    spec.startingTime = startingTime;
  }
  spec.startingTimeRate = startingTimeRate;
  spec.controlDescription = controlDescription;
  return initialize(spec);
}

Status TimeSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "TimeSeries::initialize: IO object is not valid." << std::endl;
    return Status::Failure;
  }

  // Validate the spec and create the group
  Status status = NWBDataInterface::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }
  const IO::BaseArrayDataSetConfig& dataConfig = *spec.data;
  m_dataIsLink = dataConfig.isLink();
  m_dataTypeInitialized = true;

  // Extract shape, chunking, and data type from the config
  SizeArray shape, chunking;
  status = status
      && dataConfig.getProperties(
          ioPtr.get(), shape, chunking, this->m_dataType);
  if (status != Status::Success) {
    std::cerr << "TimeSeries::initialize: Failed to get properties from "
                 "dataConfig"
              << std::endl;
    return Status::Failure;
  }

  // Validate and set timestamp and control shape
  // timestamps match data along first dimension
  SizeArray tsDsetSize;
  if (shape.empty()) {
    std::cerr << "TimeSeries::initialize: Shape cannot be empty" << std::endl;
    return Status::Failure;
  } else {
    tsDsetSize = {shape[0]};
  }

  // Validate and set the timestamp and control chunking
  SizeArray tsChunkSize;
  if (chunking.empty()) {
    // Use default chunk size if chunking of data not found
    tsChunkSize = {8192};
    std::cerr << "TimeSeries::initialize: Chunking not provided, using default "
                 "chunk size of 8192 for timestamps"
              << std::endl;
  } else {
    tsChunkSize = {chunking[0]};
  }

  // create comments attribute
  if (spec.description != "") {
    status = status
        && ioPtr->createAttribute(spec.description, m_path, "description");
  }
  status = status && ioPtr->createAttribute(spec.comments, m_path, "comments");

  // setup data datasets
  try {
    ioPtr->createArrayDataSet(dataConfig, AQNWB::mergePaths(m_path, "data"));
    if (dataConfig.isLink()) {
      // For links, we don't set attributes since there is no dataset to attach
      // them to. Instead, validate that the target has the required attributes.
      const auto* linkConfig =
          dynamic_cast<const IO::LinkArrayDataSetConfig*>(&dataConfig);
      if (linkConfig) {
        // Validate that the linked target has the required TimeSeries data
        // attributes: conversion, resolution, offset, and unit.
        status = status
            && linkConfig->validateTarget(
                *ioPtr, {}, {}, {"conversion", "resolution", "offset", "unit"});
      } else {
        std::cerr << "TimeSeries::initialize: dataConfig.isLink() is true but "
                  << "dynamic_cast to LinkArrayDataSetConfig failed."
                  << std::endl;
        status = Status::Failure;
      }
    } else {
      status = status
          && this->createDataAttributes(m_path,
                                        spec.conversion,
                                        spec.resolution,
                                        spec.offset,
                                        spec.unit,
                                        spec.continuity);
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "Failed to create data dataset: " << e.what() << std::endl;
    status = Status::Failure;
  }

  // setup timestamps datasets
  try {
    if (!spec.startingTime.has_value()) {
      IO::ArrayDataSetConfig timestampsConfig(
          this->timestampsType, tsDsetSize, tsChunkSize);
      ioPtr->createArrayDataSet(timestampsConfig,
                                AQNWB::mergePaths(m_path, "timestamps"));
      status = status && this->createTimestampsAttributes(m_path);
    } else  // setup starting_time datasets
    {
      const double startingTime = spec.startingTime.value();
      std::string startingTimePath = AQNWB::mergePaths(m_path, "starting_time");
      IO::ArrayDataSetConfig startingTimeConfig(
          AQNWB::IO::BaseDataType::F64, {1}, {1});
      ioPtr->createArrayDataSet(startingTimeConfig, startingTimePath);
      auto startingTimeRecorder = this->recordStartingTime();
      status = status
          && startingTimeRecorder->writeDataBlock(
              {1}, AQNWB::IO::BaseDataType::F64, &startingTime);
      status = status
          && ioPtr->createAttribute(AQNWB::IO::BaseDataType::F32,
                                    &spec.startingTimeRate,
                                    startingTimePath,
                                    "rate");
      status =
          status && ioPtr->createAttribute("seconds", startingTimePath, "unit");
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "Failed to create timestamps dataset: " << e.what()
              << std::endl;
    status = Status::Failure;
  }

  // create control datasets if necessary
  try {
    if (!spec.controlDescription.empty()) {
      // control matches data along first dimension
      IO::ArrayDataSetConfig controlConfig(
          AQNWB::IO::BaseDataType::U8, tsDsetSize, tsChunkSize);
      ioPtr->createArrayDataSet(controlConfig,
                                AQNWB::mergePaths(m_path, "control"));

      // control_description is its own data and contains for each control value
      // a string description
      const SizeArray controlDescriptionShape = {
          spec.controlDescription.size()};
      const SizeArray controlDescriptionChunkSize = {
          spec.controlDescription.size()};
      const SizeArray controlDescriptionPositionOffset = {0};
      IO::BaseDataType controlDesriptionType(IO::BaseDataType::Type::V_STR,
                                             0);  // 0 indicates variable length
      IO::ArrayDataSetConfig controlDescriptionConfig(
          controlDesriptionType,
          controlDescriptionShape,
          controlDescriptionChunkSize);
      ioPtr->createArrayDataSet(
          controlDescriptionConfig,
          AQNWB::mergePaths(m_path, "control_description"));
      auto controlDescriptionRecorder = this->recordControlDescription();
      status = status
          && controlDescriptionRecorder->writeDataBlock(
              controlDescriptionShape,
              controlDescriptionPositionOffset,
              controlDesriptionType,
              spec.controlDescription);
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "Failed to create control dataset: " << e.what() << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status TimeSeries::writeData(const SizeArray& dataShape,
                             const SizeArray& positionOffset,
                             const void* dataInput,
                             const void* timestampsInput,
                             const void* controlInput)
{
  if (dataShape.empty() || positionOffset.empty()) {
    std::cerr << "TimeSeries::writeData: dataShape and positionOffset must "
                 "not be empty."
              << std::endl;
    return Status::Failure;
  }

  // Get all recorders before writing anything, such that a missing dataset
  // does not leave partially written data
  std::shared_ptr<IO::BaseRecordingData> timestampsRecorder = nullptr;
  if (timestampsInput != nullptr) {
    timestampsRecorder = this->recordTimestamps();
    if (timestampsRecorder == nullptr) {
      std::cerr << "TimeSeries::writeData: " << m_path
                << " has no timestamps dataset (it uses starting_time and "
                   "rate). Pass nullptr for the timestamps."
                << std::endl;
      return Status::Failure;
    }
  }
  std::shared_ptr<IO::BaseRecordingData> controlRecorder = nullptr;
  if (controlInput != nullptr) {
    controlRecorder = this->recordControl();
    if (controlRecorder == nullptr) {
      std::cerr << "TimeSeries::writeData: " << m_path
                << " has no control dataset. Set the controlDescription of "
                   "the spec to record control values."
                << std::endl;
      return Status::Failure;
    }
  }
  auto dataRecorder = this->recordData();
  if (dataRecorder == nullptr) {
    std::cerr << "TimeSeries::writeData: " << m_path << " has no data dataset."
              << std::endl;
    return Status::Failure;
  }

  // timestamps and control match the shape of the first data dimension
  const SizeArray timeShape = {dataShape[0]};
  const SizeArray timePositionOffset = {positionOffset[0]};
  Status tsStatus = Status::Success;
  if (timestampsRecorder != nullptr) {
    tsStatus = timestampsRecorder->writeDataBlock(
        timeShape, timePositionOffset, this->timestampsType, timestampsInput);
  }
  Status dataStatus = dataRecorder->writeDataBlock(
      dataShape, positionOffset, this->m_dataType, dataInput);
  Status controlStatus = Status::Success;
  if (controlRecorder != nullptr) {
    controlStatus = controlRecorder->writeDataBlock(
        timeShape, timePositionOffset, this->controlType, controlInput);
  }

  return dataStatus && tsStatus && controlStatus;
}

Status TimeSeries::appendSamples(const SizeType& numSamples,
                                 const void* dataInput,
                                 const void* timestampsInput,
                                 const void* controlInput)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "TimeSeries::appendSamples: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }
  if (m_dataIsLink) {
    std::cerr << "TimeSeries::appendSamples: The data of " << m_path
              << " is a link. Write the data via the linked TimeSeries."
              << std::endl;
    return Status::Failure;
  }
  auto dataRecorder = this->recordData();
  if (dataRecorder == nullptr) {
    std::cerr << "TimeSeries::appendSamples: " << m_path
              << " has no data dataset." << std::endl;
    return Status::Failure;
  }
  // The current extent of the data determines the offset and the shape of a
  // sample, such that appending also works after reopening a file
  const SizeArray currentShape = dataRecorder->getShape();
  if (currentShape.empty()) {
    std::cerr << "TimeSeries::appendSamples: The data of " << m_path
              << " must have at least one dimension." << std::endl;
    return Status::Failure;
  }
  // The timing mode of the series determines whether timestamps are required
  const bool usesTimestamps =
      ioPtr->objectExists(AQNWB::mergePaths(m_path, "timestamps"));
  if (usesTimestamps && timestampsInput == nullptr) {
    std::cerr << "TimeSeries::appendSamples: " << m_path
              << " uses timestamps, i.e., the timestamps of the samples are "
                 "required."
              << std::endl;
    return Status::Failure;
  }
  if (!usesTimestamps && timestampsInput != nullptr) {
    std::cerr << "TimeSeries::appendSamples: " << m_path
              << " uses starting_time and rate, i.e., timestamps must be "
                 "nullptr."
              << std::endl;
    return Status::Failure;
  }
  if (!m_dataTypeInitialized) {
    // Use the data type of the dataset if the series was not initialized in
    // this session (e.g., after reopening the file)
    m_dataType =
        ioPtr->getStorageObjectDataType(AQNWB::mergePaths(m_path, "data"));
    m_dataTypeInitialized = true;
  }

  SizeArray dataShape = currentShape;
  dataShape[0] = numSamples;
  SizeArray positionOffset(currentShape.size(), 0);
  positionOffset[0] = currentShape[0];
  return this->writeData(
      dataShape, positionOffset, dataInput, timestampsInput, controlInput);
}
