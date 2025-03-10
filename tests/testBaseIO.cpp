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

TEST_CASE("Test findTypes functionality", "[BaseIO]")
{
  std::string filename = getTestFilePath("test_findTypes.h5");
  HDF5::HDF5IO io(filename);
  io.open(FileMode::Overwrite);

  SECTION("Empty file returns empty result")
  {
    auto result =
        io.findTypes("/", {"core::NWBFile"}, SearchMode::STOP_ON_TYPE);
    REQUIRE(result.empty());
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
  io.close();
}
