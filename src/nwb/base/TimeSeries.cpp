#include "nwb/base/TimeSeries.hpp"

using namespace AQNWB::NWB;

// TimeSeries

/** Constructor */
TimeSeries::TimeSeries(const std::string& path,
                       std::shared_ptr<BaseIO> io,
                       const BaseDataType& dataType,
                       const std::string& unit,
                       const std::string& description,
                       const std::string& comments,
                       const SizeArray& dsetSize,
                       const SizeArray& chunkSize,
                       const float& conversion,
                       const float& resolution,
                       const float& offset)
    : Container(path, io)
    , dataType(dataType)
    , unit(unit)
    , description(description)
    , comments(comments)
    , dsetSize(dsetSize)
    , chunkSize(chunkSize)
    , conversion(conversion)
    , resolution(resolution)
    , offset(offset)
{
}

/** Destructor */
TimeSeries::~TimeSeries() {}

void TimeSeries::initialize()
{
  Container::initialize();

  // setup attributes
  m_io->createCommonNWBAttributes(m_path, "core", neurodataType, description);
  m_io->createAttribute(comments, m_path, "comments");

  // setup datasets
  this->data = std::unique_ptr<BaseRecordingData>(m_io->createArrayDataSet(
      dataType, dsetSize, chunkSize, m_path + "/data"));
  m_io->createDataAttributes(m_path, conversion, resolution, unit);

  SizeArray tsDsetSize = {
      dsetSize[0]};  // timestamps match data along first dimension
  this->timestamps =
      std::unique_ptr<BaseRecordingData>(m_io->createArrayDataSet(
          this->timestampsType, tsDsetSize, chunkSize, m_path + "/timestamps"));
  m_io->createTimestampsAttributes(m_path);
}

Status TimeSeries::writeData(const std::vector<SizeType>& dataShape,
                             const std::vector<SizeType>& positionOffset,
                             const void* dataInput,
                             const void* timestampsInput)
{
  Status tsStatus = Status::Success;
  if (timestampsInput != nullptr) {
    const std::vector<SizeType> timestampsShape = {
        dataShape[0]};  // timestamps should match shape of the first data
                        // dimension
    const std::vector<SizeType> timestampsPositionOffset = {positionOffset[0]};
    tsStatus = this->timestamps->writeDataBlock(timestampsShape,
                                                timestampsPositionOffset,
                                                this->timestampsType,
                                                timestampsInput);
  }

  Status dataStatus = this->data->writeDataBlock(
      dataShape, positionOffset, this->dataType, dataInput);

  if ((dataStatus != Status::Success) || (tsStatus != Status::Success)) {
    return Status::Failure;
  } else {
    return Status::Success;
  }
}
