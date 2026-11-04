#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/BaseIO.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/device/DeviceModel.hpp"
#include "testUtils.hpp"

using namespace AQNWB::NWB;

TEST_CASE("Device", "[device]")
{
  SECTION(
      "test Device and DeviceModel are registered as subclasses of "
      "RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("core::Device") != registry.end());
    REQUIRE(registry.find("core::DeviceModel") != registry.end());
  }

  SECTION("test Device constructor")
  {
    // create the device without writing data
    std::string path = "test_path";
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    auto device = Device::create(path, io);

    REQUIRE(device->getPath() == path);
    REQUIRE(device->getIO() == io);
  }

  SECTION("test Device and DeviceModel write/read")
  {
    std::string path = getTestFilePath("testDevice.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    auto model = DeviceModel::create("/model", io);
    REQUIRE(model->initialize(DeviceModel::DeviceModelSpec("Imec")
                                  .withModelNumber("PRB_1_4_0480_1")
                                  .withDescription("Neuropixels 1.0 probe"))
            == Status::Success);
    auto device = Device::create("/probe", io);
    REQUIRE(device->initialize(Device::DeviceSpec()
                                   .withDescription("Probe A")
                                   .withSerialNumber("18005116521")
                                   .withModel(model))
            == Status::Success);
    // the deprecated manufacturer attribute is not written
    REQUIRE_FALSE(io->attributeExists("/probe/manufacturer"));
    io->flush();
    io->close();

    // Read data back from file
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open(FileMode::ReadOnly);
    auto readDevice = std::dynamic_pointer_cast<Device>(
        RegisteredType::create("/probe", readio));
    REQUIRE(readDevice != nullptr);
    REQUIRE(readDevice->readDescription()->values().data[0] == "Probe A");
    REQUIRE(readDevice->readSerialNumber()->values().data[0] == "18005116521");

    // the model is read via the link of the device
    auto readModel = readDevice->readModel();
    REQUIRE(readModel != nullptr);
    REQUIRE(readModel->readManufacturer()->values().data[0] == "Imec");
    REQUIRE(readModel->readModelNumber()->values().data[0] == "PRB_1_4_0480_1");
    REQUIRE(readModel->readDescription()->values().data[0]
            == "Neuropixels 1.0 probe");
    readio->close();
  }

  SECTION("test DeviceModel requires a manufacturer")
  {
    std::string path = getTestFilePath("testDeviceModelNoManufacturer.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    auto model = DeviceModel::create("/model", io);
    REQUIRE(model->initialize(DeviceModel::DeviceModelSpec(""))
            == Status::Failure);
    io->close();
  }

  SECTION("test Device without metadata")
  {
    std::string path = getTestFilePath("testDeviceNoMetadata.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    auto device = Device::create("/device", io);
    REQUIRE(device->initialize(Device::DeviceSpec()) == Status::Success);
    REQUIRE_FALSE(io->attributeExists("/device/description"));
    REQUIRE_FALSE(io->attributeExists("/device/manufacturer"));
    REQUIRE_FALSE(io->objectExists("/device/model"));
    io->close();
  }

  SECTION("test the deprecated Device::initialize with a manufacturer")
  {
    std::string path = getTestFilePath("testDeviceDeprecatedManufacturer.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    auto device = Device::create("/device", io);
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4996)
#endif
    REQUIRE(device->initialize("Test Device Description", "Test Manufacturer")
            == Status::Success);
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
    io->flush();
    REQUIRE(device->readDescription()->values().data[0]
            == "Test Device Description");
    REQUIRE(device->readManufacturer()->values().data[0]
            == "Test Manufacturer");
    io->close();
  }
}
