#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("DynamicTable", "[table]")
{
  std::string tablePath = "/test_table";

  SECTION("test DynamicTable is registerd as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("hdmf-common::DynamicTable") != registry.end());
  }

  SECTION("test initialization and column names")
  {
    std::string path = getTestFilePath("testDynamicTable.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    NWB::DynamicTable table(tablePath, io);
    Status status = table.initialize("A test dynamic table");
    REQUIRE(status == Status::Success);

    // Test reading description
    auto readDesc = table.readDescription()->values().data;
    REQUIRE(readDesc[0] == "A test dynamic table");

    // Test setting and reading column names
    std::vector<std::string> colNames = {"col1", "col2", "col3"};
    table.setColNames(colNames);
    status = table.finalize();
    REQUIRE(status == Status::Success);

    auto readColNames = table.readColNames()->values().data;
    REQUIRE(readColNames == colNames);

    io->close();
  }

  SECTION("test adding columns and row IDs")
  {
    std::string path = getTestFilePath("testDynamicTableColumns.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    NWB::DynamicTable table(tablePath, io);
    Status status = table.initialize("Table with columns");
    REQUIRE(status == Status::Success);

    // Add string vector data column
    std::vector<std::string> values = {"value1", "value2", "value3"};
    SizeArray dataShape = {values.size()};
    SizeArray chunking = {values.size()};
    auto columnDataset = io->createArrayDataSet(
        BaseDataType::V_STR, dataShape, chunking, tablePath + "/col1");
    auto vectorData =
        std::make_unique<NWB::VectorData>(tablePath + "/col1", io);
    vectorData->initialize(std::move(columnDataset), "Column 1");
    status = table.addColumn(vectorData, values);
    REQUIRE(status == Status::Success);

    // Set row IDs
    std::vector<int> ids = {1, 2, 3};
    SizeArray idShape = {ids.size()};
    SizeArray idChunking = {ids.size()};
    auto idDataset = io->createArrayDataSet(
        BaseDataType::I32, idShape, idChunking, tablePath + "/id");
    auto elementIDs =
        std::make_unique<NWB::ElementIdentifiers>(tablePath + "/id", io);
    elementIDs->initialize(std::move(idDataset));
    status = table.setRowIDs(elementIDs, ids);
    REQUIRE(status == Status::Success);

    // Finalize table
    status = table.finalize();
    REQUIRE(status == Status::Success);

    io->close();

    // Reopen file and verify data
    io = createIO("HDF5", path);
    io->open();
    NWB::DynamicTable readTable(tablePath, io);

    auto readColNames = readTable.readColNames()->values().data;
    std::vector<std::string> expectedColNames = {"col1"};
    REQUIRE(readColNames == expectedColNames);

    // Read row IDs
    auto readIdsData = readTable.readIdColumn()->readData()->values().data;
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

      NWB::DynamicTable table(tablePath, io);
      Status status = table.initialize("Table for appending");
      REQUIRE(status == Status::Success);

      // Add initial column
      std::vector<std::string> values = {"value1", "value2", "value3"};
      SizeArray dataShape = {values.size()};
      SizeArray chunking = {values.size()};
      std::string columnPath = mergePaths(tablePath, "col1");
      auto columnDataset = io->createArrayDataSet(
          BaseDataType::V_STR, dataShape, chunking, columnPath);
      auto vectorData = std::make_unique<NWB::VectorData>(columnPath, io);
      vectorData->initialize(std::move(columnDataset), "Column 1");
      status = table.addColumn(vectorData, values);
      REQUIRE(status == Status::Success);

      // table.setColNames({"col1"});
      status = table.finalize();
      REQUIRE(status == Status::Success);

      io->close();
    }

    // Now reopen and append new column
    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      NWB::DynamicTable table(tablePath, io);

      // Add new column
      std::vector<std::string> newValues = {"new1", "new2", "new3"};
      SizeArray newDataShape = {newValues.size()};
      SizeArray newChunking = {newValues.size()};
      std::string columnPath2 = mergePaths(tablePath, "col2");
      auto newColumnDataset = io->createArrayDataSet(
          BaseDataType::V_STR, newDataShape, newChunking, columnPath2);
      auto newVectorData = std::make_unique<NWB::VectorData>(columnPath2, io);
      newVectorData->initialize(std::move(newColumnDataset), "Column 2");
      Status status = table.addColumn(newVectorData, newValues);
      REQUIRE(status == Status::Success);

      // Finalize the table
      status = table.finalize();
      REQUIRE(status == Status::Success);

      // Verify updated column names
      std::vector<std::string> colNames = {"col1", "col2"};
      auto readColNames = table.readColNames()->values().data;
      REQUIRE(readColNames == colNames);

      // Swap the columns
      colNames = {"col2", "col1"};
      table.setColNames(colNames);
      status = table.finalize();
      REQUIRE(status == Status::Success);

      // Verify updated column names
      auto readColNames2 = table.readColNames()->values().data;
      REQUIRE(readColNames2 == colNames);

      io->close();
    }
  }
}
