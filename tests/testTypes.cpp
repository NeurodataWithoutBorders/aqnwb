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
}

TEST_CASE("Test Status enum operators", "[Types]")
{
  SECTION("Test && operator")
  {
    // cppcheck-suppress-begin duplicateExpression
    // cppcheck-suppress-begin compareBoolExpressionWithInt
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
    // cppcheck-suppress-end duplicateExpression
    // cppcheck-suppress-end compareBoolExpressionWithInt
  }

  SECTION("Test || operator")
  {
    // cppcheck-suppress-begin duplicateExpression
    // cppcheck-suppress-begin compareBoolExpressionWithInt
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
    // cppcheck-suppress-end duplicateExpression
    // cppcheck-suppress-end compareBoolExpressionWithInt
  }

  SECTION("Test chaining of operators")
  {
    // cppcheck-suppress-begin duplicateExpression
    // cppcheck-suppress-begin compareBoolExpressionWithInt
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
    // cppcheck-suppress-end duplicateExpression
    // cppcheck-suppress-end compareBoolExpressionWithInt
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
    // cppcheck-suppress-begin knownConditionTrueFalse
    REQUIRE(Types::SizeTypeNotSet
            == (std::numeric_limits<Types::SizeType>::max)());
    // cppcheck-suppress-end knownConditionTrueFalse
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

TEST_CASE("Test CellValue", "[Types]")
{
  SECTION("Default constructor")
  {
    Types::CellValue cell;
    REQUIRE(cell.holds_alternative<std::monostate>());
    REQUIRE(cell.toString() == "null");
  }

  SECTION("Scalar constructors and get")
  {
    Types::CellValue intCell(42);
    REQUIRE(intCell.holds_alternative<int>());
    REQUIRE(intCell.get<int>() == 42);
    REQUIRE(intCell.toString() == "42");

    Types::CellValue doubleCell(3.14);
    REQUIRE(doubleCell.holds_alternative<double>());
    REQUIRE_THAT(doubleCell.get<double>(),
                 Catch::Matchers::WithinRel(3.14, 1e-5));
    // toString for double might have trailing zeros, so we just check it
    // doesn't throw
    REQUIRE_NOTHROW(doubleCell.toString());

    Types::CellValue stringCell("hello");
    REQUIRE(stringCell.holds_alternative<std::string>());
    REQUIRE(stringCell.get<std::string>() == "hello");
    REQUIRE(stringCell.toString() == "hello");

    std::string str = "world";
    Types::CellValue stdStringCell(str);
    REQUIRE(stdStringCell.holds_alternative<std::string>());
    REQUIRE(stdStringCell.get<std::string>() == "world");
    REQUIRE(stdStringCell.toString() == "world");
  }

  SECTION("Vector constructors and get")
  {
    std::vector<int> intVec = {1, 2, 3};
    Types::CellValue intVecCell(intVec);
    REQUIRE(intVecCell.holds_alternative<std::vector<int>>());
    REQUIRE(intVecCell.get<std::vector<int>>() == intVec);
    REQUIRE(intVecCell.toString() == "[1, 2, 3]");

    std::vector<std::string> strVec = {"a", "b"};
    Types::CellValue strVecCell(strVec);
    REQUIRE(strVecCell.holds_alternative<std::vector<std::string>>());
    REQUIRE(strVecCell.get<std::vector<std::string>>() == strVec);
    REQUIRE(strVecCell.toString() == "[a, b]");
  }

  SECTION("Implicit conversion operator")
  {
    Types::CellValue intCell(42);
    int val = intCell;
    REQUIRE(val == 42);

    Types::CellValue strCell("test");
    std::string strVal = strCell;
    REQUIRE(strVal == "test");

    std::vector<int> vec = {1, 2};
    Types::CellValue vecCell(vec);
    std::vector<int> vecVal = vecCell;
    REQUIRE(vecVal == vec);
  }

  SECTION("holds_alternative with wrong type")
  {
    Types::CellValue intCell(42);
    REQUIRE_FALSE(intCell.holds_alternative<double>());
    REQUIRE_FALSE(intCell.holds_alternative<std::string>());
    REQUIRE_FALSE(intCell.holds_alternative<std::vector<int>>());

    Types::CellValue vecCell(std::vector<int> {1, 2});
    REQUIRE_FALSE(vecCell.holds_alternative<int>());
    REQUIRE_FALSE(vecCell.holds_alternative<std::vector<double>>());
  }

  SECTION("get with wrong type throws")
  {
    Types::CellValue intCell(42);
    REQUIRE_THROWS_AS(intCell.get<double>(), std::bad_variant_access);
    REQUIRE_THROWS_AS(intCell.get<std::vector<int>>(), std::bad_variant_access);
  }
}
