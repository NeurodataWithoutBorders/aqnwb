#pragma once

#include <cstddef>
#include <vector>

namespace AQNWB
{

// Forward declaration of Channel
class Channel;

/**
 * @brief Provides definitions for various types used in the project.
 */
class Types
{
public:
  /**
   * @brief Represents the status of an operation.
   */
  enum Status
  {
    Success = 0,
    Failure = -1
  };

  /**
   * @brief Overloaded && operator for Status enum
   * @param lhs Left-hand side Status
   * @param rhs Right-hand side Status
   * @return Success if both statuses are Success, Failure otherwise
   */
  friend Status operator&&(Status lhs, Status rhs)
  {
    return (lhs == Success && rhs == Success) ? Success : Failure;
  }

  /**
   * @brief Overloaded || operator for Status enum
   * @param lhs Left-hand side Status
   * @param rhs Right-hand side Status
   * @return Success if either status is Success, Failure otherwise
   */
  friend Status operator||(Status lhs, Status rhs)
  {
    return (lhs == Success || rhs == Success) ? Success : Failure;
  }

  /**
   * @brief Types of object used in the NWB schema
   */
  enum StorageObjectType
  {
    Group = 0,
    Dataset = 1,
    Attribute = 2,
    Undefined = -1
  };

  /**
   *  \brief Helper struct to check if a value is a data field, i.e.,
   * Dataset or Attribute
   *
   * This function is used to enforce constraints on templated functions that
   * should only be callable for valid StorageObjectType values
   */
  template<StorageObjectType T>
  struct IsDataStorageObjectType
      : std::integral_constant<bool, (T == Dataset || T == Attribute)>
  {
  };

  /**
   * @brief Alias for the size type used in the project.
   */
  using SizeType = size_t;

  /**
   * @brief Alias for an array of size types used in the project.
   */
  using SizeArray = std::vector<size_t>;

  /**
   * @brief Alias for a vector of channels.
   */
  using ChannelVector = std::vector<Channel>;
};
}  // namespace AQNWB
