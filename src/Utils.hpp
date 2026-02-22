#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <random>
#include <regex>
#include <sstream>
#include <string>

#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"

namespace AQNWB
{
namespace detail
{
/**
 * @brief Convert a std::time_t value to a local std::tm structure.
 * @param time_value The time value to convert.
 * @return The corresponding local std::tm structure.
 */
inline std::tm to_local_time(std::time_t time_value)
{
  std::tm local_tm {};
#if defined(_WIN32)
  localtime_s(&local_tm, &time_value);
#elif defined(__unix__) || defined(__APPLE__)
  localtime_r(&time_value, &local_tm);
#else
  const std::tm* local_tm_ptr = std::localtime(&time_value);
  if (local_tm_ptr) {
    local_tm = *local_tm_ptr;
  }
#endif
  return local_tm;
}

/**
 * @brief Convert a std::time_t value to a UTC std::tm structure.
 * @param time_value The time value to convert.
 * @return The corresponding UTC std::tm structure.
 */
inline std::tm to_utc_time(std::time_t time_value)
{
  std::tm utc_tm {};
#if defined(_WIN32)
  gmtime_s(&utc_tm, &time_value);
#elif defined(__unix__) || defined(__APPLE__)
  gmtime_r(&time_value, &utc_tm);
#else
  const std::tm* utc_tm_ptr = std::gmtime(&time_value);
  if (utc_tm_ptr) {
    utc_tm = *utc_tm_ptr;
  }
#endif
  return utc_tm;
}

/**
 * @brief Get the UTC offset in seconds for a given time_t value.
 * @param time_value The time value to check.
 * @return The offset from UTC in seconds.
 */
inline long get_utc_offset_seconds(std::time_t time_value)
{
  std::tm local_tm = to_local_time(time_value);
  std::tm utc_tm = to_utc_time(time_value);

  std::time_t local_time = std::mktime(&local_tm);
  std::time_t utc_time = std::mktime(&utc_tm);
  if (local_time == static_cast<std::time_t>(-1)
      || utc_time == static_cast<std::time_t>(-1))
  {
    return 0;
  }
  return static_cast<long>(std::difftime(local_time, utc_time));
}

/**
 * @brief Format a UTC offset in seconds as a string (+HH:MM or -HH:MM).
 * @param offset_seconds The offset in seconds.
 * @return The formatted UTC offset string.
 */
inline std::string format_utc_offset(long offset_seconds)
{
  const char sign = (offset_seconds < 0) ? '-' : '+';
  long abs_offset = (offset_seconds < 0) ? -offset_seconds : offset_seconds;
  long hours = abs_offset / 3600;
  long minutes = (abs_offset % 3600) / 60;

  std::ostringstream oss;
  oss << sign << std::setw(2) << std::setfill('0') << hours << ':'
      << std::setw(2) << std::setfill('0') << minutes;
  return oss.str();
}

/**
 * @brief Convert a 16-bit unsigned integer to little-endian byte order.
 * @param value The value to convert.
 * @return The value in little-endian byte order.
 */
inline uint16_t to_little_endian_u16(uint16_t value)
{
#if defined(_WIN32) \
    || (defined(__BYTE_ORDER__) \
        && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
  return value;
#else
  return static_cast<uint16_t>((value >> 8) | (value << 8));
#endif
}
}  // namespace detail

/**
 * @brief Generates a UUID (Universally Unique Identifier) as a string.
 * @return The generated UUID as a string.
 */
static inline std::string generateUuid()
{
  std::array<uint8_t, 16> bytes {};
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  for (size_t i = 0; i < bytes.size(); i += 4) {
    uint32_t random_value = dist(gen);
    bytes[i] = static_cast<uint8_t>(random_value & 0xFF);
    bytes[i + 1] = static_cast<uint8_t>((random_value >> 8) & 0xFF);
    bytes[i + 2] = static_cast<uint8_t>((random_value >> 16) & 0xFF);
    bytes[i + 3] = static_cast<uint8_t>((random_value >> 24) & 0xFF);
  }

  // RFC 4122 version 4 UUID.
  bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
  bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

  std::ostringstream oss;
  oss << std::hex << std::nouppercase << std::setfill('0');
  for (size_t i = 0; i < bytes.size(); ++i) {
    if (i == 4 || i == 6 || i == 8 || i == 10) {
      oss << '-';
    }
    oss << std::setw(2) << static_cast<int>(bytes[i]);
  }
  return oss.str();
}

/**
 * @brief Get the current time in ISO 8601 format with the UTC offset.
 * @return The current time as a string in ISO 8601 format.
 */
static inline std::string getCurrentTime()
{
  auto now = std::chrono::system_clock::now();
  auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto micros =
      std::chrono::duration_cast<std::chrono::microseconds>(now - seconds)
          .count();
  std::time_t time_value = std::chrono::system_clock::to_time_t(seconds);
  std::tm local_tm = detail::to_local_time(time_value);
  long offset_seconds = detail::get_utc_offset_seconds(time_value);

  std::ostringstream oss;
  oss << std::put_time(&local_tm, "%Y-%m-%dT%H:%M:%S");
  oss << '.' << std::setw(6) << std::setfill('0') << micros;
  oss << detail::format_utc_offset(offset_seconds);
  return oss.str();
}

/**
 * @brief Check that a string is formatted in ISO8601 format
 *
 * This function only validates the regex pattern but does not check that
 * the time values specified are indeed valid.
 *
 * @return bool indicating whether the string is in ISO8601 form
 */
static inline bool isISO8601Date(const std::string& dateStr)
{
  // Define the regex pattern for ISO 8601 extended format with timezone offset
  // Allow one or more fractional seconds digits
  const std::string iso8601Pattern =
      R"(^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d+[+-]\d{2}:\d{2}$)";
  std::regex pattern(iso8601Pattern);

  // Check if the date string matches the regex pattern
  return std::regex_match(dateStr, pattern);
}

/**
 * @brief Factory method to create an IO object of the specified type.
 * @param type The type of IO object to create (e.g., "HDF5").
 * @param filename The filename to use for the IO object.
 * @return A shared pointer to a BaseIO object.
 * @throws std::invalid_argument if the type is invalid.
 */
static inline std::shared_ptr<IO::BaseIO> createIO(const std::string& type,
                                                   const std::string& filename)
{
  if (type == "HDF5") {
    return std::make_shared<AQNWB::IO::HDF5::HDF5IO>(filename);
  } else {
    throw std::invalid_argument("Invalid IO type");
  }
}

/**
 * @brief Merge two paths into a single path, handling extra trailing and
 * starting "/".
 *
 * This function is useful for constructing paths in HDF5 files, where paths
 * always use "/".
 *
 * @param path1 The first path to merge.
 * @param path2 The second path to merge.
 *
 * @return The merged path with no extra "/" at the start or end, and exactly
 * one "/" between the two input paths.
 */
static inline std::string mergePaths(const std::string& path1,
                                     const std::string& path2)
{
  std::string result = path1;
  // Remove trailing "/" from path1
  while (!result.empty() && result.back() == '/' && result != "/") {
    result.pop_back();
  }
  // Remove leading "/" from path2
  size_t start = 0;
  while (start < path2.size() && path2[start] == '/') {
    start++;
  }
  // Get path2 without trailing slashes
  std::string path2Clean = path2.substr(start);
  while (!path2Clean.empty() && path2Clean.back() == '/' && path2Clean != "/") {
    path2Clean.pop_back();
  }
  // Append path2 to path1 with a "/" in between
  if (!result.empty() && !path2Clean.empty()) {
    result += '/';
  }
  result += path2Clean;

  // Remove any potential occurrences of "//" and replace with "/"
  size_t pos = result.find("//");
  while (pos != std::string::npos) {
    result.replace(pos, 2, "/");
    pos = result.find("//", pos);
  }

  // Remove trailing "/" from final result if not root path
  while (!result.empty() && result.back() == '/' && result != "/") {
    result.pop_back();
  }

  return result;
}

/**
 * @brief Method to convert float values to uint16 values. This method
 * was adapted from JUCE AudioDataConverters using a default value of
 * destBytesPerSample = 2.
 * @param source The source float data to convert
 * @param dest The destination for the converted uint16 data
 * @param numSamples The number of samples to convert
 */
static inline void convertFloatToInt16LE(const float* source,
                                         void* dest,
                                         SizeType numSamples)
{
  // TODO - several steps in this function may be unnecessary for our use
  // case. Consider simplifying the intermediate cast to char and the
  // final cast to uint16_t.
  auto maxVal = static_cast<double>(0x7fff);
  auto intData = static_cast<char*>(dest);

  for (SizeType i = 0; i < numSamples; ++i) {
    auto clampedValue =
        std::clamp(maxVal * static_cast<double>(source[i]), -maxVal, maxVal);
    auto intValue =
        static_cast<uint16_t>(static_cast<int16_t>(std::round(clampedValue)));
    intValue = detail::to_little_endian_u16(intValue);
    *reinterpret_cast<uint16_t*>(intData) = intValue;
    intData += 2;  // destBytesPerSample is always 2
  }
}

/**
 * @brief Method to scale float values and convert to int16 values
 * @param numSamples The number of samples to convert
 * @param conversion_factor The conversion factor to scale the data
 * @param data The data to convert
 */
static inline std::unique_ptr<int16_t[]> transformToInt16(
    SizeType numSamples, float conversion_factor, const float* data)
{
  std::unique_ptr<float[]> scaledData = std::make_unique<float[]>(numSamples);
  std::unique_ptr<int16_t[]> intData = std::make_unique<int16_t[]>(numSamples);

  // copy data and multiply by scaling factor
  float multFactor = 1.0f / (32767.0f * conversion_factor);
  std::transform(data,
                 data + numSamples,
                 scaledData.get(),
                 [multFactor](float value) { return value * multFactor; });

  // convert float to int16
  convertFloatToInt16LE(scaledData.get(), intData.get(), numSamples);

  return intData;
}

/**
 * @brief Check if a SizeType index is valid (i.e., not equal to SizeTypeNotSet)
 * @param index The index to check
 * @return True if the index is valid, false otherwise
 */
static inline bool isValidIndex(SizeType index)
{
  return (index != AQNWB::Types::SizeTypeNotSet);
}

/**
 * @brief Convert an integer status code to a Types::Status enum value.
 * Shorthand for `return (status < 0) ? Status::Failure : Status::Success;`
 * @param status The integer status code to convert.
 * @return The corresponding Types::Status enum value.
 */
static inline Status intToStatus(int status)
{
  return (status < 0) ? Status::Failure : Status::Success;
}

/**
 * @brief Check status and print to standard error
 * @param status The status of the operation
 * @param operation The operation name that will be printed
 */
static inline void checkStatus(Status status, const std::string& operation)
{
  if (status != Status::Success) {
    std::cerr << operation << " failed" << std::endl;
  }
}

}  // namespace AQNWB
