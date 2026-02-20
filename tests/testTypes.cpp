#include <catch2/catch_all.hpp>

#include "Types.hpp"

using namespace AQNWB;

TEST_CASE("Test storageObjectTypeToString", "[Types]")
{
  SECTION("All enum values have proper string mappings")
  {
    // Test each defined enum value
    REQUIRE(Types::storageObjectTypeToString(Types::StorageObjectType::Group)
            == "Group");
    REQUIRE(Types::storageObjectTypeToString(Types::StorageObjectType::Dataset)
            == "Dataset");
    REQUIRE(
        Types::storageObjectTypeToString(Types::StorageObjectType::Attribute)
        == "Attribute");
    REQUIRE(
        Types::storageObjectTypeToString(Types::StorageObjectType::Undefined)
        == "Undefined");
  }

  SECTION("Unknown values return 'Unknown'")
  {
    // Cast invalid integer to StorageObjectType to test default case
    // This tests that the function handles unexpected values gracefully
    REQUIRE(Types::storageObjectTypeToString(
                static_cast<Types::StorageObjectType>(99))
            == "Unknown");
    REQUIRE(Types::storageObjectTypeToString(
                static_cast<Types::StorageObjectType>(-99))
            == "Unknown");
  }
}
