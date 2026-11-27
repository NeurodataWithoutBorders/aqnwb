#include <memory>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "nwb/RegisterTypes.hpp"
#include "nwb/RegisteredType.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

// Tests for reading files with neurodata_types of optional modules (e.g.,
// optical physiology). This file is always compiled, i.e., also if the
// modules are disabled via the CMake options (e.g., AQNWB_ENABLE_OPHYS=OFF).
TEST_CASE("Read neurodata_types of optional modules", "[modules]")
{
  std::string path = getTestFilePath("testModulesRead.h5");
  {
    // Create a group that is tagged as a TwoPhotonSeries without using the
    // classes of the ophys module
    std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
    io->open();
    REQUIRE(io->createGroup("/series") == Status::Success);
    REQUIRE(io->createCommonNWBAttributes("/series", "core", "TwoPhotonSeries")
            == Status::Success);
    io->close();
  }

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  // Use fallbackToBase=true to read types without a registered class via the
  // generic Container and Data classes
  auto series = NWB::RegisteredType::create("/series", readIO, true);
  REQUIRE(series != nullptr);
#ifdef AQNWB_HAS_OPHYS
  // The ophys module is enabled, i.e., the TwoPhotonSeries class is used
  REQUIRE(series->getFullTypeName() == "core::TwoPhotonSeries");
#else
  // The ophys module is disabled, i.e., the generic Container class is used
  REQUIRE(series->getFullTypeName() == "hdmf-common::Container");
  REQUIRE_FALSE(
      NWB::RegisteredType::getRegistry().count("core::TwoPhotonSeries") > 0);
#endif
  readIO->close();
}

TEST_CASE("registerAllTypes", "[modules]")
{
  NWB::registerAllTypes();
  auto registry = NWB::RegisteredType::getRegistry();
  REQUIRE(registry.count("hdmf-common::Container") == 1);
  REQUIRE(registry.count("core::TimeSeries") == 1);
  REQUIRE(registry.count("core::NWBFile") == 1);
#ifdef AQNWB_HAS_IMAGE
  REQUIRE(registry.count("core::ImageSeries") == 1);
#else
  REQUIRE(registry.count("core::ImageSeries") == 0);
#endif
#ifdef AQNWB_HAS_OPHYS
  // The ophys module requires the image module
  REQUIRE(registry.count("core::ImageSeries") == 1);
  REQUIRE(registry.count("core::TwoPhotonSeries") == 1);
#else
  REQUIRE(registry.count("core::TwoPhotonSeries") == 0);
#endif
#ifdef AQNWB_HAS_OGEN
  REQUIRE(registry.count("core::OptogeneticSeries") == 1);
#endif
  // Registering the types again does not change the registry
  NWB::registerAllTypes();
  REQUIRE(NWB::RegisteredType::getRegistry().size() == registry.size());
}
