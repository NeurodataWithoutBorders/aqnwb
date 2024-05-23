#pragma once

#include <string>

namespace AQNWB
{
/**
 * @brief Class for storing acquisition system channel information.
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
          float conversion = 1e6f,  // uV to V
          float samplingRate = 30000.f,
          float bitVolts = 0.000002f,  // least significant bit needed to convert 16-bit int to volts
          std::vector<float> position = {0.f, 0.f, 0.f})
      : name(name)
      , groupName(groupName)
      , conversion(conversion)
      , samplingRate(samplingRate)
      , bitVolts(bitVolts)
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
   * @brief Getter for conversion factor
   * @return The conversion value.
   */
  float getConversion() const { return bitVolts / conversion; }

  /**
   * @brief Getter for samplingRate
   * @return The samplingRate value.
   */
  float getSamplingRate() const { return samplingRate; }

  /**
   * @brief Getter for bitVolts
   * @return The bitVolts value.
   */
  float getBitVolts() const { return bitVolts; }

  /**
   * @brief Name of the channel.
   */
  std::string name;

  /**
   * @brief Name of the array group the channel belongs to.
   */
  std::string groupName;

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
  /**
   * @brief Conversion factor.
   */
  float conversion;

  /**
   * @brief Sampling rate of the channel.
   */
  float samplingRate;

  /**
   * @brief floating point value of microvolts per bit
   */
  float bitVolts;
};
}  // namespace AQNWB
