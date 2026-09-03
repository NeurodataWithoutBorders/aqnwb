#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/epoch/TimeIntervals.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("timeIntervalsWorkflowExamples")
{
  // -------------------------------------------------------------------------
  // Variant 1: Row-based acquisition
  // -------------------------------------------------------------------------
  SECTION("row-based time intervals acquisition workflow")
  {
    std::string path = getTestFilePath("exampleTimeIntervalsRowBased.nwb");

    // [example_timeintervals_rowbased_io_snippet]
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    // [example_timeintervals_rowbased_io_snippet]
    REQUIRE(io->isOpen());

    // [example_timeintervals_rowbased_nwbfile_snippet]
    auto nwbfile = NWB::NWBFile::create(io);
    auto subjectSpec =
        AQNWB::NWB::Subject::SubjectSpec()
            .withSubjectId("mouse001")
            .withSpecies("Mus musculus")
            .withSex("M")
            .withAge("P90D")
            .withDescription(
                "Wild type mouse used for electrophysiology study");
    std::string currentTime = getCurrentTime();
    Status initStatus = nwbfile->initialize(generateUuid(),
                                            "a recording session",
                                            "data collection info",
                                            currentTime,
                                            currentTime,
                                            subjectSpec);
    AQNWB::checkStatus(initStatus, "NWBFile initialization");
    // [example_timeintervals_rowbased_nwbfile_snippet]
    REQUIRE(initStatus == Status::Success);

    // [example_timeintervals_rowbased_create_table_snippet]
    // Create a TimeIntervals table for trials. The table will live at
    // /intervals/trials inside the NWB file. We request the optional tags
    // column.
    auto columnSpecs = NWB::TimeIntervals::createDefaultDataSpecs(
        NWB::NWBFile::INTERVALS_PATH + "/trials",
        100,  // number of rows in a chunk (chunked storage is required to
              // support append when the total number of rows is not known in
              // advance)
        true);  // create tags column

    // Optionally add a custom column to the spec list before creating the
    // table. Here we add a string "condition" column with a chunk size of 100
    // rows.
    IO::ArrayDataSetConfig conditionConfig(
        BaseDataType::V_STR,  // data type
        SizeArray {0},  // initial size (0 = extensible)
        SizeArray {100});  // chunk size
    columnSpecs.push_back(NWB::VectorData::createDataSpec(
        "condition", conditionConfig, "Trial condition label."));

    auto trialsTable = nwbfile->createTimeIntervals(
        "trials",  // name of the table
        "Experimental trials.",  // description
        columnSpecs);  // pre-built column spec list
    // [example_timeintervals_rowbased_create_table_snippet]
    REQUIRE(trialsTable != nullptr);

    // [example_timeintervals_rowbased_start_snippet]
    Status startStatus = io->startRecording();
    // [example_timeintervals_rowbased_start_snippet]
    REQUIRE(startStatus == Status::Success);

    // [example_timeintervals_rowbased_write_snippet]
    // During acquisition, append individual rows, each
    // representing a detected interval using addRow().
    // Each row is a map from column name to value.  The row ID is
    // auto-generated (0, 1, 2, …) when not supplied.
    // All columns — including custom ones — must be provided for every row.
    AQNWB::Types::RowData row0 = {
        {"start_time", 0.123f},
        {"stop_time", 1.456f},
        {"tags", std::vector<std::string> {"correct", "fast"}},
        {"condition", std::string("visual_stimulus")}};
    Status s0 = trialsTable->addRow(row0);

    // We can also append multiple rows at once using addRows().
    std::vector<AQNWB::Types::RowData> moreRows = {
        {{"start_time", 2.123f},
         {"stop_time", 3.456f},
         {"tags", std::vector<std::string> {"incorrect"}},
         {"condition", std::string("auditory_stimulus")}},
        {{"start_time", 4.123f},
         {"stop_time", 5.456f},
         {"tags", std::vector<std::string> {"correct", "slow"}},
         {"condition", std::string("visual_stimulus")}}};
    Status s1 = trialsTable->addRows(moreRows);

    io->flush();  // optional, flush data to disk
    // [example_timeintervals_rowbased_write_snippet]
    REQUIRE(s0 == Status::Success);
    REQUIRE(s1 == Status::Success);

    // [example_timeintervals_rowbased_stop_snippet]
    io->stopRecording();
    io->close();
    // [example_timeintervals_rowbased_stop_snippet]
  }

  // -------------------------------------------------------------------------
  // Variant 2: Column-based (bulk) acquisition
  // -------------------------------------------------------------------------
  SECTION("column-based time intervals acquisition workflow")
  {
    std::string path = getTestFilePath("exampleTimeIntervalsColumnBased.nwb");

    // [example_timeintervals_colbased_io_snippet]
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    // [example_timeintervals_colbased_io_snippet]
    REQUIRE(io->isOpen());

    // [example_timeintervals_colbased_nwbfile_snippet]
    auto nwbfile = NWB::NWBFile::create(io);
    auto subjectSpec =
        AQNWB::NWB::Subject::SubjectSpec()
            .withSubjectId("mouse001")
            .withSpecies("Mus musculus")
            .withSex("M")
            .withAge("P90D")
            .withDescription(
                "Wild type mouse used for electrophysiology study");
    std::string currentTime = getCurrentTime();
    Status initStatus = nwbfile->initialize(generateUuid(),
                                            "a recording session",
                                            "data collection info",
                                            currentTime,
                                            currentTime,
                                            subjectSpec);
    AQNWB::checkStatus(initStatus, "NWBFile initialization");
    // [example_timeintervals_colbased_nwbfile_snippet]
    REQUIRE(initStatus == Status::Success);

    // [example_timeintervals_colbased_create_table_snippet]
    // Create a TimeIntervals table for epochs.
    auto columnSpecs = NWB::TimeIntervals::createDefaultDataSpecs(
        NWB::NWBFile::INTERVALS_PATH + "/epochs",
        100,  // row chunk size
        false);  // no tags column

    auto epochsTable = nwbfile->createTimeIntervals(
        "epochs", "Experimental epochs.", columnSpecs);
    // [example_timeintervals_colbased_create_table_snippet]
    REQUIRE(epochsTable != nullptr);

    // [example_timeintervals_colbased_addcolumn_snippet]
    // Custom columns can also be added after table creation but before
    // startRecording() using addColumn(DataSpecPtr).  This is the recommended
    // approach and is consistent with the DataSpec-based approach used by
    // initialize(). The alternative approach would be to create VectorData
    // columns directly and then adding them via addColumn().
    auto labelSpec = NWB::VectorData::createDataSpec(
        "label",
        IO::ArrayDataSetConfig(BaseDataType::V_STR,  // variable-length string
                               SizeArray {0},  // initial size (0 = extensible)
                               SizeArray {100}),  // chunk size
        "Epoch label.");
    Status addColStatus = epochsTable->addColumn(labelSpec);
    REQUIRE(addColStatus == Status::Success);
    // [example_timeintervals_colbased_addcolumn_snippet]

    // [example_timeintervals_colbased_start_snippet]
    Status startStatus = io->startRecording();
    // [example_timeintervals_colbased_start_snippet]
    REQUIRE(startStatus == Status::Success);

    // [example_timeintervals_colbased_write_snippet]
    // After the recording session, write all interval data as full columns in a
    // single call per column.  This is efficient when the complete dataset is
    // available in memory at write time.
    std::vector<float> startTimes = {0.100f, 10.600f, 20.100f};
    std::vector<float> stopTimes = {10.000f, 20.000f, 30.000f};
    std::vector<std::string> labels = {"baseline", "stimulation", "recovery"};
    std::vector<int> rowIds = {0, 1, 2};

    SizeArray dataShape = {startTimes.size()};
    SizeArray positionOffset = {0};

    // Write the start_time column
    auto startTimeColumn = epochsTable->readStartTime();
    Status startStatusWrite = startTimeColumn->recordData()->writeDataBlock(
        dataShape, positionOffset, BaseDataType::F32, startTimes.data());
    REQUIRE(startStatusWrite == Status::Success);

    // Write the stop_time column
    auto stopTimeColumn = epochsTable->readStopTime();
    Status stopStatusWrite = stopTimeColumn->recordData()->writeDataBlock(
        dataShape, positionOffset, BaseDataType::F32, stopTimes.data());
    REQUIRE(stopStatusWrite == Status::Success);

    // Write the custom "label" column
    auto labelCol = epochsTable->readColumn<NWB::VectorData>("label");
    Status labelStatusWrite = labelCol->recordData()->writeDataBlock(
        dataShape, positionOffset, BaseDataType::V_STR, labels);
    REQUIRE(labelStatusWrite == Status::Success);

    // Write the row IDs
    Status idStatus = epochsTable->setRowIDs(rowIds);
    REQUIRE(idStatus == Status::Success);

    io->flush();  // optional, flush data to disk
    // [example_timeintervals_colbased_write_snippet]
    REQUIRE(startStatusWrite == Status::Success);
    REQUIRE(stopStatusWrite == Status::Success);
    REQUIRE(labelStatusWrite == Status::Success);
    REQUIRE(idStatus == Status::Success);

    // [example_timeintervals_colbased_stop_snippet]
    io->stopRecording();
    io->close();
    // [example_timeintervals_colbased_stop_snippet]
  }
}