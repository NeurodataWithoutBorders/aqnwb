#pragma once

#include <string>

#include "Types.hpp"

using SizeType = AQNWB::Types::SizeType;

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
          SizeType localIndex,
          SizeType globalIndex,
          float conversion = 1e6f,  // uV to V
          float samplingRate = 30000.f,  // placeholder
          float bitVolts = 0.000002f,  // least significant bit needed to
                                       // convert 16-bit int to volts
                                       // currently a placeholder
          std::vector<float> position = {0.f, 0.f, 0.f});

  /**
   * @brief Destructor
   */
  ~Channel();

  /**
   * @brief Getter for conversion factor
   * @return The conversion value.
   */
  float getConversion() const;

  /**
   * @brief Getter for samplingRate
   * @return The samplingRate value.
   */
  float getSamplingRate() const;
  /**
   * @brief Getter for bitVolts
   * @return The bitVolts value.
   */
  float getBitVolts() const;

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
  SizeType localIndex;

  /**
   * @brief Index of channel across the recording system.
   */
  SizeType globalIndex;

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
