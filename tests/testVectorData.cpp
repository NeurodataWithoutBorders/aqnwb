#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("VectorData", "[base]")
{
  SECTION("test VectorData<int> write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::I32;
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string description = "Test VectorData<int>";
    std::string path = getTestFilePath("testVectorData.h5");

    // Create the HDF5 file to write to
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // create BaseRecordingData to pass to VectorData.initialize
    IO::ArrayDataSetConfig config(dataType, dataShape, chunking);
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(config, dataPath);

    // setup VectorData object
    NWB::VectorData<int> columnVectorData = NWB::VectorData<int>(dataPath, io);
    columnVectorData.initialize(std::move(columnDataset), description);

    // Write data to file
    Status writeStatus = columnVectorData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();
    io->close();

    // Read data back from file
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open(FileMode::ReadOnly);

    // Read all fields using the standard read methods
    auto readRegisteredType = NWB::RegisteredType::create(dataPath, readio);
    auto readVectorData =
        std::make_shared<NWB::VectorData<int>>(dataPath, readio);

    // Read the "namespace" attribute via the readNamespace field
    auto namespaceData = readVectorData->readNamespace();
    std::string namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    auto neurodataTypeData = readVectorData->readNeurodataType();
    std::string neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    auto descriptionData = readVectorData->readDescription();
    std::string descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data via the readData field
    auto dataData = readVectorData->readData<int>();
    auto dataBlockInt = dataData->values();
    REQUIRE(dataBlockInt.data == data);

    readio->close();
  }

  SECTION("test VectorData<double> write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::F64;
    std::vector<double> data = {
        1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
    std::string description = "Test VectorData<double>";
    std::string path = getTestFilePath("testVectorData.h5");

    // Create the HDF5 file to write to
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // create BaseRecordingData to pass to VectorData.initialize
    IO::ArrayDataSetConfig config(dataType, dataShape, chunking);
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(config, dataPath);

    // setup VectorData object
    NWB::VectorData<double> columnVectorData =
        NWB::VectorData<double>(dataPath, io);
    columnVectorData.initialize(std::move(columnDataset), description);

    // Write data to file
    Status writeStatus = columnVectorData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();
    io->close();

    // Read data back from file
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open(FileMode::ReadOnly);

    // Read all fields using the standard read methods
    auto readRegisteredType = NWB::RegisteredType::create(dataPath, readio);
    auto readVectorData =
        std::make_shared<NWB::VectorData<double>>(dataPath, readio);

    // Read the "namespace" attribute via the readNamespace field
    auto namespaceData = readVectorData->readNamespace();
    std::string namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    auto neurodataTypeData = readVectorData->readNeurodataType();
    std::string neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    auto descriptionData = readVectorData->readDescription();
    std::string descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data via the readData field
    auto dataData = readVectorData->readData<double>();
    auto dataBlockDouble = dataData->values();
    REQUIRE(dataBlockDouble.data == data);

    readio->close();
  }

  SECTION("test VectorData<string> write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::V_STR;
    std::vector<std::string> data = {"one",
                                     "two",
                                     "three",
                                     "four",
                                     "five",
                                     "six",
                                     "seven",
                                     "eight",
                                     "nine",
                                     "ten"};
    std::string description = "Test VectorData<string>";
    std::string path = getTestFilePath("testVectorData.h5");

    // Create the HDF5 file to write to
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // create BaseRecordingData to pass to VectorData.initialize
    IO::ArrayDataSetConfig config(dataType, dataShape, chunking);
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(config, dataPath);

    // setup VectorData object
    NWB::VectorData<std::string> columnVectorData =
        NWB::VectorData<std::string>(dataPath, io);
    columnVectorData.initialize(std::move(columnDataset), description);

    // Write data to file
    Status writeStatus = columnVectorData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data);
    REQUIRE(writeStatus == Status::Success);
    io->flush();
    io->close();

    // Read data back from file
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open(FileMode::ReadOnly);

    // Read all fields using the standard read methods
    auto readRegisteredType = NWB::RegisteredType::create(dataPath, readio);
    auto readVectorData =
        std::make_shared<NWB::VectorData<std::string>>(dataPath, readio);

    // Read the "namespace" attribute via the readNamespace field
    auto namespaceData = readVectorData->readNamespace();
    std::string namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    auto neurodataTypeData = readVectorData->readNeurodataType();
    std::string neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    auto descriptionData = readVectorData->readDescription();
    std::string descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data via the readData field
    auto dataData = readVectorData->readData<std::string>();
    auto dataBlockString = dataData->values();
    REQUIRE(dataBlockString.data == data);

    readio->close();
  }
}
