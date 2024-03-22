#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

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

  // Convert to tm struct to extract date and time components
  std::tm utcTime = *std::gmtime(&currentTime);

  // Format the date and time in ISO 8601 format with the UTC offset
  std::ostringstream oss;
  oss << std::put_time(&utcTime, "%FT%T%z");

  return oss.str();
}
