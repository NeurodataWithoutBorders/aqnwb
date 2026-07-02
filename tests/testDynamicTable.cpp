#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/event/DurationVectorData.hpp"
#include "nwb/event/TimestampVectorData.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("DynamicTable", "[table]")
{
  std::string tablePath = "/test_table";

  SECTION("test DynamicTable is registered as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("hdmf-common::DynamicTable") != registry.end());
  }

  SECTION("test initialization and column names")
  {
    std::string path = getTestFilePath("testDynamicTable.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    Status status = table->initialize("A test dynamic table");
    REQUIRE(status == Status::Success);

    // Test reading description
    auto readDesc = table->readDescription()->values().data;
    REQUIRE(readDesc[0] == "A test dynamic table");

    // Test setting and reading column names
    std::vector<std::string> colNames = {"col1", "col2", "col3"};
    table->setColNames(colNames);
    status = table->finalize();
    REQUIRE(status == Status::Success);

    auto readColNames = table->readColNames()->values().data;
    REQUIRE(readColNames == colNames);

    io->close();
  }

  SECTION("test adding columns and row IDs")
  {
    std::string path = getTestFilePath("testDynamicTableColumns.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    Status status = table->initialize("Table with columns");
    REQUIRE(status == Status::Success);

    // Add string vector data column
    std::vector<std::string> values = {"value1", "value2", "value3"};
    SizeArray dataShape = {values.size()};
    SizeArray chunking = {values.size()};
    IO::ArrayDataSetConfig config(BaseDataType::V_STR, dataShape, chunking);
    auto vectorData = NWB::VectorData::create(tablePath + "/col1", io);
    vectorData->initialize(config, "Column 1");
    status = table->addColumn(vectorData, values);
    REQUIRE(status == Status::Success);

    // Set row IDs
    std::vector<int> ids = {1, 2, 3};
    SizeArray idShape = {ids.size()};
    SizeArray idChunking = {ids.size()};
    IO::ArrayDataSetConfig idConfig(BaseDataType::I32, idShape, idChunking);
    auto elementIDs = NWB::ElementIdentifiers::create(tablePath + "/id", io);
    elementIDs->initialize(idConfig);
    status = table->setRowIDs(elementIDs, ids);
    REQUIRE(status == Status::Success);

    // Finalize table
    status = table->finalize();
    REQUIRE(status == Status::Success);

    io->close();

    // Reopen file and verify data
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::DynamicTable::create(tablePath, io);

    auto readColNames = readTable->readColNames()->values().data;
    std::vector<std::string> expectedColNames = {"col1"};
    REQUIRE(readColNames == expectedColNames);

    // Read row IDs
    auto readIdsData = readTable->readIdColumn()->readData()->values().data;
    REQUIRE(readIdsData == ids);

    io->close();
  }

  SECTION("test appending column to existing table")
  {
    // First create a table with initial columns
    std::string path = getTestFilePath("testDynamicTableAppend.h5");
    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto table = NWB::DynamicTable::create(tablePath, io);
      Status status = table->initialize("Table for appending");
      REQUIRE(status == Status::Success);

      // Add initial column
      std::vector<std::string> values = {"value1", "value2", "value3"};
      SizeArray dataShape = {values.size()};
      SizeArray chunking = {values.size()};
      std::string columnPath = mergePaths(tablePath, "col1");
      IO::ArrayDataSetConfig config(BaseDataType::V_STR, dataShape, chunking);
      auto vectorData = NWB::VectorData::create(columnPath, io);
      vectorData->initialize(config, "Column 1");
      status = table->addColumn(vectorData, values);
      REQUIRE(status == Status::Success);

      // table->setColNames({"col1"});
      status = table->finalize();
      REQUIRE(status == Status::Success);

      io->close();
    }

    // Now reopen and append new column
    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto table = NWB::DynamicTable::create(tablePath, io);

      // Add new column
      std::vector<std::string> newValues = {"new1", "new2", "new3"};
      SizeArray newDataShape = {newValues.size()};
      SizeArray newChunking = {newValues.size()};
      std::string columnPath2 = mergePaths(tablePath, "col2");
      IO::ArrayDataSetConfig config(
          BaseDataType::V_STR, newDataShape, newChunking);
      auto newVectorData = NWB::VectorData::create(columnPath2, io);
      newVectorData->initialize(config, "Column 2");
      Status status = table->addColumn(newVectorData, newValues);
      REQUIRE(status == Status::Success);

      // Finalize the table
      status = table->finalize();
      REQUIRE(status == Status::Success);

      // Verify updated column names
      std::vector<std::string> colNames = {"col1", "col2"};
      auto readColNames = table->readColNames()->values().data;
      REQUIRE(readColNames == colNames);

      // Swap the columns
      colNames = {"col2", "col1"};
      table->setColNames(colNames);
      status = table->finalize();
      REQUIRE(status == Status::Success);

      // Verify updated column names
      auto readColNames2 = table->readColNames()->values().data;
      REQUIRE(readColNames2 == colNames);

      io->close();
    }
  }

  // This test section tests support for derived types of VectorData as columns
  // in DynamicTable, specifically TimestampVectorData and DurationVectorData.
  // It creates a DynamicTable with these columns, writes data to them, and then
  // reads back the data via DynamicTable.readColumn to verify correctness.
  SECTION("test adding TimestampVectorData and DurationVectorData columns")
  {
    std::string path = getTestFilePath("testDynamicTableEventColumns.h5");
    std::vector<float> timestamps = {0.1f, 0.4f, 0.9f};
    std::vector<float> durations = {0.05f, 0.2f, 0.15f};
    std::vector<int> ids = {1, 2, 3};
    SizeArray dataShape = {timestamps.size()};
    SizeArray chunking = {timestamps.size()};
    SizeArray positionOffset = {0};

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto table = NWB::DynamicTable::create(tablePath, io);
      Status status = table->initialize("Table with event columns");
      REQUIRE(status == Status::Success);

      IO::ArrayDataSetConfig eventConfig(
          BaseDataType::F32, dataShape, chunking);
      auto timestampColumn = CORE::TimestampVectorData::create(
          mergePaths(tablePath, "timestamp"), io);
      REQUIRE(timestampColumn != nullptr);
      status = timestampColumn->initialize(
          0.001f, "Timestamp column for event table", eventConfig);
      REQUIRE(status == Status::Success);
      status = timestampColumn->recordData()->writeDataBlock(
          dataShape, positionOffset, BaseDataType::F32, timestamps.data());
      REQUIRE(status == Status::Success);

      auto durationColumn = CORE::DurationVectorData::create(
          mergePaths(tablePath, "duration"), io);
      REQUIRE(durationColumn != nullptr);
      status = durationColumn->initialize(
          0.001f, "Duration column for event table", eventConfig);
      REQUIRE(status == Status::Success);
      status = durationColumn->recordData()->writeDataBlock(
          dataShape, positionOffset, BaseDataType::F32, durations.data());
      REQUIRE(status == Status::Success);

      SizeArray idShape = {ids.size()};
      SizeArray idChunking = {ids.size()};
      IO::ArrayDataSetConfig idConfig(BaseDataType::I32, idShape, idChunking);
      auto elementIDs = NWB::ElementIdentifiers::create(tablePath + "/id", io);
      REQUIRE(elementIDs != nullptr);
      status = elementIDs->initialize(idConfig);
      REQUIRE(status == Status::Success);
      status = table->setRowIDs(elementIDs, ids);
      REQUIRE(status == Status::Success);

      table->setColNames({"timestamp", "duration"});
      status = table->finalize();
      REQUIRE(status == Status::Success);

      io->close();
    }

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto readTable = NWB::DynamicTable::create(tablePath, io);
      auto readColNames = readTable->readColNames()->values().data;
      REQUIRE(readColNames
              == std::vector<std::string>({"timestamp", "duration"}));

      auto readTimestampColumn =
          readTable->readColumn<CORE::TimestampVectorData>("timestamp");
      REQUIRE(readTimestampColumn != nullptr);
      REQUIRE(readTimestampColumn->readUnit()->values().data[0] == "seconds");
      REQUIRE(readTimestampColumn->readResolution()->values().data[0]
              == Catch::Approx(0.001f));
      auto readTimestampValues = readTimestampColumn->readData()->values().data;
      REQUIRE(readTimestampValues.size() == timestamps.size());
      for (size_t i = 0; i < readTimestampValues.size(); ++i) {
        REQUIRE(readTimestampValues[i] == Catch::Approx(timestamps[i]));
      }

      auto readDurationColumn =
          readTable->readColumn<CORE::DurationVectorData>("duration");
      REQUIRE(readDurationColumn != nullptr);
      REQUIRE(readDurationColumn->readUnit()->values().data[0] == "seconds");
      REQUIRE(readDurationColumn->readResolution()->values().data[0]
              == Catch::Approx(0.001f));
      auto readDurationValues = readDurationColumn->readData()->values().data;
      REQUIRE(readDurationValues.size() == durations.size());
      for (size_t i = 0; i < readDurationValues.size(); ++i) {
        REQUIRE(readDurationValues[i] == Catch::Approx(durations[i]));
      }

      auto readIds = readTable->readIdColumn()->readData()->values().data;
      REQUIRE(readIds == ids);

      io->close();
    }
  }

  SECTION("test DynamicTable.findOwnedTypes")
  {
    std::string path = getTestFilePath("testDynamicTableFindOwned.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    Status status = table->initialize("Table with columns");
    REQUIRE(status == Status::Success);

    // Add string vector data column
    std::vector<std::string> values = {"value1", "value2", "value3"};
    SizeArray dataShape = {values.size()};
    SizeArray chunking = {values.size()};
    IO::ArrayDataSetConfig strConfig(BaseDataType::V_STR, dataShape, chunking);
    std::string columnPath = mergePaths(tablePath, "col1");
    auto vectorData = NWB::VectorData::create(columnPath, io);
    vectorData->initialize(strConfig, "Column 1");
    status = table->addColumn(vectorData, values);
    REQUIRE(status == Status::Success);

    // Set row IDs
    std::vector<int> ids = {1, 2, 3};
    SizeArray idShape = {ids.size()};
    SizeArray idChunking = {ids.size()};

    std::string idPath = mergePaths(tablePath, "id");
    IO::ArrayDataSetConfig i32Config(BaseDataType::I32, idShape, idChunking);
    auto elementIDs = NWB::ElementIdentifiers::create(idPath, io);
    elementIDs->initialize(i32Config);
    status = table->setRowIDs(elementIDs, ids);
    REQUIRE(status == Status::Success);

    // Final
    status = table->finalize();
    REQUIRE(status == Status::Success);
    io->flush();

    // Find all typed objects that are owned by this object
    auto types = table->findOwnedTypes();
    REQUIRE(types.size() == 2);
    REQUIRE(types["/test_table/id"] == "hdmf-common::ElementIdentifiers");
    REQUIRE(types["/test_table/col1"] == "hdmf-common::VectorData");

    io->close();
  }
}
