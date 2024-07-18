#include "nwb/base/TimeSeries.hpp"

using namespace AQNWB::NWB;

// TimeSeries

/** Constructor */
TimeSeries::TimeSeries(const std::string& path,
                       std::shared_ptr<BaseIO> io,
                       const std::string& description,
                       const std::string& comments)
    : Container(path, io)
    , description(description)
    , comments(comments)
{
}

/** Destructor */
TimeSeries::~TimeSeries() {}

void TimeSeries::initialize()
{
  Container::initialize();

  io->createCommonNWBAttributes(path, "core", neurodataType, description);
  io->createAttribute(comments, path, "comments");
}

Status TimeSeries::writeDataBlock(const std::vector<SizeType>& dataShape,
                                  const std::vector<SizeType>& positionOffset,
                                  const BaseDataType& dataType,
                                  const void* data,
                                  const BaseDataType& timestampsType,
                                  const void* timestamps)
{
  Status tsStatus = Status::Success;
  if (timestamps != nullptr) {
    const std::vector<SizeType> timestampsShape = {
        dataShape[0]};  // timestamps should match shape of the first data
                        // dimension
    const std::vector<SizeType> timestampsPositionOffset = {positionOffset[0]};
    tsStatus = this->timestamps->writeDataBlock(
        timestampsShape, timestampsPositionOffset, timestampsType, timestamps);
  }

  Status dataStatus =
      this->data->writeDataBlock(dataShape, positionOffset, dataType, data);

  if ((dataStatus != Status::Success) or (tsStatus != Status::Success)) {
    return Status::Failure;
  } else {
    return Status::Success;
  }
}
