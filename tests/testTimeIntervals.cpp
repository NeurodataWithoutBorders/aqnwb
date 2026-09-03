#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/epoch/TimeIntervals.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "nwb/hdmf/table/VectorIndex.hpp"
#include "testUtils.hpp"

using namespace AQNWB;
using namespace AQNWB::NWB;
using namespace AQNWB::IO;
using namespace AQNWB::IO::HDF5;

TEST_CASE("TimeIntervals - createDefaultDataSpecs", "[TimeIntervals]")
{
  SECTION("Without tags column")
  {
    auto specs =
        TimeIntervals::createDefaultDataSpecs("/intervals/epochs", 100, false);

    // Should have id, start_time, stop_time
    REQUIRE(specs.size() == 3);
    REQUIRE(specs[0]->name == "id");
    REQUIRE(specs[1]->name == "start_time");
    REQUIRE(specs[2]->name == "stop_time");
  }

  SECTION("With tags column")
  {
    auto specs =
        TimeIntervals::createDefaultDataSpecs("/intervals/epochs", 100, true);

    // Should have id, start_time, stop_time, tags, tags_index
    REQUIRE(specs.size() == 5);
    REQUIRE(specs[0]->name == "id");
    REQUIRE(specs[1]->name == "start_time");
    REQUIRE(specs[2]->name == "stop_time");
    REQUIRE(specs[3]->name == "tags");
    REQUIRE(specs[4]->name == "tags_index");
  }
}

TEST_CASE("TimeIntervals - Initialize and Add Rows", "[TimeIntervals]")
{
  std::string filename = getTestFilePath("testTimeIntervals.h5");

  SECTION("Basic initialization and row addition")
  {
    {
      auto io = createIO("HDF5", filename);
      io->open(FileMode::Overwrite);

      // Verify intervals group does not exist initially
      REQUIRE(io->objectExists("/intervals") == false);

      auto timeIntervals = TimeIntervals::create("/intervals/epochs", io);

      REQUIRE(timeIntervals != nullptr);

      Status status = timeIntervals->initialize("Test epochs");
      REQUIRE(status == Status::Success);

      // Verify intervals group was created
      REQUIRE(io->objectExists("/intervals") == true);

      // Add some rows
      AQNWB::Types::RowData row1 = {{"start_time", 1.0f}, {"stop_time", 2.0f}};

      AQNWB::Types::RowData row2 = {{"start_time", 2.5f}, {"stop_time", 3.5f}};

      REQUIRE(timeIntervals->addRow(row1) == Status::Success);
      REQUIRE(timeIntervals->addRow(row2) == Status::Success);

      REQUIRE(timeIntervals->finalize() == Status::Success);
    }

    // Read back and verify
    {
      auto io = createIO("HDF5", filename);
      io->open(FileMode::ReadOnly);
      auto timeIntervals = TimeIntervals::create("/intervals/epochs", io);

      REQUIRE(timeIntervals != nullptr);

      auto startTimeCol = timeIntervals->readStartTime();
      REQUIRE(startTimeCol != nullptr);

      auto stopTimeCol = timeIntervals->readStopTime();
      REQUIRE(stopTimeCol != nullptr);

      auto startTimeData = startTimeCol->readData();
      auto stopTimeData = stopTimeCol->readData();

      REQUIRE(startTimeData->getShape()[0] == 2);
      REQUIRE(stopTimeData->getShape()[0] == 2);

      auto startTimes = startTimeData->values().data;
      auto stopTimes = stopTimeData->values().data;

      REQUIRE(startTimes[0] == Catch::Approx(1.0f));
      REQUIRE(startTimes[1] == Catch::Approx(2.5f));

      REQUIRE(stopTimes[0] == Catch::Approx(2.0f));
      REQUIRE(stopTimes[1] == Catch::Approx(3.5f));
    }
  }

  SECTION("Initialization at non-intervals path")
  {
    {
      auto io = createIO("HDF5", filename);
      io->open(FileMode::Overwrite);

      // Verify intervals group does not exist initially
      REQUIRE(io->objectExists("/intervals") == false);
      REQUIRE(io->createGroup("/intervals2") == Status::Success);

      auto timeIntervals = TimeIntervals::create("/intervals2/epochs", io);

      REQUIRE(timeIntervals != nullptr);

      Status status = timeIntervals->initialize("Test epochs");
      REQUIRE(status == Status::Success);

      // Verify intervals group was NOT created
      REQUIRE(io->objectExists("/intervals") == false);
    }
  }

  SECTION("Initialization with tags column")
  {
    {
      auto io = createIO("HDF5", filename);
      io->open(FileMode::Overwrite);

      // Verify intervals group does not exist initially
      REQUIRE(io->objectExists("/intervals") == false);

      auto timeIntervals = TimeIntervals::create("/intervals/epochs", io);

      REQUIRE(timeIntervals != nullptr);

      auto specs =
          TimeIntervals::createDefaultDataSpecs("/intervals/epochs", 100, true);
      Status status = timeIntervals->initialize("Test epochs with tags", specs);
      REQUIRE(status == Status::Success);

      // Verify intervals group was created
      REQUIRE(io->objectExists("/intervals") == true);

      // Add rows with tags
      AQNWB::Types::RowData row1 = {
          {"start_time", 1.0f},
          {"stop_time", 2.0f},
          {"tags", std::vector<std::string> {"tag1", "tag2"}}};

      AQNWB::Types::RowData row2 = {
          {"start_time", 2.5f},
          {"stop_time", 3.5f},
          {"tags", std::vector<std::string> {"tag3"}}};

      REQUIRE(timeIntervals->addRow(row1) == Status::Success);
      REQUIRE(timeIntervals->addRow(row2) == Status::Success);

      REQUIRE(timeIntervals->finalize() == Status::Success);
    }

    // Read back and verify
    {
      auto io = createIO("HDF5", filename);
      io->open(FileMode::ReadOnly);
      auto timeIntervals = TimeIntervals::create("/intervals/epochs", io);

      REQUIRE(timeIntervals != nullptr);

      auto tagsCol = timeIntervals->readTags();
      REQUIRE(tagsCol != nullptr);

      auto tagsIndexCol = timeIntervals->readTagsIndex();
      REQUIRE(tagsIndexCol != nullptr);

      auto tagsData = tagsCol->readData();
      auto tagsIndexData = tagsIndexCol->readData();

      REQUIRE(tagsData->getShape()[0] == 3);  // 3 tags total
      REQUIRE(tagsIndexData->getShape()[0] == 2);  // 2 rows

      auto tags = tagsData->values().data;

      REQUIRE(tags[0] == "tag1");
      REQUIRE(tags[1] == "tag2");
      REQUIRE(tags[2] == "tag3");

      auto tagsIndex = tagsIndexData->values().data;

      REQUIRE(tagsIndex[0] == 2);
      REQUIRE(tagsIndex[1] == 3);
    }
  }
}