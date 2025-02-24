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
  SECTION("test VectorData is registered as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    // check that hdfm-common::VectorData is in the registry
    REQUIRE(registry.find("hdmf-common::VectorData") != registry.end());
  }

  // Create a single file for all VectorData test sections
  std::string path = getTestFilePath("testVectorData.h5");
  std::shared_ptr<BaseIO> io = createIO("HDF5", path);
  io->open();

  SECTION("test VectorData write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata_basic";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::I32;
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string description = "Test VectorData";

    // create BaseRecordingData to pass to VectorData.initialize
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(dataType, dataShape, chunking, dataPath);

    // setup VectorData object
    auto columnVectorData = NWB::VectorData(dataPath, io);
    columnVectorData.initialize(std::move(columnDataset), description);

    // Write data to file
    Status writeStatus = columnVectorData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Read all fields using the standard read methods
    auto readDataUntyped = NWB::RegisteredType::create(dataPath, io);
    auto readVectorData =
        std::dynamic_pointer_cast<NWB::VectorData>(readDataUntyped);
    REQUIRE(readVectorData != nullptr);

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
  }

  io->close();
}

TEST_CASE("VectorDataTyped", "[base]")
{
  // Create a single file for all VectorDataTyped test sections
  std::string path = getTestFilePath("testVectorDataTyped.h5");
  std::shared_ptr<BaseIO> io = createIO("HDF5", path);
  io->open();

  SECTION("test VectorDataTyped with int")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata_int";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::I32;
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string description = "Test VectorDataTyped with int";

    // create BaseRecordingData to pass to VectorData.initialize
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(dataType, dataShape, chunking, dataPath);

    // setup VectorData object
    auto columnVectorData = NWB::VectorData(dataPath, io);
    columnVectorData.initialize(std::move(columnDataset), description);

    // Write data to file
    Status writeStatus = columnVectorData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Test using RegisteredType::create
    auto readDataUntyped = NWB::RegisteredType::create(dataPath, io);
    auto readVectorData =
        std::dynamic_pointer_cast<NWB::VectorData>(readDataUntyped);
    REQUIRE(readVectorData != nullptr);
    auto readVectorDataTyped =
        std::make_shared<NWB::VectorDataTyped<int>>(dataPath, io);
    REQUIRE(readVectorDataTyped != nullptr);

    // Read the "namespace" attribute via the readNamespace field
    auto namespaceData = readVectorDataTyped->readNamespace();
    std::string namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    auto neurodataTypeData = readVectorDataTyped->readNeurodataType();
    std::string neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    auto descriptionData = readVectorDataTyped->readDescription();
    std::string descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data
    auto dataData = readVectorDataTyped->readData();
    auto dataBlockInt = dataData->values();
    REQUIRE(dataBlockInt.data == data);

    // Test fromVectorData conversion
    auto baseVectorData = NWB::VectorData(dataPath, io);
    auto convertedVectorDataTyped =
        NWB::VectorDataTyped<int>::fromVectorData(baseVectorData);
    REQUIRE(convertedVectorDataTyped != nullptr);

    // Read the "namespace" attribute via the readNamespace field
    namespaceData = convertedVectorDataTyped->readNamespace();
    namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    neurodataTypeData = convertedVectorDataTyped->readNeurodataType();
    neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    descriptionData = convertedVectorDataTyped->readDescription();
    descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data
    auto convertedData = convertedVectorDataTyped->readData();
    auto convertedBlockInt = convertedData->values();
    REQUIRE(convertedBlockInt.data == data);
  }

  SECTION("test VectorDataTyped with double")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata_double";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::F64;
    std::vector<double> data = {
        1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
    std::string description = "Test VectorDataTyped with double";

    // create BaseRecordingData to pass to VectorData.initialize
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(dataType, dataShape, chunking, dataPath);

    // setup VectorData object
    auto columnVectorData = NWB::VectorData(dataPath, io);
    columnVectorData.initialize(std::move(columnDataset), description);

    // Write data to file
    Status writeStatus = columnVectorData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Test using RegisteredType::create
    auto readDataUntyped = NWB::RegisteredType::create(dataPath, io);
    auto readVectorData =
        std::dynamic_pointer_cast<NWB::VectorData>(readDataUntyped);
    REQUIRE(readVectorData != nullptr);
    auto readVectorDataTyped =
        std::make_shared<NWB::VectorDataTyped<double>>(dataPath, io);
    REQUIRE(readVectorDataTyped != nullptr);

    // Read the "namespace" attribute via the readNamespace field
    auto namespaceData = readVectorDataTyped->readNamespace();
    std::string namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    auto neurodataTypeData = readVectorDataTyped->readNeurodataType();
    std::string neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    auto descriptionData = readVectorDataTyped->readDescription();
    std::string descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data
    auto dataData = readVectorDataTyped->readData();
    auto dataBlockDouble = dataData->values();
    REQUIRE(dataBlockDouble.data == data);

    // Test fromVectorData conversion
    auto baseVectorData = NWB::VectorData(dataPath, io);
    auto convertedVectorDataTyped =
        NWB::VectorDataTyped<double>::fromVectorData(baseVectorData);
    REQUIRE(convertedVectorDataTyped != nullptr);

    // Read the "namespace" attribute via the readNamespace field
    namespaceData = convertedVectorDataTyped->readNamespace();
    namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    neurodataTypeData = convertedVectorDataTyped->readNeurodataType();
    neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    descriptionData = convertedVectorDataTyped->readDescription();
    descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data
    auto convertedData = convertedVectorDataTyped->readData();
    auto convertedBlockDouble = convertedData->values();
    REQUIRE(convertedBlockDouble.data == data);
  }

  SECTION("test VectorDataTyped with string")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata_string";
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
    std::string description = "Test VectorDataTyped with string";

    // create BaseRecordingData to pass to VectorData.initialize
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(dataType, dataShape, chunking, dataPath);

    // setup VectorData object
    auto columnVectorData = NWB::VectorData(dataPath, io);
    columnVectorData.initialize(std::move(columnDataset), description);

    // Write data to file
    Status writeStatus = columnVectorData.m_dataset->writeDataBlock(
        dataShape, positionOffset, dataType, data);
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Test using RegisteredType::create
    auto readDataUntyped = NWB::RegisteredType::create(dataPath, io);
    auto readVectorData =
        std::dynamic_pointer_cast<NWB::VectorData>(readDataUntyped);
    REQUIRE(readVectorData != nullptr);
    auto readVectorDataTyped =
        std::make_shared<NWB::VectorDataTyped<std::string>>(dataPath, io);
    REQUIRE(readVectorDataTyped != nullptr);

    // Read the "namespace" attribute via the readNamespace field
    auto namespaceData = readVectorDataTyped->readNamespace();
    std::string namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    auto neurodataTypeData = readVectorDataTyped->readNeurodataType();
    std::string neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    auto descriptionData = readVectorDataTyped->readDescription();
    std::string descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data
    auto dataData = readVectorDataTyped->readData();
    auto dataBlockString = dataData->values();
    REQUIRE(dataBlockString.data == data);

    // Test fromVectorData conversion
    auto baseVectorData = NWB::VectorData(dataPath, io);
    auto convertedVectorDataTyped =
        NWB::VectorDataTyped<std::string>::fromVectorData(baseVectorData);
    REQUIRE(convertedVectorDataTyped != nullptr);

    // Read the "namespace" attribute via the readNamespace field
    namespaceData = convertedVectorDataTyped->readNamespace();
    namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    neurodataTypeData = convertedVectorDataTyped->readNeurodataType();
    neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "VectorData");

    // Read the "description" attribute via the readDescription field
    descriptionData = convertedVectorDataTyped->readDescription();
    descriptionStr = descriptionData->values().data[0];
    REQUIRE(descriptionStr == description);

    // Read the data
    auto convertedData = convertedVectorDataTyped->readData();
    auto convertedBlockString = convertedData->values();
    REQUIRE(convertedBlockString.data == data);
  }

  io->close();
}
