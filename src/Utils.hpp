#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <boost/date_time.hpp>
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
inline std::string generateUuid()
{
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuidStr = boost::uuids::to_string(uuid);

  return uuidStr;
}

/**
 * @brief Get the current time in ISO 8601 format with the UTC offset.
 * @return The current time as a string in ISO 8601 format.
 */
inline std::string getCurrentTime()
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
inline std::shared_ptr<IO::BaseIO> createIO(const std::string& type,
                                            const std::string& filename)
{
  if (type == "HDF5") {
    return std::make_shared<IO::HDF5::HDF5IO>(filename);
  } else {
    throw std::invalid_argument("Invalid IO type");
  }
}

inline std::unique_ptr<int16_t[]> transformToInt16(SizeType numSamples,
                                                   float conversion_factor,
                                                   const float* data)
{
  std::unique_ptr<float[]> scaledData = std::make_unique<float[]>(numSamples);
  std::unique_ptr<int16_t[]> intData = std::make_unique<int16_t[]>(numSamples);

  // copy data and multiply by scaling factor
  double multFactor = 1 / (32767.0f * conversion_factor);
  std::transform(data,
                 data + numSamples,
                 scaledData.get(),
                 [multFactor](float value) { return value * multFactor; });

  // convert float to int16
  std::transform(
      scaledData.get(),
      scaledData.get() + numSamples,
      intData.get(),
      [](float value)
      { return static_cast<int16_t>(std::clamp(value, -32768.0f, 32767.0f)); });

  return intData;
}
}  // namespace AQNWB
