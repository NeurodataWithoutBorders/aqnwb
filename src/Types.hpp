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
