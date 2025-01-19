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
  io.close();
}