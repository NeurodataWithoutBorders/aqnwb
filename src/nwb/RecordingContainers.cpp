
#include "nwb/RecordingContainers.hpp"

#include "nwb/base/TimeSeries.hpp"

using namespace AQNWB::NWB;
// Recording Container

RecordingContainers::RecordingContainers() {}

RecordingContainers::~RecordingContainers() {}

void RecordingContainers::addData(std::unique_ptr<TimeSeries> data)
{
  this->containers.push_back(std::move(data));
}

TimeSeries* RecordingContainers::getTimeSeries(const SizeType& timeseriesInd)
{
  if (timeseriesInd >= this->containers.size()) {
    return nullptr;
  } else {
    return this->containers[timeseriesInd].get();
  }
}

Status RecordingContainers::writeTimeseriesData(
    const SizeType& timeseriesInd,
    const Channel& channel,
    const std::vector<SizeType>& dataShape,
    const std::vector<SizeType>& positionOffset,
    const void* data,
    const void* timestamps)
{
  TimeSeries* ts = getTimeSeries(timeseriesInd);

  if (ts == nullptr)
    return Status::Failure;

  // write data and timestamps to datasets
  if (channel.localIndex == 0) {
    // write with timestamps if it's the first channel
    return ts->writeData(dataShape, positionOffset, data, timestamps);
  } else {
    // write without timestamps if its another channel in the same timeseries
    return ts->writeData(dataShape, positionOffset, data);
  }
}
