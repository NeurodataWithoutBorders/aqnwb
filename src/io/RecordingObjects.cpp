
#include <limits>

#include "io/RecordingObjects.hpp"

#include "nwb/RegisteredType.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "nwb/misc/AnnotationSeries.hpp"

using namespace AQNWB::IO;
// Recording Objects

RecordingObjects::RecordingObjects() {}

RecordingObjects::~RecordingObjects() {}

SizeType RecordingObjects::getRecordingIndex(
    const std::shared_ptr<const AQNWB::NWB::RegisteredType>& object) const
{
  // Check if object already exists in the vector
  for (SizeType i = 0; i < m_recording_objects.size(); ++i) {
    // compares identy of the raw pointer for the RegisteredType object
    if (m_recording_objects[i].get() == object.get()) {
      return i;  // Return existing index
    }
  }
  // Return sentinel value for failure
  return std::numeric_limits<SizeType>::max();
}

SizeType RecordingObjects::addRecordingObject(
    const std::shared_ptr<AQNWB::NWB::RegisteredType>& object)
{
  // Check if object already exists in the vector
  SizeType objectIndex = getRecordingIndex(object);
  if (isValidIndex(objectIndex)) {
    return objectIndex;  // Return existing index if found
  } else {
    // If not found, add it and return the new index
    m_recording_objects.push_back(object);
    return m_recording_objects.size() - 1;
  }
}

std::shared_ptr<AQNWB::NWB::RegisteredType>
RecordingObjects::getRecordingObject(const SizeType& objectInd)
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

Status RecordingObjects::clearRecordingDataCache()
{
  Status overallStatus = Status::Success;

  // Call clearRecordingDataCache on all RegisteredType objects in the
  // collection
  for (auto& object : m_recording_objects) {
    if (object) {
      try {
        object->clearRecordingDataCache();
      } catch (const std::exception& e) {
        std::cerr << "Error clearing recording data cache for object at path: "
                  << object->getPath() << ". Error: " << e.what() << std::endl;
        overallStatus = Status::Failure;
      }
    }
  }
  return overallStatus;
}
