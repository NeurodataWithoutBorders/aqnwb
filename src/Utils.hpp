#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/date_time.hpp>
#include "boost/date_time/c_local_time_adjustor.hpp"

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
  using local_adj = boost::date_time::c_local_adjustor<boost::posix_time::ptime>;
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
}  // namespace AQNWB
