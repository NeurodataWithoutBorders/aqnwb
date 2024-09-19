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
  this->m_io->createCommonNWBAttributes(
      this->m_path, "core", neurodataType, description);
  this->m_io->createAttribute(comments, this->m_path, "comments");

  // setup datasets
  this->data =
      std::unique_ptr<BaseRecordingData>(this->m_io->createArrayDataSet(
          dataType, dsetSize, chunkSize, this->m_path + "/data"));
  this->m_io->createDataAttributes(this->m_path, conversion, resolution, unit);

  SizeArray tsDsetSize = {
      dsetSize[0]};  // timestamps match data along first dimension
  this->timestamps = std::unique_ptr<BaseRecordingData>(
      this->m_io->createArrayDataSet(this->timestampsType,
                                     tsDsetSize,
                                     chunkSize,
                                     this->m_path + "/timestamps"));
  this->m_io->createTimestampsAttributes(this->m_path);
}

Status TimeSeries::writeData(const std::vector<SizeType>& dataShape,
                             const std::vector<SizeType>& positionOffset,
                             const void* data,
                             const void* timestamps)
{
  Status tsStatus = Status::Success;
  if (timestamps != nullptr) {
    const std::vector<SizeType> timestampsShape = {
        dataShape[0]};  // timestamps should match shape of the first data
                        // dimension
    const std::vector<SizeType> timestampsPositionOffset = {positionOffset[0]};
    tsStatus = this->timestamps->writeDataBlock(timestampsShape,
                                                timestampsPositionOffset,
                                                this->timestampsType,
                                                timestamps);
  }

  Status dataStatus = this->data->writeDataBlock(
      dataShape, positionOffset, this->dataType, data);

  if ((dataStatus != Status::Success) or (tsStatus != Status::Success)) {
    return Status::Failure;
  } else {
    return Status::Success;
  }
}
