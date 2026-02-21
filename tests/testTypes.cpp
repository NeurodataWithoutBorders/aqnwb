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
    // Suppress conversion warning for intentionally invalid enum values
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    REQUIRE(Types::storageObjectTypeToString(
                static_cast<Types::StorageObjectType>(99))
            == "Unknown");
    REQUIRE(Types::storageObjectTypeToString(
                static_cast<Types::StorageObjectType>(-99))
            == "Unknown");
#pragma GCC diagnostic pop
  }
}

TEST_CASE("Test Status enum operators", "[Types]")
{
  SECTION("Test && operator")
  {
    // Success && Success = Success
    REQUIRE((Types::Status::Success && Types::Status::Success)
            == Types::Status::Success);

    // Success && Failure = Failure
    REQUIRE((Types::Status::Success && Types::Status::Failure)
            == Types::Status::Failure);

    // Failure && Success = Failure
    REQUIRE((Types::Status::Failure && Types::Status::Success)
            == Types::Status::Failure);

    // Failure && Failure = Failure
    REQUIRE((Types::Status::Failure && Types::Status::Failure)
            == Types::Status::Failure);
  }

  SECTION("Test || operator")
  {
    // Success || Success = Success
    REQUIRE((Types::Status::Success || Types::Status::Success)
            == Types::Status::Success);

    // Success || Failure = Success
    REQUIRE((Types::Status::Success || Types::Status::Failure)
            == Types::Status::Success);

    // Failure || Success = Success
    REQUIRE((Types::Status::Failure || Types::Status::Success)
            == Types::Status::Success);

    // Failure || Failure = Failure
    REQUIRE((Types::Status::Failure || Types::Status::Failure)
            == Types::Status::Failure);
  }

  SECTION("Test chaining of operators")
  {
    // Test that multiple && operations work correctly
    REQUIRE((Types::Status::Success && Types::Status::Success
             && Types::Status::Success)
            == Types::Status::Success);
    REQUIRE((Types::Status::Success && Types::Status::Success
             && Types::Status::Failure)
            == Types::Status::Failure);

    // Test that multiple || operations work correctly
    REQUIRE((Types::Status::Failure || Types::Status::Failure
             || Types::Status::Success)
            == Types::Status::Success);
    REQUIRE((Types::Status::Failure || Types::Status::Failure
             || Types::Status::Failure)
            == Types::Status::Failure);
  }
}

TEST_CASE("Test IsDataStorageObjectType template", "[Types]")
{
  SECTION("Dataset is a data storage object type")
  {
    REQUIRE(
        Types::IsDataStorageObjectType<Types::StorageObjectType::Dataset>::value
        == true);
  }

  SECTION("Attribute is a data storage object type")
  {
    REQUIRE(Types::IsDataStorageObjectType<
                Types::StorageObjectType::Attribute>::value
            == true);
  }

  SECTION("Group is not a data storage object type")
  {
    REQUIRE(
        Types::IsDataStorageObjectType<Types::StorageObjectType::Group>::value
        == false);
  }

  SECTION("Undefined is not a data storage object type")
  {
    REQUIRE(Types::IsDataStorageObjectType<
                Types::StorageObjectType::Undefined>::value
            == false);
  }
}

TEST_CASE("Test SizeTypeNotSet constant", "[Types]")
{
  SECTION("SizeTypeNotSet equals max value of SizeType")
  {
    // Verify that SizeTypeNotSet is set to the maximum value
    REQUIRE(Types::SizeTypeNotSet
            == (std::numeric_limits<Types::SizeType>::max)());
  }

  SECTION("SizeTypeNotSet is distinct from typical indices")
  {
    // Verify that SizeTypeNotSet is different from common index values
    REQUIRE(Types::SizeTypeNotSet != 0);
    REQUIRE(Types::SizeTypeNotSet != 1);
    REQUIRE(Types::SizeTypeNotSet != 100);
    REQUIRE(Types::SizeTypeNotSet != 1000);
  }

  SECTION("SizeTypeNotSet can be used to check if index is set")
  {
    // Example usage pattern: check if an index has been set
    Types::SizeType index = Types::SizeTypeNotSet;
    REQUIRE(index == Types::SizeTypeNotSet);

    // After setting to a valid value
    index = 42;
    REQUIRE(index != Types::SizeTypeNotSet);
  }
}
