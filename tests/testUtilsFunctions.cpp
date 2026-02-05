#include <ctime>
#include <regex>

#include "testUtils.hpp"

#include "Utils.hpp"

TEST_CASE("isISO8601Date function tests", "[utils]")
{
  SECTION("Valid ISO 8601 date strings")
  {
    REQUIRE(AQNWB::isISO8601Date("2018-09-28T14:43:54.123+02:00"));
    REQUIRE(AQNWB::isISO8601Date("2025-01-19T00:40:03.214144-08:00"));
    REQUIRE(AQNWB::isISO8601Date("2021-12-31T23:59:59.999999+00:00"));
    REQUIRE(AQNWB::isISO8601Date("2000-01-01T00:00:00.0+01:00"));
    REQUIRE(AQNWB::isISO8601Date(
        "2018-09-28T14:43:54.12345+02:00"));  // Allow for too many fractional
                                              // seconds
  }

  SECTION("Invalid ISO 8601 date strings")
  {
    REQUIRE_FALSE(AQNWB::isISO8601Date(
        "2018-09-28 14:43:54.123+02:00"));  // Space instead of 'T'
    REQUIRE_FALSE(AQNWB::isISO8601Date(
        "2018-09-28T14:43:54+02:00"));  // Missing fractional seconds
    REQUIRE_FALSE(AQNWB::isISO8601Date(
        "2018-09-28T14:43:54.123+0200"));  // Missing colon in timezone
    REQUIRE_FALSE(AQNWB::isISO8601Date(
        "2018-09-28T14:43:54.123Z"));  // Missing timezone offset
    REQUIRE_FALSE(AQNWB::isISO8601Date(
        "2018-09-28T14:43:54.123-0800"));  // Incorrect timezone format
    REQUIRE_FALSE(
        AQNWB::isISO8601Date("2018-09-28T14:43:54.123"));  // Missing timezone
    REQUIRE_FALSE(AQNWB::isISO8601Date("Random text 1213"));
  }
}

TEST_CASE("Test UUID generation", "[utils]")
{
  // Test that generated UUIDs are valid
  std::string uuid = AQNWB::generateUuid();

  // UUID format regex (8-4-4-4-12 hex digits)
  std::regex uuidRegex(
      "^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$");

  SECTION("UUID format is valid")
  {
    REQUIRE(std::regex_match(uuid, uuidRegex));
  }

  SECTION("UUIDs are unique")
  {
    std::string uuid2 = AQNWB::generateUuid();
    REQUIRE(uuid != uuid2);
  }
}

TEST_CASE("Test current time format", "[utils]")
{
  std::string time = AQNWB::getCurrentTime();

  // ISO 8601 format regex with timezone offset
  std::regex timeRegex(
      "^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{6}[+-]\\d{2}:\\d{2}$");

  SECTION("Time format is valid ISO 8601")
  {
    REQUIRE(std::regex_match(time, timeRegex));
  }
}

TEST_CASE("Test IO creation", "[utils]")
{
  std::string testFile = getTestFilePath("test_createIO.h5");

  SECTION("Create HDF5 IO")
  {
    REQUIRE_NOTHROW(AQNWB::createIO("HDF5", testFile));
    auto io = AQNWB::createIO("HDF5", testFile);
    REQUIRE(io != nullptr);
  }

  SECTION("Invalid IO type throws")
  {
    REQUIRE_THROWS_AS(AQNWB::createIO("INVALID", testFile),
                      std::invalid_argument);
  }
}

TEST_CASE("Test path merging", "[utils]")
{
  SECTION("Basic path merging")
  {
    REQUIRE(AQNWB::mergePaths("path1", "path2") == "path1/path2");
    REQUIRE(AQNWB::mergePaths("/path1", "path2") == "/path1/path2");
    REQUIRE(AQNWB::mergePaths("path1/", "/path2") == "path1/path2");
    REQUIRE(AQNWB::mergePaths("/path1/", "/path2/") == "/path1/path2");
    REQUIRE(AQNWB::mergePaths("/path1/", "path2/") == "/path1/path2");
  }

  SECTION("Handle empty paths")
  {
    REQUIRE(AQNWB::mergePaths("", "path2") == "path2");
    REQUIRE(AQNWB::mergePaths("path1", "") == "path1");
    REQUIRE(AQNWB::mergePaths("", "") == "");
    REQUIRE(AQNWB::mergePaths("/", "") == "/");
  }

  SECTION("Handle root paths")
  {
    REQUIRE(AQNWB::mergePaths("/", "path2") == "/path2");
    REQUIRE(AQNWB::mergePaths("/", "/path2") == "/path2");
    REQUIRE(AQNWB::mergePaths("/", "/") == "/");
  }

  SECTION("Remove duplicate slashes")
  {
    REQUIRE(AQNWB::mergePaths("path1//", "//path2") == "path1/path2");
    REQUIRE(AQNWB::mergePaths("path1///", "///path2") == "path1/path2");
  }
}

TEST_CASE("Test time conversion functions", "[utils]")
{
  std::time_t now = std::time(nullptr);

  SECTION("to_local_time")
  {
    std::tm local_tm = AQNWB::detail::to_local_time(now);
    // Basic sanity check: year should be reasonable (e.g., > 120 for 2020+)
    REQUIRE(local_tm.tm_year > 120);
    REQUIRE(local_tm.tm_mon >= 0);
    REQUIRE(local_tm.tm_mon <= 11);
    REQUIRE(local_tm.tm_mday >= 1);
    REQUIRE(local_tm.tm_mday <= 31);
  }

  SECTION("to_utc_time")
  {
    std::tm utc_tm = AQNWB::detail::to_utc_time(now);
    // Basic sanity check
    REQUIRE(utc_tm.tm_year > 120);
    REQUIRE(utc_tm.tm_mon >= 0);
    REQUIRE(utc_tm.tm_mon <= 11);
    REQUIRE(utc_tm.tm_mday >= 1);
    REQUIRE(utc_tm.tm_mday <= 31);
  }

  SECTION("get_utc_offset_seconds")
  {
    long offset = AQNWB::detail::get_utc_offset_seconds(now);
    // Offset should be within -12 to +14 hours (in seconds)
    REQUIRE(offset >= -12 * 3600);
    REQUIRE(offset <= 14 * 3600);
  }

  SECTION("format_utc_offset")
  {
    REQUIRE(AQNWB::detail::format_utc_offset(0) == "+00:00");
    REQUIRE(AQNWB::detail::format_utc_offset(3600) == "+01:00");
    REQUIRE(AQNWB::detail::format_utc_offset(-3600) == "-01:00");
    REQUIRE(AQNWB::detail::format_utc_offset(5400) == "+01:30");
    REQUIRE(AQNWB::detail::format_utc_offset(-5400) == "-01:30");
  }
}

TEST_CASE("Test endian conversion", "[utils]")
{
  uint16_t val = 0x1234;
  uint16_t converted = AQNWB::detail::to_little_endian_u16(val);

  // Check system endianness
  uint16_t num = 1;
  bool isLittleEndian = (*reinterpret_cast<char*>(&num) == 1);

  if (isLittleEndian) {
    REQUIRE(converted == 0x1234);
  } else {
    REQUIRE(converted == 0x3412);
  }
}