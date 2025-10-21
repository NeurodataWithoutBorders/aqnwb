#pragma once

#include "Channel.hpp"
#include "Types.hpp"
#include "nwb/base/TimeSeries.hpp"

/*!
 * \namespace AQNWB::NWB
 * \brief  \brief Namespace for all classes related to the NWB data standard
 */
namespace AQNWB::NWB
{
// Forward declaration
class RegisteredType;
}  // namespace AQNWB::NWB

namespace AQNWB::IO
{

/**
 * @brief The RecordingObjects class provides an interface for managing
 * and holding groups of RegisteredType objects used for recording
 * during data acquisition.
 */

class RecordingObjects
{
public:
  /**
   * @brief Constructor for RecordingObjects class.
   */
  RecordingObjects();

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  RecordingObjects(const RecordingObjects&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  RecordingObjects& operator=(const RecordingObjects&) = delete;

  /**
   * @brief Destructor for RecordingObjects class.
   */
  ~RecordingObjects();

  /**
   * @brief Adds a RegisteredType object to the recording objects collection.
   * @param object The RegisteredType object to add as a shared pointer.
   * @return The index of the added object in the collection.
   */
  SizeType addRecordingObject(
      const std::shared_ptr<AQNWB::NWB::RegisteredType>& object);

  /**
   * @brief Gets the RegisteredType object from the recording objects collection
   * @param objectInd The index of the object within the collection.
   */
  std::shared_ptr<AQNWB::NWB::RegisteredType> getRecordingObject(
      const SizeType& objectInd);

  /**
   * @brief Gets the RegisteredType object from the recording objects collection
   * based on the path of the object.
   * NOTE: This function performs a linear search. If for some reason multiple
   * objects with the same path exist, the first one found will be returned.
   * @param path The path of the object to retrieve.
   * @return Shared pointer to the RegisteredType object if found, nullptr
   *         otherwise.
   */
  std::shared_ptr<AQNWB::NWB::RegisteredType> getRecordingObject(
      const std::string& path) const;

  /**
   * @brief Finds the index of a RegisteredType object in the recording objects
   * collection.
   * @param object The pointer to the RegisteredType object to find.
   * @return The index of the object in the collection, or
   * std::numeric_limits<SizeType>::max() if not found.
   */
  SizeType getRecordingIndex(
      const std::shared_ptr<const AQNWB::NWB::RegisteredType>& object) const;

  /**
   * @brief Clear the recording objects collection.
   */
  void clear() { m_recording_objects.clear(); }

  /**
   * @brief Finalize all RegisteredType objects managed by this RecordingObjects
   * instance. This method calls finalize() on all objects in the collection.
   * @return The status of the finalize operation.
   */
  Status finalize();

  /**
   * @brief Clear recording data cache for all RegisteredType objects
   * managed by this RecordingObjects instance. This method calls
   * clearRecordingDataCache() on all objects in the collection.
   * @return The status of the clear operation.
   */
  Status clearRecordingDataCache();

  /**
   * @brief Get the number of recording objects
   */
  inline SizeType size() const { return m_recording_objects.size(); }

  /**
   * @brief Create a string representation of the RecordingObjects contents
   * @return A string listing the index, types, and paths of all RegisteredType
   *         objects manages by this RecordingObjects instance.
   */
  std::string toString() const;

private:
  /**
   * @brief The RegisteredType objects used for recording
   */
  std::vector<std::shared_ptr<AQNWB::NWB::RegisteredType>> m_recording_objects;

  /**
   * @brief The name of the collection of recording objects
   */
  std::string m_name;
};

}  // namespace AQNWB::IO
