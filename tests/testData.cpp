#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/hdmf/base/Data.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("Data", "[base]")
{
  SECTION("test Data is registered as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    // check that hdfm-common::Data is in the registry
    REQUIRE(registry.find("hdmf-common::Data") != registry.end());
  }

  // Create a single file for all Data test sections
  std::string path = getTestFilePath("testData.h5");
  std::shared_ptr<BaseIO> io = createIO("HDF5", path);
  io->open();

  SECTION("test Data write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/data_basic";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::I32;
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // create BaseRecordingData to pass to Data.initialize
    IO::ArrayDataSetConfig config(dataType, dataShape, chunking);
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(config, dataPath);

    // setup Data object
    auto columnData = NWB::Data(dataPath, io);
    columnData.initialize(std::move(columnDataset));

    // Write data to file
    Status writeStatus = columnData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Read all fields using the standard read methods
    auto readDataUntyped = NWB::RegisteredType::create(dataPath, io);
    auto readData = std::dynamic_pointer_cast<NWB::Data>(readDataUntyped);
    REQUIRE(readData != nullptr);
    REQUIRE(readData->getTypeName() == "Data");
    REQUIRE(readData->getNamespace() == "hdmf-common");

    // Read the "namespace" attribute via the readNamespace field
    auto namespaceData = readData->readNamespace();
    std::string namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    auto neurodataTypeData = readData->readNeurodataType();
    std::string neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "Data");
  }

  io->close();
}

TEST_CASE("DataTyped", "[base]")
{
  // Create a single file for all DataTyped test sections
  std::string path = getTestFilePath("testDataTyped.h5");
  std::shared_ptr<BaseIO> io = createIO("HDF5", path);
  io->open();

  SECTION("test DataTyped<int> write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/data_int";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::I32;
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // create BaseRecordingData to pass to Data.initialize
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(dataType, dataShape, chunking, dataPath);

    // setup Data object
    auto columnData = NWB::Data(dataPath, io);
    columnData.initialize(std::move(columnDataset));

    // Write data to file
    Status writeStatus = columnData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Test DataTyped<int> direct construction
    auto readDataTyped = NWB::DataTyped<int>(dataPath, io);
    REQUIRE(readDataTyped.getTypeName() == "Data");
    REQUIRE(readDataTyped.getNamespace() == "hdmf-common");

    // Read the data via the readData field
    auto dataData = readDataTyped.readData();
    auto dataBlockInt = dataData->values();
    REQUIRE(dataBlockInt.data == data);

    // Test fromData conversion
    auto baseData = NWB::Data(dataPath, io);
    auto convertedDataTyped = NWB::DataTyped<int>::fromData(baseData);
    REQUIRE(convertedDataTyped->getTypeName() == "Data");
    REQUIRE(convertedDataTyped->getNamespace() == "hdmf-common");
    auto convertedData = convertedDataTyped->readData();
    auto convertedBlockInt = convertedData->values();
    REQUIRE(convertedBlockInt.data == data);
  }

  SECTION("test DataTyped<double> write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/data_double";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::F64;
    std::vector<double> data = {
        1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};

    // create BaseRecordingData to pass to Data.initialize
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(dataType, dataShape, chunking, dataPath);

    // setup Data object
    auto columnData = NWB::Data(dataPath, io);
    columnData.initialize(std::move(columnDataset));

    // Write data to file
    Status writeStatus = columnData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Test DataTyped<double> direct construction
    auto readDataTyped = NWB::DataTyped<double>(dataPath, io);
    REQUIRE(readDataTyped.getTypeName() == "Data");
    REQUIRE(readDataTyped.getNamespace() == "hdmf-common");

    // Read the data via the readData field
    auto dataData = readDataTyped.readData();
    auto dataBlockDouble = dataData->values();
    REQUIRE(dataBlockDouble.data == data);

    // Test fromData conversion
    auto baseData = NWB::Data(dataPath, io);
    auto convertedDataTyped = NWB::DataTyped<double>::fromData(baseData);
    REQUIRE(convertedDataTyped->getTypeName() == "Data");
    REQUIRE(convertedDataTyped->getNamespace() == "hdmf-common");
    auto convertedData = convertedDataTyped->readData();
    auto convertedBlockDouble = convertedData->values();
    REQUIRE(convertedBlockDouble.data == data);
  }

  SECTION("test DataTyped<string> write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/data_string";
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

    // create BaseRecordingData to pass to Data.initialize
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(dataType, dataShape, chunking, dataPath);

    // setup Data object
    auto columnData = NWB::Data(dataPath, io);
    columnData.initialize(std::move(columnDataset));

    // Write data to file
    Status writeStatus = columnData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data);
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Test DataTyped<string> direct construction
    auto readDataTyped = NWB::DataTyped<std::string>(dataPath, io);
    REQUIRE(readDataTyped.getTypeName() == "Data");
    REQUIRE(readDataTyped.getNamespace() == "hdmf-common");

    // Read the data via the readData field
    auto dataData = readDataTyped.readData();
    auto dataBlockString = dataData->values();
    REQUIRE(dataBlockString.data == data);

    // Test fromData conversion
    auto baseData = NWB::Data(dataPath, io);
    auto convertedDataTyped = NWB::DataTyped<std::string>::fromData(baseData);
    REQUIRE(convertedDataTyped->getTypeName() == "Data");
    REQUIRE(convertedDataTyped->getNamespace() == "hdmf-common");
    auto convertedData = convertedDataTyped->readData();
    auto convertedBlockString = convertedData->values();
    REQUIRE(convertedBlockString.data == data);
  }

  io->close();
}
