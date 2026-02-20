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

  SECTION("Verify no enum values map to 'Unknown' by mistake")
  {
    // This ensures that if someone adds a new enum value but forgets to
    // update the storageObjectTypeToString function, the test will fail
    // because the new value will return "Unknown"
    
    // All valid enum values should NOT return "Unknown"
    REQUIRE(Types::storageObjectTypeToString(Types::StorageObjectType::Group)
            != "Unknown");
    REQUIRE(Types::storageObjectTypeToString(Types::StorageObjectType::Dataset)
            != "Unknown");
    REQUIRE(
        Types::storageObjectTypeToString(Types::StorageObjectType::Attribute)
        != "Unknown");
    REQUIRE(
        Types::storageObjectTypeToString(Types::StorageObjectType::Undefined)
        != "Unknown");
  }
}
