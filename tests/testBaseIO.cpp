#include <catch2/catch_all.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "testUtils.hpp"

using namespace AQNWB::IO;

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
    io.createArrayDataSet(
        BaseDataType::I32, SizeArray {0}, SizeArray {1}, "/dataset1");
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