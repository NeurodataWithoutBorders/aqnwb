#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

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
  // Get current time
  auto currentTime =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm utcTime = *std::gmtime(&currentTime);

  // Get the timezone offset
  auto localTime = std::localtime(&currentTime);
  auto utcOffset = localTime->tm_hour - utcTime.tm_hour;
  auto utcOffsetMinutes =
      (utcOffset < 0 ? -1 : 1) * (localTime->tm_min - utcTime.tm_min);

  // Adjust the UTC time to the local time
  utcTime.tm_hour += utcOffset;
  if (utcTime.tm_hour < 0) {
    utcTime.tm_hour += 24;
    utcTime.tm_mday -= 1;
  }

  // Format the date and time in ISO 8601 format with the UTC offset
  std::ostringstream oss;
  oss << std::put_time(&utcTime, "%FT%T");

  // Add the timezone offset to the date and time string
  oss << (utcOffset < 0 ? "-" : "+") << std::setw(2) << std::setfill('0')
      << std::abs(utcOffset) << ":" << std::setw(2) << std::setfill('0')
      << std::abs(utcOffsetMinutes);

  return oss.str();
}
}  // namespace AQNWB
