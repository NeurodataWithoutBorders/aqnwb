#include "nwb/base/TimeSeries.hpp"

using namespace AQNWB::NWB;

// TimeSeries

/** Constructor */
TimeSeries::TimeSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
TimeSeries::~TimeSeries() {}

void TimeSeries::initialize(const IO::BaseDataType& dataType,
                            const std::string& unit,
                            const std::string& description,
                            const std::string& comments,
                            const SizeArray& dsetSize,
                            const SizeArray& chunkSize,
                            const float& conversion,
                            const float& resolution,
                            const float& offset)
{
  Container::initialize();

  this->dataType = dataType;

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
