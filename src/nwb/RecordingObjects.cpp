
#include "nwb/RecordingObjects.hpp"

#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "nwb/misc/AnnotationSeries.hpp"
#include "nwb/RegisteredType.hpp"

using namespace AQNWB::NWB;
// Recording Objects

RecordingObjects::RecordingObjects() {}

RecordingObjects::~RecordingObjects() {}

void RecordingObjects::addRecordingObject(std::shared_ptr<RegisteredType> object)
{
  m_recording_objects.push_back(object);
}

std::shared_ptr<RegisteredType> RecordingObjects::getRecordingObject(const SizeType& objectInd)
{
  if (objectInd >= m_recording_objects.size()) {
    return nullptr;
  } else {
    return m_recording_objects[objectInd];
  }
}

Status RecordingObjects::finalize()
{
  Status overallStatus = Status::Success;
  
  // Call finalize on all RegisteredType objects in the collection
  for (auto& object : m_recording_objects) {
    if (object) {
      Status status = object->finalize();
      if (status != Status::Success) {
        overallStatus = status;
      }
    }
  }
  
  return overallStatus;
}

Status RecordingObjects::writeTimeseriesData(
    const SizeType& containerInd,
    const Channel& channel,
    const std::vector<SizeType>& dataShape,
    const std::vector<SizeType>& positionOffset,
    const void* data,
    const void* timestamps,
    const void* controlInput)
{
  auto registeredObject = getRecordingObject(containerInd);
  // Cast to TimeSeries
  // This assumes that the object at containerInd is a TimeSeries
  auto ts = std::dynamic_pointer_cast<NWB::TimeSeries>(registeredObject);
  if (ts == nullptr){
    return Status::Failure;
  }

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

Status RecordingObjects::writeElectricalSeriesData(
    const SizeType& containerInd,
    const Channel& channel,
    const SizeType& numSamples,
    const void* data,
    const void* timestamps,
    const void* controlInput)
{
  auto registeredObject = getRecordingObject(containerInd);
  auto es = std::dynamic_pointer_cast<NWB::ElectricalSeries>(registeredObject);

  if (es == nullptr)
    return Status::Failure;

  return es->writeChannel(
      channel.getLocalIndex(), numSamples, data, timestamps, controlInput);
}

Status RecordingObjects::writeSpikeEventData(const SizeType& containerInd,
                                                const SizeType& numSamples,
                                                const SizeType& numChannels,
                                                const void* data,
                                                const void* timestamps,
                                                const void* controlInput)
{
  auto registeredObject = getRecordingObject(containerInd);
  auto ses = std::dynamic_pointer_cast<NWB::SpikeEventSeries>(registeredObject);

  if (ses == nullptr)
    return Status::Failure;

  return ses->writeSpike(
      numSamples, numChannels, data, timestamps, controlInput);
}

Status RecordingObjects::writeAnnotationSeriesData(
    const SizeType& containerInd,
    const SizeType& numSamples,
    const std::vector<std::string> data,
    const void* timestamps,
    const void* controlInput)
{
  auto registeredObject = getRecordingObject(containerInd);
  auto as = std::dynamic_pointer_cast<NWB::AnnotationSeries>(registeredObject);
  
  if (as == nullptr)
    return Status::Failure;

  return as->writeAnnotation(numSamples, data, timestamps, controlInput);
}
