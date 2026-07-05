#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/hdmf/table/MeaningsTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("MeaningsTable", "[table]")
{
  std::string tablePath = "/test_meanings_table";
  std::string targetPath = "/test_target_vector_data";

  SECTION("test MeaningsTable is registered as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("hdmf-common::MeaningsTable") != registry.end());
  }

  SECTION("test initialization and column creation")
  {
    std::string path = getTestFilePath("testMeaningsTable.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // Create target VectorData
    auto targetVectorData = NWB::VectorData::create(targetPath, io);
    IO::ArrayDataSetConfig targetConfig(
        BaseDataType::I32, SizeArray {0}, SizeArray {100});
    Status targetStatus =
        targetVectorData->initialize(targetConfig, "Target vector data");
    REQUIRE(targetStatus == Status::Success);
    targetVectorData->finalize();

    // Create MeaningsTable
    auto table = NWB::MeaningsTable::create(tablePath, io);
    auto specs = NWB::MeaningsTable::createDefaultDataSpecs(
        *targetVectorData, BaseDataType::I32, 50);
    Status status = table->initialize(
        *targetVectorData, BaseDataType::I32, "A test meanings table", specs);
    REQUIRE(status == Status::Success);

    // Finalize table to write attributes
    Status finalizeStatus = table->finalize();
    REQUIRE(finalizeStatus == Status::Success);

    // Test reading description
    auto readDesc = table->readDescription()->values().data;
    REQUIRE(readDesc[0] == "A test meanings table");

    // Test column names
    auto readColNames = table->readColNames()->values().data;
    std::vector<std::string> expectedColNames = {"value", "meaning"};
    REQUIRE(readColNames == expectedColNames);

    // Test value column chunk size
    auto chunking = io->getStorageObjectChunking(tablePath + "/value");
    REQUIRE(chunking.size() == 1);
    REQUIRE(chunking[0] == 50);

    // Test meaning column chunk size
    auto meaningChunking = io->getStorageObjectChunking(tablePath + "/meaning");
    REQUIRE(meaningChunking.size() == 1);
    REQUIRE(meaningChunking[0] == 50);

    // Test target link
    REQUIRE(io->objectExists(tablePath + "/target"));
    REQUIRE(io->getStorageObjectType(tablePath + "/target")
            == StorageObjectType::Dataset);

    io->close();
  }

  SECTION("test adding and reading data")
  {
    std::string path = getTestFilePath("testMeaningsTableData.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // Create target VectorData
    auto targetVectorData = NWB::VectorData::create(targetPath, io);
    IO::ArrayDataSetConfig targetConfig(
        BaseDataType::I32, SizeArray {0}, SizeArray {100});
    targetVectorData->initialize(targetConfig, "Target vector data");
    targetVectorData->finalize();

    // Create MeaningsTable
    auto table = NWB::MeaningsTable::create(tablePath, io);
    auto specs = NWB::MeaningsTable::createDefaultDataSpecs(*targetVectorData,
                                                            BaseDataType::I32);
    table->initialize(*targetVectorData, BaseDataType::I32, "", specs);

    // Add data to the table
    std::vector<int> values = {1, 2, 3};
    std::vector<std::string> meanings = {"meaning 1", "meaning 2", "meaning 3"};

    // Get the columns
    auto valueCol = std::dynamic_pointer_cast<NWB::VectorDataTyped<int>>(
        table->readColumn<NWB::VectorDataTyped<int>>("value"));
    auto meaningCol =
        table->readColumn<NWB::VectorDataTyped<std::string>>("meaning");

    REQUIRE(valueCol != nullptr);
    REQUIRE(meaningCol != nullptr);

    // Start recording to write data
    io->startRecording();

    // Write data
    valueCol->recordData()->writeDataBlock(SizeArray {values.size()},
                                           SizeArray {0},
                                           BaseDataType::I32,
                                           values.data());
    meaningCol->recordData()->writeDataBlock(SizeArray {meanings.size()},
                                             SizeArray {0},
                                             BaseDataType::V_STR,
                                             meanings);

    // Stop the recording and finalize the table
    io->stopRecording();
    io->close();

    // Reopen file and verify data
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::MeaningsTable::create(tablePath, io);

    // Read values
    auto readValueCol = readTable->readValueColumn();
    auto readValueColTyped =
        NWB::VectorDataTyped<int>::fromVectorData(readValueCol);
    REQUIRE(readValueCol != nullptr);
    REQUIRE(readValueColTyped != nullptr);
    auto readValues = readValueColTyped->readData()->values().data;
    REQUIRE(readValues == values);

    // Read meanings
    auto readMeaningCol = readTable->readMeaningColumn();
    REQUIRE(readMeaningCol != nullptr);
    auto readMeanings = readMeaningCol->readData()->values().data;
    REQUIRE(readMeanings == meanings);

    io->close();
  }
}
