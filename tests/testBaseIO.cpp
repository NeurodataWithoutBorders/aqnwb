#include <catch2/catch_all.hpp>

#include "io/BaseIO.hpp"
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
    const BaseArrayDataSetConfig* basePtr = &config;
    // cppcheck-suppress knownConditionTrueFalse
    REQUIRE(basePtr != nullptr);
    REQUIRE(basePtr->isLink() == false);
  }

  SECTION("LinkArrayDataSetConfig inherits from BaseArrayDataSetConfig")
  {
    LinkArrayDataSetConfig linkConfig("/path/to/target");

    // Can be used as base class pointer
    const BaseArrayDataSetConfig* basePtr = &linkConfig;
    // cppcheck-suppress knownConditionTrueFalse
    REQUIRE(basePtr != nullptr);
    REQUIRE(basePtr->isLink() == true);
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

  SECTION("targetExists returns true if target exists and false if not")
  {
    std::string filename =
        getTestFilePath("test_link_target_exists_and_missing.h5");
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    // Create a dataset at /existing/dataset
    IO::ArrayDataSetConfig config(
        BaseDataType::I32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/existing_dataset");
    // Test for existing target
    LinkArrayDataSetConfig linkConfigExists("/existing_dataset");
    REQUIRE(linkConfigExists.targetExists(io) == true);
    // Test for missing target
    LinkArrayDataSetConfig linkConfigMissing("/missing_dataset");
    REQUIRE(linkConfigMissing.targetExists(io) == false);
    io.close();
  }
}

TEST_CASE("Test LinkArrayDataSetConfig::validateTarget", "[BaseIO]")
{
  std::string filename = getTestFilePath("test_validate_target.h5");

  SECTION("validateTarget succeeds for a valid target with no restrictions")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    IO::ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/valid_dataset");

    LinkArrayDataSetConfig linkConfig("/valid_dataset");
    REQUIRE(linkConfig.validateTarget(io) == Status::Success);
    io.close();
  }

  SECTION("validateTarget fails when the target does not exist")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);

    LinkArrayDataSetConfig linkConfig("/nonexistent_dataset");
    REQUIRE(linkConfig.validateTarget(io) == Status::Failure);
    io.close();
  }

  SECTION("validateTarget succeeds when data type matches allowed types")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    IO::ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/float_dataset");

    LinkArrayDataSetConfig linkConfig("/float_dataset");
    REQUIRE(
        linkConfig.validateTarget(io, {BaseDataType::F32, BaseDataType::F64})
        == Status::Success);
    io.close();
  }

  SECTION("validateTarget fails when data type is not in allowed types")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    IO::ArrayDataSetConfig config(
        BaseDataType::I32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/int_dataset");

    LinkArrayDataSetConfig linkConfig("/int_dataset");
    REQUIRE(
        linkConfig.validateTarget(io, {BaseDataType::F32, BaseDataType::F64})
        == Status::Failure);
    io.close();
  }

  SECTION("validateTarget succeeds when dimensionality matches")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    IO::ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/1d_dataset");

    LinkArrayDataSetConfig linkConfig("/1d_dataset");
    REQUIRE(linkConfig.validateTarget(io, {}, {1, 2}) == Status::Success);
    io.close();
  }

  SECTION("validateTarget fails when dimensionality is not in allowed list")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    IO::ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/1d_dataset");

    LinkArrayDataSetConfig linkConfig("/1d_dataset");
    REQUIRE(linkConfig.validateTarget(io, {}, {2, 3}) == Status::Failure);
    io.close();
  }

  SECTION("validateTarget succeeds when all required attributes are present")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    IO::ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/dataset_with_attrs");
    io.createAttribute("volts", "/dataset_with_attrs", "unit");
    io.createAttribute("no comment", "/dataset_with_attrs", "description");

    LinkArrayDataSetConfig linkConfig("/dataset_with_attrs");
    REQUIRE(linkConfig.validateTarget(io, {}, {}, {"unit", "description"})
            == Status::Success);
    io.close();
  }

  SECTION("validateTarget fails when a required attribute is missing")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    IO::ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/dataset_missing_attr");
    io.createAttribute("volts", "/dataset_missing_attr", "unit");

    LinkArrayDataSetConfig linkConfig("/dataset_missing_attr");
    REQUIRE(linkConfig.validateTarget(io, {}, {}, {"unit", "missing_attribute"})
            == Status::Failure);
    io.close();
  }

  SECTION("validateTarget checks all constraints together")
  {
    HDF5::HDF5IO io(filename);
    io.open(FileMode::Overwrite);
    IO::ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {10}, SizeArray {5});
    io.createArrayDataSet(config, "/complete_dataset");
    io.createAttribute("volts", "/complete_dataset", "unit");

    LinkArrayDataSetConfig linkConfig("/complete_dataset");
    // All constraints satisfied
    REQUIRE(linkConfig.validateTarget(io, {BaseDataType::F32}, {1}, {"unit"})
            == Status::Success);
    // Type mismatch
    REQUIRE(linkConfig.validateTarget(io, {BaseDataType::I32}, {1}, {"unit"})
            == Status::Failure);
    // Dimensionality mismatch
    REQUIRE(linkConfig.validateTarget(io, {BaseDataType::F32}, {2}, {"unit"})
            == Status::Failure);
    // Missing attribute
    REQUIRE(linkConfig.validateTarget(
                io, {BaseDataType::F32}, {1}, {"unit", "missing"})
            == Status::Failure);
    io.close();
  }
}
