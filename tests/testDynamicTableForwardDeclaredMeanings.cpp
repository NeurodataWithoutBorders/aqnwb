#include <catch2/catch_test_macros.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE(
    "DynamicTable MeaningsTable accessors compile with forward declaration",
    "[table]")
{
  const std::string path =
      getTestFilePath("testDynamicTableForwardDeclaredMeanings.h5");
  const std::string tablePath = "/test_table";

  {
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    REQUIRE(table != nullptr);
    REQUIRE(table->initialize("Table with meanings") == Status::Success);

    const std::vector<std::string> values = {"value1", "value2", "value3"};
    const SizeArray dataShape = {values.size()};
    const SizeArray chunking = {values.size()};
    IO::ArrayDataSetConfig strConfig(BaseDataType::V_STR, dataShape, chunking);
    auto vectorData =
        NWB::VectorData::create(mergePaths(tablePath, "col1"), io);
    REQUIRE(vectorData != nullptr);
    REQUIRE(vectorData->initialize(strConfig, "Column 1") == Status::Success);
    REQUIRE(table->addColumn(vectorData, values) == Status::Success);

    auto meaningsTable = table->createMeaningsTable("col1");
    REQUIRE(meaningsTable != nullptr);
    REQUIRE(io->objectExists(
        mergePaths(tablePath, "meanings_tables/col1_meanings")));

    io->stopRecording();
    io->close();
  }

  {
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto table = NWB::DynamicTable::create(tablePath, io);
    REQUIRE(table != nullptr);

    auto meaningsTable = table->readMeaningsTable("col1_meanings");
    REQUIRE(meaningsTable != nullptr);

    io->close();
  }
}
