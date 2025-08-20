#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

// [example_RegisterType_full]
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB::NWB;

TEST_CASE("RegisterType Example", "[base]")
{
  SECTION("Example to illustrate how the RegisterType registry is working")
  {
    // [example_RegisterType_setup_file]
    // Mock data
    SizeType numSamples = 10;
    std::string dataPath = "/tsdata";
    std::vector<SizeType> dataShape = {numSamples};
    std::vector<SizeType> positionOffset = {0};
    BaseDataType dataType = BaseDataType::F32;
    std::vector<float> data = getMockData1D(numSamples);
    std::vector<double> timestamps = getMockTimestamps(numSamples, 1);

    std::string filename = getTestFilePath("testRegisteredTypeExample.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<IO::HDF5::HDF5IO>(filename);
    io->open();
    auto ts = NWB::TimeSeries::create(dataPath, io);
    IO::ArrayDataSetConfig config(
        dataType, SizeArray {numSamples}, SizeArray {numSamples});
    ts->initialize(config, "unit");

    // Write data to file
    Status writeStatus =
        ts->writeData(dataShape, positionOffset, data.data(), timestamps.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();
    // [example_RegisterType_setup_file]

    // [example_RegisterType_get_type_instance]
    // Create an instance of an TimeSeries in a file.
    auto instance =
        AQNWB::NWB::RegisteredType::create("core::TimeSeries", dataPath, io);
    REQUIRE(instance != nullptr);
    // [example_RegisterType_get_type_instance]

    // [example_RegisterType_get_registered_names]
    // Retrieve and print registered subclass names
    const auto& registry = AQNWB::NWB::RegisteredType::getRegistry();
    std::cout << "Registered subclasses:" << std::endl;
    for (const auto& subclassName : registry) {
      std::cout << " - " << subclassName << std::endl;
    }
    // [example_RegisterType_get_registered_names]

    // [example_RegisterType_get_registered_factories]
    // Retrieve and print factory map
    const auto& factoryMap = AQNWB::NWB::RegisteredType::getFactoryMap();
    std::cout << "Factory functions for registered subclasses:" << std::endl;
    for (const auto& pair : factoryMap) {
      std::cout << " - " << pair.first << std::endl;
    }
    // [example_RegisterType_get_registered_factories]
  }
}
// [example_RegisterType_full]
