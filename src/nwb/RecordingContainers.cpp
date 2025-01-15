
#include "nwb/RecordingContainers.hpp"

#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "nwb/misc/AnnotationSeries.hpp"

using namespace AQNWB::NWB;
// Recording Container

RecordingContainers::RecordingContainers() {}

RecordingContainers::~RecordingContainers() {}

void RecordingContainers::addContainer(std::unique_ptr<Container> container)
{
  m_containers.push_back(std::move(container));
}

Container* RecordingContainers::getContainer(const SizeType& containerInd)
{
  if (containerInd >= m_containers.size()) {
    return nullptr;
  } else {
    return m_containers[containerInd].get();
  }
}

Status RecordingContainers::writeTimeseriesData(
    const SizeType& containerInd,
    const Channel& channel,
    const std::vector<SizeType>& dataShape,
    const std::vector<SizeType>& positionOffset,
    const void* data,
    const void* timestamps,
    const void* controlInput)
{
  TimeSeries* ts = dynamic_cast<TimeSeries*>(getContainer(containerInd));

  if (ts == nullptr)
    return Status::Failure;

  // write data and timestamps to datasets
  if (channel.getLocalIndex() == 0) {
    // write with timestamps if it's the first channel
    return ts->writeData(
        dataShape, positionOffset, data, timestamps, controlInput);
  } else {
    // write without timestamps and controlInput if its another channel in the
    // same timeseries
    return ts->writeData(dataShape, positionOffset, data);
  }
}

Status RecordingContainers::writeElectricalSeriesData(
    const SizeType& containerInd,
    const Channel& channel,
    const SizeType& numSamples,
    const void* data,
    const void* timestamps,
    const void* controlInput)
{
  ElectricalSeries* es =
      dynamic_cast<ElectricalSeries*>(getContainer(containerInd));

  if (es == nullptr)
    return Status::Failure;

  return es->writeChannel(
      channel.getLocalIndex(), numSamples, data, timestamps, controlInput);
}

Status RecordingContainers::writeSpikeEventData(const SizeType& containerInd,
                                                const SizeType& numSamples,
                                                const SizeType& numChannels,
                                                const void* data,
                                                const void* timestamps,
                                                const void* controlInput)
{
  SpikeEventSeries* ses =
      dynamic_cast<SpikeEventSeries*>(getContainer(containerInd));

  if (ses == nullptr)
    return Status::Failure;

  return ses->writeSpike(
      numSamples, numChannels, data, timestamps, controlInput);
}

Status RecordingContainers::writeAnnotationSeriesData(
    const SizeType& containerInd,
    const SizeType& numSamples,
    const std::vector<std::string> data,
    const void* timestamps,
    const void* controlInput)
{
  AnnotationSeries* as =
      dynamic_cast<AnnotationSeries*>(getContainer(containerInd));

  if (as == nullptr)
    return Status::Failure;

  return as->writeAnnotation(numSamples, data, timestamps, controlInput);
}