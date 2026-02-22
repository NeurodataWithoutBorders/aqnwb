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
  Channel(const std::string& name,
          const std::string& groupName,
          const SizeType groupIndex,
          const SizeType localIndex,
          const SizeType globalIndex,
          const float conversion = 1e6f,  // uV to V
          const float samplingRate = 30000.f,  // placeholder
          const float bitVolts = 0.05f,  // least significant bit needed to
                                         // convert 16-bit int to volts
                                         // currently a placeholder
          const std::array<float, 3> position = {0.f, 0.f, 0.f},
          const std::string& comments = "no comments");

  /**
   * @brief Destructor
   */
  ~Channel();

  /**
   * @brief Copy constructor
   */
  Channel(const Channel& other) = default;

  /**
   * @brief Move constructor
   */
  Channel(Channel&& other) = default;

  /**
   * @brief Assignment operator
   */
  Channel& operator=(const Channel& other) = default;

  /**
   * @brief Move assignment operator
   */
  Channel& operator=(Channel&& other) = default;

  /**
   * @brief Getter for conversion factor
   * @return The conversion value.
   */
  inline float getConversion() const { return m_bitVolts / m_conversion; }

  /**
   * @brief Getter for sampling rate of the channel.
   * @return The sampling rate value.
   */
  inline float getSamplingRate() const { return m_samplingRate; }

  /**
   * @brief Getter for bitVolts floating point value of microvolts per bit
   * @return The bitVolts value.
   */
  inline float getBitVolts() const { return m_bitVolts; }

  /**
   * @brief Get the name of the array group the channel belongs to.
   * @return The groupName value.
   */
  inline std::string getGroupName() const { return m_groupName; }

  /**
   * @brief Get the name of the channel
   * @return The name value.
   */
  inline std::string getName() const { return m_name; }

  /**
   * @brief Get the array group index the channel belongs to
   * @return The groupIndex value.
   */
  inline SizeType getGroupIndex() const { return m_groupIndex; }

  /**
   * @brief Get the index of the channel within the recording array.
   * @return The localIndex value.
   */
  inline SizeType getLocalIndex() const { return m_localIndex; }

  /**
   * @brief Get the index of the channel across the recording system.
   * @return The globalIndex value.
   */
  inline SizeType getGlobalIndex() const { return m_globalIndex; }

  /**
   * @brief Get the coordinates of channel (x, y, z) within the recording array.
   * @return The position value.
   */
  inline const std::array<float, 3>& getPosition() const { return m_position; }

  /**
   * @brief Get comments about the channel
   * @return The comments value.
   */
  inline std::string getComments() const { return m_comments; }

  /**
   * @brief Set comments about the channel.
   * @param comments The comments to set.
   */
  inline void setComments(const std::string& comments)
  {
    m_comments = comments;
  }

  /**
   * @brief Set coordinates of channel (x, y, z) within the recording array.
   * @param position The position to set.
   */
  inline void setPosition(const std::array<float, 3>& position)
  {
    m_position = position;
  }

  /**
   * @brief Set name of the channel.
   * @param name The name to set.
   */
  inline void setName(const std::string& name) { m_name = name; }

private:
  /**
   * @brief Name of the channel.
   */
  std::string m_name;

  /**
   * @brief Name of the array group the channel belongs to.
   */
  std::string m_groupName;

  /**
   * @brief Index of array group the channel belongs to.
   */
  SizeType m_groupIndex;

  /**
   * @brief Index of channel within the recording array.
   */
  SizeType m_localIndex;

  /**
   * @brief Index of channel across the recording system.
   */
  SizeType m_globalIndex;

  /**
   * @brief Conversion factor.
   */
  float m_conversion;

  /**
   * @brief Sampling rate of the channel.
   */
  float m_samplingRate;

  /**
   * @brief floating point value of microvolts per bit
   */
  float m_bitVolts;

  /**
   * @brief Coordinates of channel (x, y, z) within the recording array.
   */
  std::array<float, 3> m_position;

  /**
   * @brief Comments about the channel.
   */
  std::string m_comments;
};
}  // namespace AQNWB