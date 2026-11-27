#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/DynamicTableRegion.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("DynamicTableRegion", "[table]")
{
  REQUIRE(NWB::RegisteredType::getRegistry().count(
              "hdmf-common::DynamicTableRegion")
          == 1);

  std::string path = getTestFilePath("testDynamicTableRegion.h5");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();

  // Create the target table with three rows
  auto target = NWB::DynamicTable::create("/target", io);
  REQUIRE(target->initialize(
              NWB::DynamicTable::Spec().withDescription("target table"))
          == Status::Success);
  std::vector<int> ids = {0, 1, 2};
  REQUIRE(target->setRowIDs(ids) == Status::Success);
  REQUIRE(target->finalize() == Status::Success);

  SECTION("standalone region")
  {
    auto region = NWB::DynamicTableRegion::create("/region", io);
    IO::ArrayDataSetConfig config(IO::BaseDataType::I32, {2}, {2});
    REQUIRE(region->initialize(config, "rows 0 and 2", "/target")
            == Status::Success);
    std::vector<int32_t> rows = {0, 2};
    REQUIRE(region->recordData()->writeDataBlock(
                {2}, IO::BaseDataType::I32, rows.data())
            == Status::Success);
    io->close();

    std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
    readIO->open(IO::FileMode::ReadOnly);
    auto readRegion = NWB::RegisteredType::create("/region", readIO);
    REQUIRE(readRegion->getFullTypeName() == "hdmf-common::DynamicTableRegion");
    auto typedRegion =
        std::dynamic_pointer_cast<NWB::DynamicTableRegion>(readRegion);
    REQUIRE(typedRegion != nullptr);
    REQUIRE(typedRegion->readDescription()->values().data[0] == "rows 0 and 2");
    REQUIRE(typedRegion->readData<int32_t>()->values().data == rows);
    auto table = typedRegion->readTable();
    REQUIRE(table != nullptr);
    REQUIRE(table->readDescription()->values().data[0] == "target table");
    readIO->close();
  }

  SECTION("invalid configurations fail")
  {
    auto region = NWB::DynamicTableRegion::create("/region", io);
    REQUIRE(region->initialize(
                IO::ArrayDataSetConfig(IO::BaseDataType::F32, {2}, {2}),
                "float",
                "/target")
            == Status::Failure);
    REQUIRE(region->initialize(
                IO::ArrayDataSetConfig(IO::BaseDataType::I32, {2, 2}, {2, 2}),
                "2D",
                "/target")
            == Status::Failure);
    REQUIRE(region->initialize(
                IO::ArrayDataSetConfig(IO::BaseDataType::I32, {2}, {2}),
                "missing table",
                "/missing")
            == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/region"));
    io->close();
  }

  SECTION("region as a column of a table")
  {
    auto columns = NWB::DynamicTable::createDefaultDataSpecs();
    columns.push_back(NWB::DynamicTableRegion::createDataSpec(
        "target_rows",
        IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10}),
        "rows of the target table",
        "/target"));
    auto table = NWB::DynamicTable::create("/table", io);
    REQUIRE(table->initialize(NWB::DynamicTable::Spec()
                                  .withDescription("table with region")
                                  .withColumns(columns))
            == Status::Success);
    REQUIRE(table->finalize() == Status::Success);
    io->close();

    std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
    readIO->open(IO::FileMode::ReadOnly);
    auto column = NWB::RegisteredType::create("/table/target_rows", readIO);
    REQUIRE(column->getFullTypeName() == "hdmf-common::DynamicTableRegion");
    readIO->close();
  }
}
