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
    // Safely create out-of-range enum values for testing using a union to avoid
    // -Wconversion
    auto makeInvalidStorageObjectType = [](int val) -> Types::StorageObjectType
    {
      union
      {
        int i;
        Types::StorageObjectType e;
      } u;
      u.i = val;
      return u.e;
    };
    REQUIRE(Types::storageObjectTypeToString(makeInvalidStorageObjectType(99))
            == std::string("Unknown"));
    REQUIRE(Types::storageObjectTypeToString(makeInvalidStorageObjectType(-99))
            == std::string("Unknown"));
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
