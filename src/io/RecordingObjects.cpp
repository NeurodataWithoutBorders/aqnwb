#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

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
  if (!object) {
    return AQNWB::Types::SizeTypeNotSet;
  }
  auto it = m_path_to_index.find(object->getPath());
  if (it != m_path_to_index.end()) {
    return it->second;
  }
  return AQNWB::Types::SizeTypeNotSet;
}

SizeType RecordingObjects::addRecordingObject(
    const std::shared_ptr<AQNWB::NWB::RegisteredType>& object)
{
  if (!object) {
    return AQNWB::Types::SizeTypeNotSet;
  }

  // Check if object already exists in the map
  auto it = m_path_to_index.find(object->getPath());
  if (it != m_path_to_index.end()) {
    // Update the object in the vector if it's a different instance
    if (m_recording_objects[it->second].get() != object.get()) {
      throw std::runtime_error(
          "Attempting to add a different instance of a RegisteredType with the "
          "same path.");
    }
    return it->second;
  } else {
    // If not found, add it and return the new index
    m_recording_objects.push_back(object);
    SizeType newIndex = m_recording_objects.size() - 1;
    m_path_to_index[object->getPath()] = newIndex;
    return newIndex;
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

std::shared_ptr<AQNWB::NWB::RegisteredType>
RecordingObjects::getRecordingObject(const std::string& path) const
{
  auto it = m_path_to_index.find(path);
  if (it != m_path_to_index.end()) {
    return m_recording_objects[it->second];
  }
  return nullptr;
}

Status RecordingObjects::finalize()
{
  Status overallStatus = Status::Success;

  // Call finalize on all RegisteredType objects in the collection
  for (auto& object : m_recording_objects) {
    if (object) {
      Status status = object->finalize();
      overallStatus = overallStatus && status;
    }
  }
  return overallStatus;
}

std::string RecordingObjects::toString() const
{
  std::stringstream result;
  result << "RecordingObjects contents:\n";
  for (SizeType i = 0; i < m_recording_objects.size(); ++i) {
    auto obj = m_recording_objects[i];
    if (obj) {
      result << "Index = " << i << "; " << "Type = " << obj->getFullTypeName()
             << "; " << "Path = " << obj->getPath() << "; " << std::endl;
    } else {
      result << "  [" + std::to_string(i) + "] <null object>\n";
    }
  }
  return result.str();
}