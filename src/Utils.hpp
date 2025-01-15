#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include <boost/date_time.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "boost/date_time/c_local_time_adjustor.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"

namespace AQNWB
{
/**
 * @brief Generates a UUID (Universally Unique Identifier) as a string.
 * @return The generated UUID as a string.
 */
static inline std::string generateUuid()
{
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuidStr = boost::uuids::to_string(uuid);

  return uuidStr;
}

/**
 * @brief Get the current time in ISO 8601 format with the UTC offset.
 * @return The current time as a string in ISO 8601 format.
 */
static inline std::string getCurrentTime()
{
  // Set up boost time zone adjustment and time facet
  using local_adj =
      boost::date_time::c_local_adjustor<boost::posix_time::ptime>;
  boost::posix_time::time_facet* f = new boost::posix_time::time_facet();
  f->time_duration_format("%+%H:%M");

  // get local time, utc time, and offset
  auto now = boost::posix_time::microsec_clock::universal_time();
  auto utc_now = local_adj::utc_to_local(now);
  boost::posix_time::time_duration td = utc_now - now;

  // Format the date and time in ISO 8601 format with the UTC offset
  std::ostringstream oss_offset;
  oss_offset.imbue(std::locale(oss_offset.getloc(), f));
  oss_offset << td;

  std::string currentTime = to_iso_extended_string(utc_now);
  currentTime += oss_offset.str();

  return currentTime;
}

/**
 * @brief Factory method to create an IO object.
 * @return A pointer to a BaseIO object
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
    intValue = boost::endian::native_to_little(intValue);
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

}  // namespace AQNWB
