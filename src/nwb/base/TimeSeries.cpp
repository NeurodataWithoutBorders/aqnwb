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
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "TimeSeries::initialize: IO object is not valid." << std::endl;
    return Status::Failure;
  }

  auto interfaceInitStatus = NWBDataInterface::initialize();

  // Extract shape and chunking information
  SizeArray shape, chunking;

  if (dataConfig.isLink()) {
    // For links, query the target dataset for shape, chunking, and type
    const IO::LinkArrayDataSetConfig* linkConfig =
        dynamic_cast<const IO::LinkArrayDataSetConfig*>(&dataConfig);
    if (!linkConfig) {
      std::cerr << "TimeSeries::initialize: Failed to cast to "
                   "LinkArrayDataSetConfig for link data"
                << std::endl;
      return Status::Failure;
    }

    shape = linkConfig->getTargetShape(*ioPtr);
    chunking = linkConfig->getTargetChunking(*ioPtr);
    this->m_dataType = linkConfig->getTargetDataType(*ioPtr);
  } else {
    const IO::ArrayDataSetConfig* arrayConfig =
        dynamic_cast<const IO::ArrayDataSetConfig*>(&dataConfig);
    if (!arrayConfig) {
      std::cerr << "TimeSeries::initialize: Failed to cast to "
                   "ArrayDataSetConfig for array data"
                << std::endl;
      return Status::Failure;
    }

    this->m_dataType = arrayConfig->getType();
    shape = arrayConfig->getShape();
    chunking = arrayConfig->getChunking();
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
      // Use default to chunk size if chunking of data not found
      tsChunkSize = {8192};
  } else {
      tsChunkSize = {chunking[0]};
  }

  // create comments attribute
  if (description != "") {
    ioPtr->createAttribute(description, m_path, "description");
  }
  ioPtr->createAttribute(comments, m_path, "comments");

  // setup data datasets
  ioPtr->createArrayDataSet(dataConfig, AQNWB::mergePaths(m_path, "data"));
  this->createDataAttributes(
      m_path, conversion, resolution, offset, unit, continuity);

  // setup timestamps datasets
  if (startingTime < 0) { 
    IO::ArrayDataSetConfig timestampsConfig(
        this->timestampsType, tsDsetSize, tsChunkSize);
    ioPtr->createArrayDataSet(timestampsConfig,
                              AQNWB::mergePaths(m_path, "timestamps"));
    this->createTimestampsAttributes(m_path);
  } 
  else  // setup starting_time datasets
  {
    std::string startingTimePath = AQNWB::mergePaths(m_path, "starting_time");
    IO::ArrayDataSetConfig startingTimeConfig(
        AQNWB::IO::BaseDataType::F64, {1}, {1});
    ioPtr->createArrayDataSet(startingTimeConfig, startingTimePath);
    auto startingTimeRecorder = this->recordStartingTime();
    startingTimeRecorder->writeDataBlock(
        {1}, AQNWB::IO::BaseDataType::F64, &startingTime);
    ioPtr->createAttribute(AQNWB::IO::BaseDataType::F32,
                           &startingTimeRate,
                           startingTimePath,
                           "rate");
    ioPtr->createAttribute("seconds", startingTimePath, "unit");
  }

  // create control datasets if necessary
  if (controlDescription.size() > 0) {
    // control matches data along first dimension
    IO::ArrayDataSetConfig controlConfig(
        AQNWB::IO::BaseDataType::U8, tsDsetSize, tsChunkSize);
    ioPtr->createArrayDataSet(controlConfig,
                              AQNWB::mergePaths(m_path, "control"));

    // control_description is its own data and contains for each control value
    // a string description
    const SizeArray controlDescriptionShape = {controlDescription.size()};
    const SizeArray controlDescriptionChunkSize = {controlDescription.size()};
    const SizeArray controlDescriptionPositionOffset = {0};
    IO::BaseDataType controlDesriptionType(IO::BaseDataType::Type::V_STR,
                                           0);  // 0 indicates variable length
    IO::ArrayDataSetConfig controlDescriptionConfig(
        controlDesriptionType,
        controlDescriptionShape,
        controlDescriptionChunkSize);
    ioPtr->createArrayDataSet(controlDescriptionConfig,
                              AQNWB::mergePaths(m_path, "control_description"));
    auto controlDescriptionRecorder = this->recordControlDescription();
    controlDescriptionRecorder->writeDataBlock(controlDescriptionShape,
                                               controlDescriptionPositionOffset,
                                               controlDesriptionType,
                                               controlDescription);
  }
  return interfaceInitStatus;
}

Status TimeSeries::writeData(const std::vector<SizeType>& dataShape,
                             const std::vector<SizeType>& positionOffset,
                             const void* dataInput,
                             const void* timestampsInput,
                             const void* controlInput)
{
  // Write timestamps if they exist
  Status tsStatus = Status::Success;
  if (timestampsInput != nullptr) {
    // timestamps should match shape of the first data dimension
    const std::vector<SizeType> timestampsShape = {dataShape[0]};
    const std::vector<SizeType> timestampsPositionOffset = {positionOffset[0]};
    auto timestampsRecorder = this->recordTimestamps();
    tsStatus = timestampsRecorder->writeDataBlock(timestampsShape,
                                                  timestampsPositionOffset,
                                                  this->timestampsType,
                                                  timestampsInput);
  }

  // Write the data
  auto dataRecorder = this->recordData();
  Status dataStatus = dataRecorder->writeDataBlock(
      dataShape, positionOffset, this->m_dataType, dataInput);

  // Write the control data if it exists
  if (controlInput != nullptr) {
    // control should match shape of the first data dimension
    const std::vector<SizeType> controlShape = {dataShape[0]};
    const std::vector<SizeType> controlPositionOffset = {positionOffset[0]};
    auto controlRecorder = this->recordControl();
    tsStatus = controlRecorder->writeDataBlock(
        controlShape, controlPositionOffset, this->controlType, controlInput);
  }

  return dataStatus && tsStatus;
}
