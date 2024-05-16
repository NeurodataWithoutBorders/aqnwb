#pragma once

#include <string>

namespace AQNWB
{
/**
 * @brief Class for storing aquisition system channel information.
 */
class Channel
{
public:
  /**
   * @brief Constructor.
   */
  Channel(std::string name,
          std::string groupName,
          int localIndex,
          int globalIndex,
          float conversion = 1e6,
          float samplingRate = 30000,
          std::vector<float> position = {0, 0, 0})
      : name(name)
      , groupName(groupName)
      , conversion(conversion)
      , samplingRate(samplingRate)
      , localIndex(localIndex)
      , globalIndex(globalIndex)
      , position(position)
  {
  }

  /**
   * @brief Destructor
   */
  ~Channel() {}

  /**
   * @brief Name of the channel.
   */
  std::string name;

  /**
   * @brief Name of the array group the channel belongs to.
   */
  std::string groupName;

  /**
   * @brief Conversion factor.
   */
  float conversion;

  /**
   * @brief Sampling rate of the channel.
   */
  float samplingRate;

  /**
   * @brief Index of channel within the recording array.
   */
  int localIndex;

  /**
   * @brief Index of channel across the recording system.
   */
  int globalIndex;

  /**
   * @brief Coordinates of channel (x, y, z) within the recording array.
   */
  std::vector<float> position;

private:
};
}  // namespace AQNWB
