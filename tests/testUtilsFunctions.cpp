#include <regex>

#include "testUtils.hpp"

#include "Utils.hpp"

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
