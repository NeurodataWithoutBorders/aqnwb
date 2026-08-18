#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/event/DurationVectorData.hpp"
#include "nwb/event/TimestampVectorData.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/MeaningsTable.hpp"
#include "nwb/hdmf/table/VectorIndex.hpp"
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

    // Add columns so the table tracks them before reordering colnames
    IO::ArrayDataSetConfig emptyConfig(BaseDataType::V_STR, {0}, {3});
    auto col1 = NWB::VectorData::create(mergePaths(tablePath, "col1"), io);
    auto col2 = NWB::VectorData::create(mergePaths(tablePath, "col2"), io);
    auto col3 = NWB::VectorData::create(mergePaths(tablePath, "col3"), io);
    REQUIRE(col1->initialize(emptyConfig, "Column 1") == Status::Success);
    REQUIRE(col2->initialize(emptyConfig, "Column 2") == Status::Success);
    REQUIRE(col3->initialize(emptyConfig, "Column 3") == Status::Success);
    REQUIRE(table->addColumn(col1) == Status::Success);
    REQUIRE(table->addColumn(col2) == Status::Success);
    REQUIRE(table->addColumn(col3) == Status::Success);

    // Adding columns should flush colnames immediately.
    auto readInitialColNames = table->readColNames()->values().data;
    REQUIRE(readInitialColNames
            == std::vector<std::string>({"col1", "col2", "col3"}));

    // Test reordering and reading column names
    std::vector<std::string> colNames = {"col3", "col2", "col1"};
    table->setColNames(colNames);

    auto readColNames = table->readColNames()->values().data;
    REQUIRE(readColNames == colNames);

    io->close();

    // Verify colnames persist without relying on finalize()
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::DynamicTable::create(tablePath, io);
    auto reopenedColNames = readTable->readColNames()->values().data;
    REQUIRE(reopenedColNames == colNames);
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
    status = table->setRowIDs(ids);
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

  SECTION("test setColNames rejects non-permutations")
  {
    std::string path = getTestFilePath("testDynamicTableInvalidColNames.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    Status status = table->initialize("Table with invalid colname reorder");
    REQUIRE(status == Status::Success);

    IO::ArrayDataSetConfig emptyConfig(BaseDataType::V_STR, {0}, {2});
    auto col1 = NWB::VectorData::create(mergePaths(tablePath, "col1"), io);
    auto col2 = NWB::VectorData::create(mergePaths(tablePath, "col2"), io);
    REQUIRE(col1->initialize(emptyConfig, "Column 1") == Status::Success);
    REQUIRE(col2->initialize(emptyConfig, "Column 2") == Status::Success);
    REQUIRE(table->addColumn(col1) == Status::Success);
    REQUIRE(table->addColumn(col2) == Status::Success);

    auto initialColNames = table->readColNames()->values().data;
    REQUIRE(initialColNames == std::vector<std::string>({"col1", "col2"}));

    REQUIRE_THROWS_AS(table->setColNames({"col1"}), std::invalid_argument);
    REQUIRE_THROWS_AS(table->setColNames({"col1", "col3"}),
                      std::invalid_argument);

    auto unchangedColNames = table->readColNames()->values().data;
    REQUIRE(unchangedColNames == initialColNames);

    io->close();
  }

  SECTION("test DynamicTable validation")
  {
    std::string path = getTestFilePath("testDynamicTableValidation.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);

    // 1. Valid specs (contain "id")
    std::vector<NWB::DynamicTable::DataSpecPtr> validSpecs = {
        NWB::ElementIdentifiers::createDataSpec(
            "id", IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10}))};
    REQUIRE(table->validateDataSpecs(validSpecs) == Status::Success);

    // 2. Invalid specs (missing "id")
    std::vector<NWB::DynamicTable::DataSpecPtr> invalidSpecs = {
        NWB::VectorData::createDataSpec(
            "col1",
            IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}),
            "column 1")};
    REQUIRE(table->validateDataSpecs(invalidSpecs) == Status::Failure);

    // 3. Empty specs (should fail as "id" is missing)
    std::vector<NWB::DynamicTable::DataSpecPtr> emptySpecs;
    REQUIRE(table->validateDataSpecs(emptySpecs) == Status::Failure);

    // 4. Test initialize with invalid specs throws std::invalid_argument
    REQUIRE_THROWS_AS(table->initialize("Test Table", invalidSpecs),
                      std::invalid_argument);

    io->close();
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
      auto timestampColumn = AQNWB::NWB::TimestampVectorData::create(
          mergePaths(tablePath, "timestamp"), io);
      REQUIRE(timestampColumn != nullptr);
      status = timestampColumn->initialize(
          eventConfig, "Timestamp column for event table", 0.001f);
      REQUIRE(status == Status::Success);
      status = timestampColumn->recordData()->writeDataBlock(
          dataShape, positionOffset, BaseDataType::F32, timestamps.data());
      REQUIRE(status == Status::Success);

      auto durationColumn = AQNWB::NWB::DurationVectorData::create(
          mergePaths(tablePath, "duration"), io);
      REQUIRE(durationColumn != nullptr);
      status = durationColumn->initialize(
          eventConfig, "Duration column for event table", 0.001f);
      REQUIRE(status == Status::Success);
      status = durationColumn->recordData()->writeDataBlock(
          dataShape, positionOffset, BaseDataType::F32, durations.data());
      REQUIRE(status == Status::Success);

      status = table->setRowIDs(ids);
      REQUIRE(status == Status::Success);

      status = table->addColumn(timestampColumn);
      REQUIRE(status == Status::Success);

      status = table->addColumn(durationColumn);
      REQUIRE(status == Status::Success);

      status = table->finalize();
      REQUIRE(status == Status::Success);

      auto colNames = table->readColNames()->values().data;
      REQUIRE(colNames == std::vector<std::string>({"timestamp", "duration"}));

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
          readTable->readColumn<AQNWB::NWB::TimestampVectorData>("timestamp");
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
          readTable->readColumn<AQNWB::NWB::DurationVectorData>("duration");
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

  SECTION("test createMeaningsTable and readMeaningsTable")
  {
    std::string path = getTestFilePath("testDynamicTableMeanings.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    Status status = table->initialize("Table with meanings");
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

    // Create MeaningsTable
    auto meaningsTable = table->createMeaningsTable("col1");
    REQUIRE(meaningsTable != nullptr);

    // Add values to MeaningsTable
    std::vector<std::string> meaningValues = {"value1", "value2", "value3"};
    std::vector<std::string> meanings = {"meaning1", "meaning2", "meaning3"};

    auto valueCol = meaningsTable->readValueColumn();
    REQUIRE(valueCol != nullptr);
    status =
        valueCol->recordData()->writeDataBlock(SizeArray {meaningValues.size()},
                                               SizeArray {0},
                                               BaseDataType::V_STR,
                                               meaningValues);
    REQUIRE(status == Status::Success);

    auto meaningCol = meaningsTable->readMeaningColumn();
    REQUIRE(meaningCol != nullptr);
    status =
        meaningCol->recordData()->writeDataBlock(SizeArray {meanings.size()},
                                                 SizeArray {0},
                                                 BaseDataType::V_STR,
                                                 meanings);
    REQUIRE(status == Status::Success);

    std::vector<int> meaningIds = {1, 2, 3};
    status = meaningsTable->setRowIDs(meaningIds);
    REQUIRE(status == Status::Success);

    // Set row IDs for main table
    std::vector<int> ids = {1, 2, 3};
    status = table->setRowIDs(ids);
    REQUIRE(status == Status::Success);

    // Finalize main table
    io->stopRecording();
    io->close();

    // Reopen and read
    auto readio = createIO("HDF5", path);
    readio->open();

    auto readTable = NWB::DynamicTable::create(tablePath, readio);

    // Read MeaningsTable
    auto readMeaningsTable = readTable->readMeaningsTable("col1_meanings");
    REQUIRE(readMeaningsTable != nullptr);
    REQUIRE(readMeaningsTable->getPath()
            == mergePaths(tablePath, "meanings_tables/col1_meanings"));

    // Read value column
    auto readValueCol = readMeaningsTable->readValueColumn();
    REQUIRE(readValueCol != nullptr);
    auto readValueColTyped =
        AQNWB::NWB::VectorDataTyped<std::string>::fromVectorData(readValueCol);
    REQUIRE(readValueColTyped != nullptr);
    auto readValueData = readValueColTyped->readData()->values().data;
    REQUIRE(readValueData.size() == meaningValues.size());
    for (size_t i = 0; i < readValueData.size(); ++i) {
      REQUIRE(readValueData[i] == values[i]);
    }

    // Read meaning column
    auto readMeaningCol = readMeaningsTable->readMeaningColumn();
    REQUIRE(readMeaningCol != nullptr);
    auto readMeaningData = readMeaningCol->readData()->values().data;
    REQUIRE(readMeaningData.size() == meanings.size());
    for (size_t i = 0; i < readMeaningData.size(); ++i) {
      REQUIRE(readMeaningData[i] == meanings[i]);
    }

    // Read target VectorData link
    auto readTarget = readMeaningsTable->readTarget();
    REQUIRE(readTarget != nullptr);
    auto readTargetTyped =
        AQNWB::NWB::VectorDataTyped<std::string>::fromVectorData(readTarget);
    REQUIRE(readTargetTyped != nullptr);
    auto readTargetValues = readTargetTyped->readData()->values().data;
    REQUIRE(readTargetValues.size() == values.size());
    for (size_t i = 0; i < readTargetValues.size(); ++i) {
      // Check that the target values from the link match the original values
      REQUIRE(readTargetValues[i] == values[i]);
    }

    // close
    readio->close();
  }

  SECTION("test getNumberOfRows")
  {
    std::string path = getTestFilePath("testDynamicTableGetNumberOfRows.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);

    // Missing id column
    REQUIRE(table->getNumberOfRows() == 0);

    // Configure schema
    std::vector<NWB::DynamicTable::DataSpecPtr> specs;
    specs.push_back(NWB::ElementIdentifiers::createDataSpec(
        "id", IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10})));
    specs.push_back(NWB::VectorData::createDataSpec(
        "col_str",
        IO::ArrayDataSetConfig(IO::BaseDataType::V_STR, {0}, {10}),
        "String column"));

    Status status = table->initialize("Table with rows", specs);
    REQUIRE(status == Status::Success);

    // Initially 0 rows
    REQUIRE(table->getNumberOfRows() == 0);

    // Add single row
    AQNWB::Types::RowData row1 = {{"col_str", std::string("row1")}};
    status = table->addRow(row1);
    REQUIRE(status == Status::Success);
    REQUIRE(table->getNumberOfRows() == 1);

    // Add multiple rows
    std::vector<AQNWB::Types::RowData> rows = {
        {{"col_str", std::string("row2")}}, {{"col_str", std::string("row3")}}};
    status = table->addRows(rows);
    REQUIRE(status == Status::Success);
    REQUIRE(table->getNumberOfRows() == 3);

    status = table->finalize();
    REQUIRE(status == Status::Success);

    io->close();

    // Reopen and verify
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::DynamicTable::create(tablePath, io);
    REQUIRE(readTable->getNumberOfRows() == 3);

    io->close();
  }

  SECTION("test row-wise append")
  {
    std::string path = getTestFilePath("testDynamicTableRows.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);

    // Configure schema
    std::vector<NWB::DynamicTable::DataSpecPtr> specs;
    specs.push_back(NWB::ElementIdentifiers::createDataSpec(
        "id", IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10})));
    specs.push_back(NWB::VectorData::createDataSpec(
        "col_str",
        IO::ArrayDataSetConfig(IO::BaseDataType::V_STR, {0}, {10}),
        "String column"));
    specs.push_back(NWB::VectorData::createDataSpec(
        "col_f32",
        IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}),
        "Float column"));

    Status status = table->initialize("Table with rows", specs);
    REQUIRE(status == Status::Success);

    // Add single row
    AQNWB::Types::RowData row1 = {{"col_str", std::string("row1")},
                                  {"col_f32", 1.5f}};
    status = table->addRow(row1);
    REQUIRE(status == Status::Success);

    // Add multiple rows
    std::vector<AQNWB::Types::RowData> rows = {
        {{"col_str", std::string("row2")}, {"col_f32", 2.5f}},
        {{"col_str", std::string("row3")}, {"col_f32", 3.5f}}};
    status = table->addRows(rows);
    REQUIRE(status == Status::Success);

    status = table->finalize();
    REQUIRE(status == Status::Success);

    io->close();

    // Reopen and verify
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::DynamicTable::create(tablePath, io);

    auto readColNames = readTable->readColNames()->values().data;
    REQUIRE(readColNames == std::vector<std::string>({"col_str", "col_f32"}));

    auto readIds = readTable->readIdColumn()->readData()->values().data;
    REQUIRE(readIds == std::vector<int>({0, 1, 2}));

    auto colStr = readTable->readColumn<NWB::VectorData>("col_str");
    auto colStrDataGeneric = colStr->readData()->valuesGeneric();
    auto colStrDataTyped =
        DataBlock<std::string>::fromGeneric(colStrDataGeneric);
    auto colStrData = colStrDataTyped.data;
    REQUIRE(colStrData.size() == 3);
    REQUIRE(colStrData == std::vector<std::string>({"row1", "row2", "row3"}));

    auto colF32 = readTable->readColumn<NWB::VectorData>("col_f32");
    auto colF32DataGeneric = colF32->readData()->valuesGeneric();
    auto colF32DataTyped = DataBlock<float>::fromGeneric(colF32DataGeneric);
    auto colF32Data = colF32DataTyped.data;
    REQUIRE(colF32Data.size() == 3);
    REQUIRE(colF32Data == std::vector<float>({1.5f, 2.5f, 3.5f}));

    io->close();
  }

  SECTION("test addColumn with values configures column for addRow")
  {
    // Verify that addColumn(vectorData, values) registers the column in
    // m_configuredColumns so that addRow/addRows can be used afterward
    // without pre-configuring via specs.
    std::string path =
        getTestFilePath("testDynamicTableAddColumnWithValues.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    Status status = table->initialize("Table built with addColumn");
    REQUIRE(status == Status::Success);

    // Add a string column using addColumn(vectorData, values)
    std::vector<std::string> initialValues = {"a", "b", "c"};
    SizeArray dataShape = {initialValues.size()};
    SizeArray chunking = {10};  // chunked to allow append
    IO::ArrayDataSetConfig config(BaseDataType::V_STR, SizeArray {0}, chunking);
    auto col1 = NWB::VectorData::create(mergePaths(tablePath, "col1"), io);
    col1->initialize(config, "Column 1");
    status = table->addColumn(col1, initialValues);
    REQUIRE(status == Status::Success);

    // Now use addRow to append another row — this requires col1 to be in
    // m_configuredColumns, which addColumn should have registered it into.
    AQNWB::Types::RowData newRow = {{"col1", std::string("d")}};
    status = table->addRow(newRow);
    REQUIRE(status == Status::Success);

    io->close();

    // Reopen and verify all 4 values are present
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::DynamicTable::create(tablePath, io);
    auto readCol = readTable->readColumn<NWB::VectorData>("col1");
    REQUIRE(readCol != nullptr);
    auto readData = readCol->readData()->valuesGeneric();
    auto readTyped = DataBlock<std::string>::fromGeneric(readData);
    REQUIRE(readTyped.data == std::vector<std::string>({"a", "b", "c", "d"}));
    io->close();
  }

  SECTION("test addColumn without values configures column for addRow")
  {
    // Verify that addColumn(vectorData) (no values) registers the column in
    // m_configuredColumns so that addRow/addRows can be used afterward.
    std::string path =
        getTestFilePath("testDynamicTableAddColumnConfigures.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    Status status = table->initialize("Table built with addColumn no values");
    REQUIRE(status == Status::Success);

    // Initialize and add a float column without writing data yet
    SizeArray chunking = {10};
    IO::ArrayDataSetConfig config(BaseDataType::F32, SizeArray {0}, chunking);
    auto col1 = NWB::VectorData::create(mergePaths(tablePath, "col1"), io);
    col1->initialize(config, "Float column");
    status = table->addColumn(col1);
    REQUIRE(status == Status::Success);

    // Use addRows to write data — requires col1 to be in m_configuredColumns
    std::vector<AQNWB::Types::RowData> rows = {
        {{"col1", 1.0f}}, {{"col1", 2.0f}}, {{"col1", 3.0f}}};
    status = table->addRows(rows);
    REQUIRE(status == Status::Success);

    io->close();

    // Reopen and verify
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::DynamicTable::create(tablePath, io);
    auto readCol = readTable->readColumn<NWB::VectorData>("col1");
    REQUIRE(readCol != nullptr);
    auto readData = readCol->readData()->valuesGeneric();
    auto readTyped = DataBlock<float>::fromGeneric(readData);
    REQUIRE(readTyped.data == std::vector<float>({1.0f, 2.0f, 3.0f}));
    io->close();
  }

  // TODO : Add row for reference columns is not yet working.
  //        This will require support for chunked reference columns
  //        and support for column configuration with reference columns.
  /*
  SECTION("test addReferenceColumn configures column for addRow")
  {
    // Verify that addReferenceColumn registers the column in
    // m_configuredColumns so that addRow/addRows can be used afterward.
    std::string path =
        getTestFilePath("testDynamicTableAddReferenceColumn.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    Status status = table->initialize("Table with reference column");
    REQUIRE(status == Status::Success);

    // Add a reference column
    std::vector<std::string> refs = {"/path/to/obj1", "/path/to/obj2"};
    status = table->addReferenceColumn("group", "electrode group", refs);
    REQUIRE(status == Status::Success);

    // Verify the column name was registered
    auto colNames = table->readColNames()->values().data;
    REQUIRE(colNames == std::vector<std::string>({"group"}));

    io->close();
  }
  */

  SECTION("test ragged array columns with VectorIndex")
  {
    std::string path = getTestFilePath("testDynamicTableRaggedArray.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);

    // Configure schema
    std::vector<NWB::DynamicTable::DataSpecPtr> specs;
    specs.push_back(NWB::ElementIdentifiers::createDataSpec(
        "id", IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10})));
    specs.push_back(NWB::VectorData::createDataSpec(
        "col_str",
        IO::ArrayDataSetConfig(IO::BaseDataType::V_STR, {0}, {10}),
        "String column"));

    Status status = table->initialize("Table with ragged array", specs);
    REQUIRE(status == Status::Success);

    // Add target column for ragged array
    auto targetCol =
        NWB::VectorData::create(mergePaths(tablePath, "ragged_data"), io);
    targetCol->initialize(
        IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10}),
        "Target data for ragged array");
    status = table->addColumn(targetCol);
    REQUIRE(status == Status::Success);
    REQUIRE(table->readColNames()->values().data
            == std::vector<std::string>({"col_str", "ragged_data"}));

    // Add VectorIndex column
    auto indexCol = NWB::VectorIndex::create(
        mergePaths(tablePath, "ragged_data_index"), io);
    indexCol->initialize(
        IO::ArrayDataSetConfig(IO::BaseDataType::U32, {0}, {10}),
        "Index for ragged array",
        targetCol->getPath());
    status = table->addColumn(indexCol);
    REQUIRE(status == Status::Success);
    REQUIRE(table->readColNames()->values().data
            == std::vector<std::string>({"col_str", "ragged_data"}));

    // Add rows
    std::vector<AQNWB::Types::RowData> rows = {
        {{"col_str", std::string("row1")},
         {"ragged_data", std::vector<int> {1, 2, 3}}},
        {{"col_str", std::string("row2")},
         {"ragged_data", std::vector<int> {4, 5}}},
        {{"col_str", std::string("row3")},
         {"ragged_data", std::vector<int> {6, 7, 8, 9}}}};
    status = table->addRows(rows);
    REQUIRE(status == Status::Success);

    status = table->finalize();
    REQUIRE(status == Status::Success);

    io->close();

    // Reopen and verify
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::DynamicTable::create(tablePath, io);

    auto readColNames = readTable->readColNames()->values().data;
    REQUIRE(readColNames
            == std::vector<std::string>({"col_str", "ragged_data"}));

    auto readIds = readTable->readIdColumn()->readData()->values().data;
    REQUIRE(readIds == std::vector<int>({0, 1, 2}));

    auto colStr = readTable->readColumn<NWB::VectorData>("col_str");
    auto colStrDataGeneric = colStr->readData()->valuesGeneric();
    auto colStrDataTyped =
        DataBlock<std::string>::fromGeneric(colStrDataGeneric);
    auto colStrData = colStrDataTyped.data;
    REQUIRE(colStrData.size() == 3);
    REQUIRE(colStrData == std::vector<std::string>({"row1", "row2", "row3"}));

    auto raggedDataCol = readTable->readColumn<NWB::VectorData>("ragged_data");
    auto raggedDataGeneric = raggedDataCol->readData()->valuesGeneric();
    auto raggedDataTyped = DataBlock<int>::fromGeneric(raggedDataGeneric);
    auto raggedData = raggedDataTyped.data;
    REQUIRE(raggedData.size() == 9);
    REQUIRE(raggedData == std::vector<int>({1, 2, 3, 4, 5, 6, 7, 8, 9}));

    auto raggedIndexCol =
        readTable->readColumn<NWB::VectorIndex>("ragged_data_index");
    auto raggedIndexGeneric = raggedIndexCol->readData()->valuesGeneric();
    auto raggedIndexTyped =
        DataBlock<uint32_t>::fromGeneric(raggedIndexGeneric);
    auto raggedIndexData = raggedIndexTyped.data;
    REQUIRE(raggedIndexData.size() == 3);
    REQUIRE(raggedIndexData == std::vector<uint32_t>({3, 5, 9}));

    io->close();
  }

  SECTION("test readRows")
  {
    std::string path = getTestFilePath("testDynamicTableReadRows.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);

    // Configure schema
    std::vector<NWB::DynamicTable::DataSpecPtr> specs;
    specs.push_back(NWB::ElementIdentifiers::createDataSpec(
        "id", IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10})));
    specs.push_back(NWB::VectorData::createDataSpec(
        "col_str",
        IO::ArrayDataSetConfig(IO::BaseDataType::V_STR, {0}, {10}),
        "String column"));
    specs.push_back(NWB::VectorData::createDataSpec(
        "col_f32",
        IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}),
        "Float column"));

    Status status = table->initialize("Table with rows", specs);
    REQUIRE(status == Status::Success);

    // Add target column for ragged array
    auto targetCol =
        NWB::VectorData::create(mergePaths(tablePath, "ragged_data"), io);
    targetCol->initialize(
        IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10}),
        "Target data for ragged array");
    status = table->addColumn(targetCol);
    REQUIRE(status == Status::Success);

    // Add VectorIndex column
    auto indexCol = NWB::VectorIndex::create(
        mergePaths(tablePath, "ragged_data_index"), io);
    indexCol->initialize(
        IO::ArrayDataSetConfig(IO::BaseDataType::U32, {0}, {10}),
        "Index for ragged array",
        targetCol->getPath());
    status = table->addColumn(indexCol);
    REQUIRE(status == Status::Success);

    // Add rows
    std::vector<AQNWB::Types::RowData> rows = {
        {{"col_str", std::string("row1")},
         {"col_f32", 1.5f},
         {"ragged_data", std::vector<int> {1, 2, 3}}},
        {{"col_str", std::string("row2")},
         {"col_f32", 2.5f},
         {"ragged_data", std::vector<int> {4, 5}}},
        {{"col_str", std::string("row3")},
         {"col_f32", 3.5f},
         {"ragged_data", std::vector<int> {6, 7, 8, 9}}},
        {{"col_str", std::string("row4")},
         {"col_f32", 4.5f},
         {"ragged_data", std::vector<int> {10}}},
        {{"col_str", std::string("row5")},
         {"col_f32", 5.5f},
         {"ragged_data", std::vector<int> {11, 12}}}};
    status = table->addRows(rows);
    REQUIRE(status == Status::Success);

    status = table->finalize();
    REQUIRE(status == Status::Success);

    io->close();

    // Reopen and verify
    io = createIO("HDF5", path);
    io->open();
    auto readTable = NWB::DynamicTable::create(tablePath, io);

    // Read all rows
    auto readAllRows = readTable->readRows(0, 5);
    REQUIRE(readAllRows.size() == 5);

    // Read out of bounds
    REQUIRE_THROWS_AS(readTable->readRows(10, 5), std::invalid_argument);

    // Read with count exceeding available rows
    auto readExceedingRows = readTable->readRows(3, 10);
    REQUIRE(readExceedingRows.size() == 2);

    // Check row 1
    std::string r1_str = readAllRows[0].at("col_str");
    float r1_f32 = readAllRows[0].at("col_f32");
    std::vector<int> r1_ragged = readAllRows[0].at("ragged_data");
    REQUIRE(r1_str == "row1");
    REQUIRE(r1_f32 == Catch::Approx(1.5f));
    REQUIRE(r1_ragged == std::vector<int>({1, 2, 3}));

    // Check row 3
    std::string r3_str = readAllRows[2].at("col_str");
    float r3_f32 = readAllRows[2].at("col_f32");
    std::vector<int> r3_ragged = readAllRows[2].at("ragged_data");
    REQUIRE(r3_str == "row3");
    REQUIRE(r3_f32 == Catch::Approx(3.5f));
    REQUIRE(r3_ragged == std::vector<int>({6, 7, 8, 9}));

    // Read a slice of rows
    auto readSliceRows = readTable->readRows(1, 3);
    REQUIRE(readSliceRows.size() == 3);

    // Check row 2 (index 0 in slice)
    std::string s0_str = readSliceRows[0].at("col_str");
    float s0_f32 = readSliceRows[0].at("col_f32");
    std::vector<int> s0_ragged = readSliceRows[0].at("ragged_data");
    REQUIRE(s0_str == "row2");
    REQUIRE(s0_f32 == Catch::Approx(2.5f));
    REQUIRE(s0_ragged == std::vector<int>({4, 5}));

    // Check row 4 (index 2 in slice)
    std::string s2_str = readSliceRows[2].at("col_str");
    float s2_f32 = readSliceRows[2].at("col_f32");

    // Read a slice of rows with specific columns
    auto readSliceRowsCols = readTable->readRows(1, 3, {"col_str", "col_f32"});
    REQUIRE(readSliceRowsCols.size() == 3);

    // Check row 2 (index 0 in slice)
    std::string s0_str_cols = readSliceRowsCols[0].at("col_str");
    float s0_f32_cols = readSliceRowsCols[0].at("col_f32");
    REQUIRE(s0_str_cols == "row2");
    REQUIRE(s0_f32_cols == Catch::Approx(2.5f));
    REQUIRE(readSliceRowsCols[0].find("ragged_data")
            == readSliceRowsCols[0].end());

    // Check row 4 (index 2 in slice)
    std::string s2_str_cols = readSliceRowsCols[2].at("col_str");
    float s2_f32_cols = readSliceRowsCols[2].at("col_f32");
    REQUIRE(s2_str_cols == "row4");
    REQUIRE(s2_f32_cols == Catch::Approx(4.5f));
    REQUIRE(readSliceRowsCols[2].find("ragged_data")
            == readSliceRowsCols[2].end());
    std::vector<int> s2_ragged = readSliceRows[2].at("ragged_data");
    REQUIRE(s2_str == "row4");
    REQUIRE(s2_f32 == Catch::Approx(4.5f));
    REQUIRE(s2_ragged == std::vector<int>({10}));

    io->close();
  }

  SECTION("test DynamicTable.toString")
  {
    std::string path = getTestFilePath("testDynamicTableToString.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);

    // Configure schema
    std::vector<NWB::DynamicTable::DataSpecPtr> specs;
    specs.push_back(NWB::ElementIdentifiers::createDataSpec(
        "id", IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10})));
    specs.push_back(NWB::VectorData::createDataSpec(
        "col_str",
        IO::ArrayDataSetConfig(IO::BaseDataType::V_STR, {0}, {10}),
        "String column"));
    specs.push_back(NWB::VectorData::createDataSpec(
        "col_f32",
        IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}),
        "Float column"));

    Status status = table->initialize("Table for toString test", specs);
    REQUIRE(status == Status::Success);

    // Add VectorData column for ragged array target
    auto targetCol =
        NWB::VectorData::create(mergePaths(tablePath, "ragged_data"), io);
    targetCol->initialize(
        IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10}),
        "Target data for ragged array");
    status = table->addColumn(targetCol);
    REQUIRE(status == Status::Success);

    // Add VectorIndex column
    auto indexCol = NWB::VectorIndex::create(
        mergePaths(tablePath, "ragged_data_index"), io);
    indexCol->initialize(
        IO::ArrayDataSetConfig(IO::BaseDataType::U32, {0}, {10}),
        "Index for ragged array",
        targetCol->getPath());
    status = table->addColumn(indexCol);
    REQUIRE(status == Status::Success);

    // Add rows
    std::vector<AQNWB::Types::RowData> rows = {
        {{"col_str", std::string("row1")},
         {"col_f32", 1.5f},
         {"ragged_data", std::vector<int> {1, 2, 3}}},
        {{"col_str", std::string("row2")},
         {"col_f32", 2.5f},
         {"ragged_data", std::vector<int> {4, 5}}}};
    status = table->addRows(rows);
    REQUIRE(status == Status::Success);

    status = table->finalize();
    REQUIRE(status == Status::Success);

    // Test toString
    std::string tableStr = table->toString();
    std::string expectedTableStr =
        "id,col_str,col_f32,ragged_data\n"
        "0,row1,1.500000,\"[1, 2, 3]\"\n"
        "1,row2,2.500000,\"[4, 5]\"\n";
    REQUIRE(tableStr == expectedTableStr);

    // Test toString with specific rows
    std::string rowStr = table->toString(1, 1);
    std::string expectedRowStr =
        "id,col_str,col_f32,ragged_data\n"
        "1,row2,2.500000,\"[4, 5]\"\n";
    REQUIRE(rowStr == expectedRowStr);

    // Test toString with specific columns
    std::string colStr =
        table->toString(0, Types::SizeTypeNotSet, {"col_str"}, false);
    std::string expectedColStr =
        "col_str\n"
        "row1\n"
        "row2\n";
    REQUIRE(colStr == expectedColStr);

    io->close();
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
    status = table->setRowIDs(ids);
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
