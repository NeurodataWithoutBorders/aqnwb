#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/BaseIO.hpp"
#include "nwb/device/Device.hpp"
#include "testUtils.hpp"

using namespace AQNWB::NWB;

TEST_CASE("Device", "[device]")
{
  SECTION("test Device is registerd as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("core::Device") != registry.end());
  }

  SECTION("test Device constructor")
  {
    // create the device without writing data
    std::string path = "test_path";
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    Device device(path, io);

    REQUIRE(device.getPath() == path);
    REQUIRE(device.getIO() == io);
  }

  SECTION("test Device write/read")
  {
    // Create the device file
    std::string path = "test_path";
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    Device device(path, io);

    std::string description = "Test Device Description";
    std::string manufacturer = "Test Manufacturer";
    device.initialize(description, manufacturer);
    io->flush();
    io->close();

    // Read data back from file
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open(FileMode::ReadOnly);

    // Read all fields using the standard read methods
    auto readRegisteredType = NWB::RegisteredType::create(path, readio);
    auto readDevice =
        std::dynamic_pointer_cast<NWB::Device>(readRegisteredType);

    // Read the description via the readDescription field
    auto descriptionData = readDevice->readDescription();
    std::string descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the manufacturer via the readManufacturer field
    auto manufacturerData = readDevice->readManufacturer();
    std::string manufacturerStr = manufacturerData->values().data[0];
    REQUIRE(manufacturerStr == manufacturer);

    readio->close();
  }
}