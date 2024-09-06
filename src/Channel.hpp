#pragma once

#include <array>
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
  Channel(const std::string name,
          const std::string groupName,
          const SizeType groupIndex,
          const SizeType localIndex,
          const SizeType globalIndex,
          const float conversion = 1e6f,  // uV to V
          const float samplingRate = 30000.f,  // placeholder
          const float bitVolts = 0.05f,  // least significant bit needed to
                                         // convert 16-bit int to volts
                                         // currently a placeholder
          const std::array<float, 3> position = {0.f, 0.f, 0.f},
          const std::string comments = "no comments");

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
   * @brief Index of array group the channel belongs to.
   */
  SizeType groupIndex;

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
  std::array<float, 3> position;

  /**
   * @brief Comments about the channel.
   */
  std::string comments;

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
