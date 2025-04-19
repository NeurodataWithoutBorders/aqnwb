#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

// [example_RegisterType_templated_full]
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("RegisterType Templated Example", "[base]")
{
  SECTION("Example to illustrate how the RegisterType registry is working")
  {
    // [example_RegisterType_templated_full_setup_data]
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata_int";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::I32;
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string description = "Test VectorDataTyped with int";

    std::string path = getTestFilePath("ExampleTemplatedRegisteredType.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // create config for VectorData.initialize
    IO::ArrayDataSetConfig dataConfig(dataType, dataShape, chunking);

    // setup VectorData object
    auto columnVectorData = NWB::VectorData(dataPath, io);
    columnVectorData.initialize(dataConfig, description);

    // Write data to file
    Status writeStatus = columnVectorData.recordData()->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();
    // [example_RegisterType_templated_full_setup_data]

    // [example_RegisterType_templated_full_read_data]
    // Read as generic RegisteredType and case to VectorData
    auto readDataUntyped = NWB::RegisteredType::create(dataPath, io);
    auto readVectorData =
        std::dynamic_pointer_cast<NWB::VectorData>(readDataUntyped);
    // Read data as DataBlock<std::any>
    auto dataAny = readVectorData->readData();
    auto dataBlock = dataAny->valuesGeneric();

    // Create VectorDataTyped<int> from VectorData and read the
    // data as typed DataBlock<int>
    auto readVectorDataTyped =
        NWB::VectorDataTyped<int>::fromVectorData(*readVectorData);
    auto dataInt = readVectorDataTyped->readData();
    auto dataBlockInt = dataInt->values();
    // [example_RegisterType_templated_full_read_data]
  }
}
// [example_RegisterType_templated_full]
