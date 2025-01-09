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
    : Container(path, io)
    , timestamps(nullptr)
    , starting_time(nullptr)
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
  m_io->createAttribute(AQNWB::IO::BaseDataType::F32,
                        &conversion,
                        AQNWB::mergePaths(path, "data"),
                        "conversion");
  m_io->createAttribute(AQNWB::IO::BaseDataType::F32,
                        &resolution,
                        AQNWB::mergePaths(path, "data"),
                        "resolution");
  m_io->createAttribute(AQNWB::IO::BaseDataType::F32,
                        &offset,
                        AQNWB::mergePaths(path, "data"),
                        "offset");
  m_io->createAttribute(unit, path + "/data", "unit");
  if (continuity != ContinuityType::Undefined) {
    m_io->createAttribute(
        ContinuityTypeNames[continuity], path + "/data", "continuity");
  }
  return Status::Success;
}

Status TimeSeries::createTimestampsAttributes(const std::string& path)
{
  int interval = 1;
  m_io->createAttribute(AQNWB::IO::BaseDataType::I32,
                        static_cast<const void*>(&interval),
                        path + "/timestamps",
                        "interval");
  m_io->createAttribute("seconds", path + "/timestamps", "unit");

  return Status::Success;
}

void TimeSeries::initialize(const IO::BaseDataType& dataType,
                            const std::string& unit,
                            const std::string& description,
                            const std::string& comments,
                            const SizeArray& dsetSize,
                            const SizeArray& chunkSize,
                            const float& conversion,
                            const float& resolution,
                            const float& offset,
                            const ContinuityType& continuity,
                            const double& startingTime,
                            const float& startingTimeRate,
                            bool useControl)
{
  Container::initialize();

  this->dataType = dataType;

  // create comments attribute
  if (description != "")
    m_io->createAttribute(description, m_path, "description");
  m_io->createAttribute(comments, m_path, "comments");

  // setup data datasets
  this->data = std::unique_ptr<IO::BaseRecordingData>(m_io->createArrayDataSet(
      dataType, dsetSize, chunkSize, AQNWB::mergePaths(m_path, "data")));
  this->createDataAttributes(
      m_path, conversion, resolution, offset, unit, continuity);

  // setup timestamps datasets
  if (startingTime < 0) {
    this->starting_time = nullptr;
    // timestamps match data along first dimension
    SizeArray tsDsetSize = {dsetSize[0]};
    SizeArray tsChunkSize = {chunkSize[0]};
    this->timestamps = std::unique_ptr<IO::BaseRecordingData>(
        m_io->createArrayDataSet(this->timestampsType,
                                 tsDsetSize,
                                 tsChunkSize,
                                 AQNWB::mergePaths(m_path, "timestamps")));
    this->createTimestampsAttributes(m_path);
  } else  // setup starting_time datasets
  {
    this->timestamps = nullptr;
    std::string startingTimePath = AQNWB::mergePaths(m_path, "starting_time");
    this->starting_time = m_io->createArrayDataSet(
        AQNWB::IO::BaseDataType::F64, {1}, {1}, startingTimePath);
    this->starting_time->writeDataBlock(
        {1}, AQNWB::IO::BaseDataType::F64, &startingTime);
    m_io->createAttribute(AQNWB::IO::BaseDataType::F32,
                          &startingTimeRate,
                          startingTimePath,
                          "rate");
    m_io->createAttribute("seconds", startingTimePath, "unit");
  }

  // create control datasets if necessary
  if (useControl) {
    // control matches data along first dimension
    SizeArray controlDsetSize = {dsetSize[0]};
    SizeArray controlChunkSize = {chunkSize[0]};
    this->control = std::unique_ptr<IO::BaseRecordingData>(
        m_io->createArrayDataSet(AQNWB::IO::BaseDataType::U8,
                                 controlDsetSize,
                                 controlChunkSize,
                                 AQNWB::mergePaths(m_path, "control")));
    IO::BaseDataType controlDesriptionType(IO::BaseDataType::Type::V_STR,
                                           0);  // 0 indicates variable length
    this->control_description =
        std::unique_ptr<IO::BaseRecordingData>(m_io->createArrayDataSet(
            controlDesriptionType,
            controlDsetSize,
            controlChunkSize,
            AQNWB::mergePaths(m_path, "control_description")));
  } else {
    this->control = nullptr;
    this->control_description = nullptr;
  }
}

Status TimeSeries::writeData(
    const std::vector<SizeType>& dataShape,
    const std::vector<SizeType>& positionOffset,
    const void* dataInput,
    const void* timestampsInput,
    const void* controlInput,
    const std::vector<std::string>& controlDescriptionInput)
{
  // Write timestamps if they exist
  Status tsStatus = Status::Success;
  if (timestampsInput != nullptr) {
    // timestamps should match shape of the first data dimension
    const std::vector<SizeType> timestampsShape = {dataShape[0]};
    const std::vector<SizeType> timestampsPositionOffset = {positionOffset[0]};
    tsStatus = this->timestamps->writeDataBlock(timestampsShape,
                                                timestampsPositionOffset,
                                                this->timestampsType,
                                                timestampsInput);
  }

  // Write the data
  Status dataStatus = this->data->writeDataBlock(
      dataShape, positionOffset, this->dataType, dataInput);

  // Write the control data if it exists
  if (controlInput != nullptr) {
    // control should match shape of the first data dimension
    const std::vector<SizeType> controlShape = {dataShape[0]};
    const std::vector<SizeType> controlPositionOffset = {positionOffset[0]};
    tsStatus = this->control->writeDataBlock(
        controlShape, controlPositionOffset, this->controlType, controlInput);
  }
  if (controlDescriptionInput.size() > 0) {
    // control_description should match shape of the first data dimension
    const std::vector<SizeType> controlDescriptionShape = {dataShape[0]};
    const std::vector<SizeType> controlDescriptionPositionOffset = {
        positionOffset[0]};
    IO::BaseDataType controlDesriptionType(IO::BaseDataType::Type::V_STR,
                                           0);  // 0 indicates variable length
    tsStatus = this->control_description->writeStringDataBlock(
        controlDescriptionShape,
        controlDescriptionPositionOffset,
        controlDesriptionType,
        controlDescriptionInput);
  }

  if ((dataStatus != Status::Success) || (tsStatus != Status::Success)) {
    return Status::Failure;
  } else {
    return Status::Success;
  }
}
