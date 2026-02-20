#include <catch2/catch_all.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "testUtils.hpp"

using namespace AQNWB::IO;

TEST_CASE("Test ArrayDataSetConfig", "[BaseIO]")
{
  SECTION("Test constructor")
  {
    ArrayDataSetConfig config(BaseDataType::I32, SizeArray {10}, SizeArray {5});
    REQUIRE(config.getType() == BaseDataType::I32);
    REQUIRE(config.getShape() == SizeArray {10});
    REQUIRE(config.getChunking() == SizeArray {5});
  }
}

TEST_CASE("BaseDataType equality operator", "[BaseIO]")
{
  SECTION("Same type and size")
  {
    BaseDataType type1(BaseDataType::T_I32, 4);
    BaseDataType type2(BaseDataType::T_I32, 4);
    REQUIRE(type1 == type2);
  }

  SECTION("Different type")
  {
    BaseDataType type1(BaseDataType::T_I32, 4);
    BaseDataType type2(BaseDataType::T_F32, 4);
    REQUIRE(!(type1 == type2));
  }

  SECTION("Different size")
  {
    BaseDataType type1(BaseDataType::T_I32, 4);
    BaseDataType type2(BaseDataType::T_I32, 8);
    REQUIRE(!(type1 == type2));
  }

  SECTION("Different type and size")
  {
    BaseDataType type1(BaseDataType::T_I32, 4);
    BaseDataType type2(BaseDataType::T_F32, 8);
    REQUIRE(!(type1 == type2));
  }
}

TEST_CASE("BaseDataType fromTypeId", "[BaseIO]")
{
  SECTION("Supported data types")
  {
    REQUIRE(BaseDataType::fromTypeId(typeid(uint8_t)).type
            == BaseDataType::T_U8);
    REQUIRE(BaseDataType::fromTypeId(typeid(uint16_t)).type
            == BaseDataType::T_U16);
    REQUIRE(BaseDataType::fromTypeId(typeid(uint32_t)).type
            == BaseDataType::T_U32);
    REQUIRE(BaseDataType::fromTypeId(typeid(uint64_t)).type
            == BaseDataType::T_U64);
    REQUIRE(BaseDataType::fromTypeId(typeid(int8_t)).type
            == BaseDataType::T_I8);
    REQUIRE(BaseDataType::fromTypeId(typeid(int16_t)).type
            == BaseDataType::T_I16);
    REQUIRE(BaseDataType::fromTypeId(typeid(int32_t)).type
            == BaseDataType::T_I32);
    REQUIRE(BaseDataType::fromTypeId(typeid(int64_t)).type
            == BaseDataType::T_I64);
    REQUIRE(BaseDataType::fromTypeId(typeid(float)).type
            == BaseDataType::T_F32);
    REQUIRE(BaseDataType::fromTypeId(typeid(double)).type
            == BaseDataType::T_F64);
  }

  SECTION("Unsupported data type")
  {
    REQUIRE_THROWS_AS(BaseDataType::fromTypeId(typeid(std::string)),
                      std::runtime_error);
  }
}

TEST_CASE("Test findTypes and getFullTypeName", "[BaseIO]")
{
  std::string filename = getTestFilePath("test_findTypes.h5");
  HDF5::HDF5IO io(filename);
  io.open(FileMode::Overwrite);

  SECTION("Empty file returns empty result")
  {
    auto result =
        io.findTypes("/", {"core::NWBFile"}, SearchMode::STOP_ON_TYPE);
    REQUIRE(result.empty());

    // Confirm that getFullTypeName throws an exception
    REQUIRE_THROWS(io.getFullTypeName("/"));
  }

  SECTION("Single type at root")
  {
    // Create root group with type attributes
    io.createGroup("/");
    io.createAttribute("core", "/", "namespace");
    io.createAttribute("NWBFile", "/", "neurodata_type");

    auto result =
        io.findTypes("/", {"core::NWBFile"}, SearchMode::STOP_ON_TYPE);
    REQUIRE(result.size() == 1);
    REQUIRE(result["/"] == "core::NWBFile");

    // test getFullTypeName
    std::string fullTypeName = io.getFullTypeName("/");
    REQUIRE(fullTypeName == "core::NWBFile");
  }

  SECTION("Search for dataset type")
  {
    // Create root group with type attributes
    io.createGroup("/");
    IO::ArrayDataSetConfig config(
        BaseDataType::I32, SizeArray {0}, SizeArray {1});
    io.createArrayDataSet(config, "/dataset1");
    io.createAttribute("hdmf-common", "/dataset1", "namespace");
    io.createAttribute("VectorData", "/dataset1", "neurodata_type");

    auto result = io.findTypes(
        "/", {"hdmf-common::VectorData"}, SearchMode::STOP_ON_TYPE);
    REQUIRE(result.size() == 1);
    REQUIRE(result["/dataset1"] == "hdmf-common::VectorData");

    // The getFullTypeName for non-root paths
    REQUIRE(io.getFullTypeName("/dataset1") == "hdmf-common::VectorData");
  }

  SECTION("Multiple nested types with STOP_ON_TYPE")
  {
    // Setup hierarchy
    io.createGroup("/");
    io.createAttribute("core", "/", "namespace");
    io.createAttribute("NWBFile", "/", "neurodata_type");

    io.createGroup("/testProcessingModule");
    io.createAttribute("core", "/testProcessingModule", "namespace");
    io.createAttribute(
        "ProcessingModule", "/testProcessingModule", "neurodata_type");

    auto result = io.findTypes("/",
                               {"core::NWBFile", "core::ProcessingModule"},
                               SearchMode::STOP_ON_TYPE);
    REQUIRE(result.size() == 1);
    REQUIRE(result["/"] == "core::NWBFile");
  }

  SECTION("Multiple nested types with CONTINUE_ON_TYPE")
  {
    // Setup hierarchy
    io.createGroup("/");
    io.createAttribute("core", "/", "namespace");
    io.createAttribute("NWBFile", "/", "neurodata_type");

    io.createGroup("/testProcessingModule");
    io.createAttribute("core", "/testProcessingModule", "namespace");
    io.createAttribute(
        "ProcessingModule", "/testProcessingModule", "neurodata_type");

    auto result = io.findTypes("/",
                               {"core::NWBFile", "core::ProcessingModule"},
                               SearchMode::CONTINUE_ON_TYPE);
    REQUIRE(result.size() == 2);
    REQUIRE(result["/"] == "core::NWBFile");
    REQUIRE(result["/testProcessingModule"] == "core::ProcessingModule");
  }

  SECTION("Non-matching types are not included")
  {
    // Setup hierarchy
    io.createGroup("/");
    io.createAttribute("core", "/", "namespace");
    io.createAttribute("NWBFile", "/", "neurodata_type");

    io.createGroup("/testProcessingModule");
    io.createAttribute("core", "/testProcessingModule", "namespace");
    io.createAttribute(
        "ProcessingModule", "/testProcessingModule", "neurodata_type");

    auto result =
        io.findTypes("/", {"core::Device"}, SearchMode::CONTINUE_ON_TYPE);
    REQUIRE(result.empty());
  }

  SECTION("Missing attributes are handled gracefully")
  {
    // Create group with missing neurodata_type attribute
    io.createGroup("/");
    io.createAttribute("core", "/", "namespace");

    io.createGroup("/testProcessingModule");
    io.createAttribute("core", "/testProcessingModule", "namespace");
    io.createAttribute(
        "ProcessingModule", "/testProcessingModule", "neurodata_type");

    auto result = io.findTypes(
        "/", {"core::ProcessingModule"}, SearchMode::CONTINUE_ON_TYPE);
    REQUIRE(result.size() == 1);
    REQUIRE(result["/testProcessingModule"] == "core::ProcessingModule");
  }

  SECTION("Search for any type with empty set")
  {
    // Setup hierarchy
    io.createGroup("/");
    io.createAttribute("core", "/", "namespace");
    io.createAttribute("NWBFile", "/", "neurodata_type");

    io.createGroup("/testProcessingModule");
    io.createAttribute("core", "/testProcessingModule", "namespace");
    io.createAttribute(
        "ProcessingModule", "/testProcessingModule", "neurodata_type");

    auto result = io.findTypes("/", {}, SearchMode::CONTINUE_ON_TYPE);
    REQUIRE(result.size() == 2);
    REQUIRE(result["/"] == "core::NWBFile");
    REQUIRE(result["/testProcessingModule"] == "core::ProcessingModule");

    result = io.findTypes("/", {}, SearchMode::STOP_ON_TYPE);
    REQUIRE(result.size() == 1);
    REQUIRE(result["/"] == "core::NWBFile");

    result = io.findTypes("/", {}, SearchMode::STOP_ON_TYPE, true);
    REQUIRE(result.size() == 1);
    REQUIRE(result["/testProcessingModule"] == "core::ProcessingModule");
  }

  SECTION("Test with exclude_starting_path=true")
  {
    // Setup hierarchy
    io.createGroup("/");
    io.createAttribute("core", "/", "namespace");
    io.createAttribute("NWBFile", "/", "neurodata_type");

    io.createGroup("/testProcessingModule");
    io.createAttribute("core", "/testProcessingModule", "namespace");
    io.createAttribute(
        "ProcessingModule", "/testProcessingModule", "neurodata_type");

    io.createGroup("/testProcessingModule/testTimeSeries");
    io.createAttribute(
        "core", "/testProcessingModule/testTimeSeries", "namespace");
    io.createAttribute(
        "TimeSeries", "/testProcessingModule/testTimeSeries", "neurodata_type");

    // If we exclude the starting path, then we should not find any NWBFile
    // types
    auto result =
        io.findTypes("/", {"core::NWBFile"}, SearchMode::STOP_ON_TYPE, true);
    REQUIRE(result.size() == 0);

    // If we exclude the starting path but search for any type, then we should
    // still find the ProcessingModule type as the next typed object below the
    // root
    result = io.findTypes("/", {}, SearchMode::STOP_ON_TYPE, true);
    REQUIRE(result.size() == 1);
    REQUIRE(result["/testProcessingModule"] == "core::ProcessingModule");

    // If we exclude the starting path, then we should still find the
    // ProcessingModule type
    result = io.findTypes(
        "/", {"core::ProcessingModule"}, SearchMode::STOP_ON_TYPE, true);
    REQUIRE(result.size() == 1);
    REQUIRE(result["/testProcessingModule"] == "core::ProcessingModule");

    // If we exclude the starting path, then we should still find the
    // ProcessingModule and the TimeSeries type if search for any type and
    // ContineOnType is used
    result = io.findTypes("/", {}, SearchMode::CONTINUE_ON_TYPE, true);
    REQUIRE(result.size() == 2);
    REQUIRE(result["/testProcessingModule"] == "core::ProcessingModule");
    REQUIRE(result["/testProcessingModule/testTimeSeries"]
            == "core::TimeSeries");

    // If we include the starting path and using ContineOnType, then we should
    // find all types
    result = io.findTypes("/", {}, SearchMode::CONTINUE_ON_TYPE, false);
    REQUIRE(result.size() == 3);
    REQUIRE(result["/"] == "core::NWBFile");
    REQUIRE(result["/testProcessingModule"] == "core::ProcessingModule");
    REQUIRE(result["/testProcessingModule/testTimeSeries"]
            == "core::TimeSeries");
  }
  io.close();
}

TEST_CASE("Test BaseArrayDataSetConfig polymorphism", "[BaseIO]")
{
  SECTION("ArrayDataSetConfig inherits from BaseArrayDataSetConfig")
  {
    ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {100}, SizeArray {10});

    // Can be used as base class pointer
    BaseArrayDataSetConfig* basePtr = &config;
    REQUIRE(basePtr != nullptr);
    REQUIRE(basePtr->isLink() == false);
  }

  SECTION("LinkArrayDataSetConfig inherits from BaseArrayDataSetConfig")
  {
    LinkArrayDataSetConfig linkConfig("/path/to/target");

    // Can be used as base class pointer
    BaseArrayDataSetConfig* basePtr = &linkConfig;
    REQUIRE(basePtr != nullptr);
    REQUIRE(basePtr->isLink() == true);
  }

  SECTION("ArrayDataSetConfig and LinkArrayDataSetConfig are polymorphic")
  {
    ArrayDataSetConfig arrayConfig(
        BaseDataType::I32, SizeArray {50}, SizeArray {5});
    LinkArrayDataSetConfig linkConfig("/some/target/path");

    BaseArrayDataSetConfig* ptr1 = &arrayConfig;
    BaseArrayDataSetConfig* ptr2 = &linkConfig;

    REQUIRE(ptr1->isLink() == false);
    REQUIRE(ptr2->isLink() == true);
  }
}

TEST_CASE("Test LinkArrayDataSetConfig", "[BaseIO]")
{
  SECTION("Test constructor with target path")
  {
    std::string targetPath = "/acquisition/raw_data/data";
    LinkArrayDataSetConfig config(targetPath);

    REQUIRE(config.getTargetPath() == targetPath);
    REQUIRE(config.isLink() == true);
  }

  SECTION("Test with different target paths")
  {
    LinkArrayDataSetConfig config1("/path/to/dataset1");
    LinkArrayDataSetConfig config2("/path/to/dataset2");

    REQUIRE(config1.getTargetPath() == "/path/to/dataset1");
    REQUIRE(config2.getTargetPath() == "/path/to/dataset2");
    REQUIRE(config1.isLink() == true);
    REQUIRE(config2.isLink() == true);
  }

  SECTION("Test with empty path")
  {
    LinkArrayDataSetConfig config("");
    REQUIRE(config.getTargetPath() == "");
    REQUIRE(config.isLink() == true);
  }

  SECTION("Test with absolute and relative paths")
  {
    LinkArrayDataSetConfig absoluteConfig("/absolute/path");
    LinkArrayDataSetConfig relativeConfig("relative/path");

    REQUIRE(absoluteConfig.getTargetPath() == "/absolute/path");
    REQUIRE(relativeConfig.getTargetPath() == "relative/path");
  }
}

TEST_CASE("Test ArrayDataSetConfig does not report as link", "[BaseIO]")
{
  SECTION("Regular ArrayDataSetConfig is not a link")
  {
    ArrayDataSetConfig config(
        BaseDataType::F64, SizeArray {1000}, SizeArray {100});
    REQUIRE(config.isLink() == false);
  }

  SECTION("Multiple ArrayDataSetConfig instances are not links")
  {
    ArrayDataSetConfig config1(BaseDataType::I8, SizeArray {10}, SizeArray {5});
    ArrayDataSetConfig config2(
        BaseDataType::U16, SizeArray {20}, SizeArray {10});
    ArrayDataSetConfig config3(
        BaseDataType::F32, SizeArray {30}, SizeArray {15});

    REQUIRE(config1.isLink() == false);
    REQUIRE(config2.isLink() == false);
    REQUIRE(config3.isLink() == false);
  }
}

TEST_CASE("Test storageObjectTypeToString", "[BaseIO]")
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
