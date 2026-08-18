
#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/event/EventsTable.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/MeaningsTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("eventsWorkflowExamples")
{
  // -------------------------------------------------------------------------
  // Variant 1: Row-based acquisition
  // -------------------------------------------------------------------------
  SECTION("row-based event acquisition workflow")
  {
    std::string path = getTestFilePath("exampleEventsRowBased.nwb");

    // [example_events_rowbased_io_snippet]
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    // [example_events_rowbased_io_snippet]
    REQUIRE(io->isOpen());

    // [example_events_rowbased_nwbfile_snippet]
    auto nwbfile = NWB::NWBFile::create(io);
    Status initStatus = nwbfile->initialize(generateUuid());
    AQNWB::checkStatus(initStatus, "NWBFile initialization");
    // [example_events_rowbased_nwbfile_snippet]
    REQUIRE(initStatus == Status::Success);

    // [example_events_rowbased_create_table_snippet]
    // Create an EventsTable for lick events.  The table will live at
    // /events/licks inside the NWB file.  We request a timestamp column
    // (resolution 1/30000 s) and an optional annotation column; no duration
    // column is needed for this event type.
    float timestampResolution = 1.0f / 30000.0f;
    auto columnSpecs = NWB::EventsTable::createDefaultDataSpecs(
        timestampResolution,
        -1.0f,  // omit duration column
        true,  // create annotation column
        100);  // number of rows in a chunk (chunked storage is required to
               // support append when the total number of rows is not known in
               // advance)

    // Optionally add a custom column to the spec list before creating the
    // table. Here we add a float32 "confidence" column with a chunk size of 100
    // rows.
    IO::ArrayDataSetConfig confidenceConfig(
        BaseDataType::F32,  // data type
        SizeArray {0},  // initial size (0 = extensible)
        SizeArray {100});  // chunk size
    columnSpecs.push_back(NWB::VectorData::createDataSpec(
        "confidence", confidenceConfig, "Detection confidence score [0, 1]."));

    auto eventsTable = nwbfile->createEventsTable(
        "licks",  // name of the table
        "Lick events detected from the lickometer signal.",  // description
        "Thresholding of lickometer analog signal at 1.5 V",  // source
                                                              // description
        columnSpecs);  // pre-built column spec list
    // [example_events_rowbased_create_table_snippet]
    REQUIRE(eventsTable != nullptr);

    // [example_events_rowbased_start_snippet]
    Status startStatus = io->startRecording();
    // [example_events_rowbased_start_snippet]
    REQUIRE(startStatus == Status::Success);

    // [example_events_rowbased_write_snippet]
    // During acquisition, append individual rows, each
    // representing a detected event using addRow().
    // Each row is a map from column name to value.  The row ID is
    // auto-generated (0, 1, 2, …) when not supplied.
    // All columns — including custom ones — must be provided for every row.
    AQNWB::Types::RowData row0 = {{"timestamp", 0.123f},
                                  {"annotation", std::string("lick")},
                                  {"confidence", 0.95f}};
    Status s0 = eventsTable->addRow(row0);

    // We can also append multiple rows at once using addRows().
    std::vector<AQNWB::Types::RowData> moreRows = {
        {{"timestamp", 0.456f},
         {"annotation", std::string("lick")},
         {"confidence", 0.87f}},
        {{"timestamp", 0.789f},
         {"annotation", std::string("lick")},
         {"confidence", 0.91f}}};
    Status s1 = eventsTable->addRows(moreRows);

    io->flush();  // optional, flush data to disk
    // [example_events_rowbased_write_snippet]
    REQUIRE(s0 == Status::Success);
    REQUIRE(s1 == Status::Success);

    // [example_events_rowbased_stop_snippet]
    io->stopRecording();
    io->close();
    // [example_events_rowbased_stop_snippet]
  }

  // -------------------------------------------------------------------------
  // Variant 2: Column-based (bulk) acquisition
  // -------------------------------------------------------------------------
  SECTION("column-based event acquisition workflow")
  {
    std::string path = getTestFilePath("exampleEventsColumnBased.nwb");

    // [example_events_colbased_io_snippet]
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    // [example_events_colbased_io_snippet]
    REQUIRE(io->isOpen());

    // [example_events_colbased_nwbfile_snippet]
    auto nwbfile = NWB::NWBFile::create(io);
    Status initStatus = nwbfile->initialize(generateUuid());
    REQUIRE(initStatus == Status::Success);
    // [example_events_colbased_nwbfile_snippet]
    REQUIRE(initStatus == Status::Success);

    // [example_events_colbased_create_table_snippet]
    // Create an EventsTable for stimulus onset events.  Here we include both
    // a timestamp column and a duration column (resolution 1/30000 s).
    float timestampResolution = 1.0f / 30000.0f;
    float durationResolution = 1.0f / 30000.0f;
    auto columnSpecs =
        NWB::EventsTable::createDefaultDataSpecs(timestampResolution,
                                                 durationResolution,
                                                 false,  // no annotation column
                                                 100);  // row chunk size

    auto eventsTable =
        nwbfile->createEventsTable("stimulus_onsets",
                                   "Stimulus onset events for visual gratings.",
                                   "Hardware TTL pulse on channel 1",
                                   columnSpecs);
    // [example_events_colbased_create_table_snippet]
    REQUIRE(eventsTable != nullptr);

    // [example_events_colbased_addcolumn_snippet]
    // Custom columns can also be added after table creation but before
    // startRecording() using addColumn(DataSpecPtr).  This is the recommended
    // approach and is consistent with the DataSpec-based approach used by
    // initialize(). The alternative approach would be to create VectorData
    // columns directly and then adding them via addColumn().
    auto conditionSpec = NWB::VectorData::createDataSpec(
        "condition",
        IO::ArrayDataSetConfig(BaseDataType::V_STR,  // variable-length string
                               SizeArray {0},  // initial size (0 = extensible)
                               SizeArray {100}),  // chunk size
        "Stimulus condition label.");
    Status addColStatus = eventsTable->addColumn(conditionSpec);
    REQUIRE(addColStatus == Status::Success);
    // [example_events_colbased_addcolumn_snippet]

    // [example_events_colbased_start_snippet]
    Status startStatus = io->startRecording();
    // [example_events_colbased_start_snippet]
    REQUIRE(startStatus == Status::Success);

    // [example_events_colbased_write_snippet]
    // After the recording session, write all event data as full columns in a
    // single call per column.  This is efficient when the complete dataset is
    // available in memory at write time.
    std::vector<float> timestamps = {0.100f, 0.600f, 1.100f, 1.600f};
    std::vector<float> durations = {0.250f, 0.250f, 0.250f, 0.250f};
    std::vector<std::string> conditions = {
        "grating_0", "grating_90", "grating_0", "grating_90"};
    std::vector<int> rowIds = {0, 1, 2, 3};

    SizeArray dataShape = {timestamps.size()};
    SizeArray positionOffset = {0};

    // Write the timestamp column
    auto timestampColumn = eventsTable->readTimestampColumn();
    Status tsStatus = timestampColumn->recordData()->writeDataBlock(
        dataShape, positionOffset, BaseDataType::F32, timestamps.data());
    REQUIRE(tsStatus == Status::Success);

    // Write the duration column
    auto durationColumn = eventsTable->readDurationColumn();
    Status durStatus = durationColumn->recordData()->writeDataBlock(
        dataShape, positionOffset, BaseDataType::F32, durations.data());
    REQUIRE(durStatus == Status::Success);

    // Write the custom "condition" column
    auto conditionCol = eventsTable->readColumn<NWB::VectorData>("condition");
    Status condStatus = conditionCol->recordData()->writeDataBlock(
        dataShape, positionOffset, BaseDataType::V_STR, conditions);
    REQUIRE(condStatus == Status::Success);

    // Write the row IDs
    Status idStatus = eventsTable->setRowIDs(rowIds);
    REQUIRE(idStatus == Status::Success);

    io->flush();  // optional, flush data to disk
    // [example_events_colbased_write_snippet]
    REQUIRE(tsStatus == Status::Success);
    REQUIRE(durStatus == Status::Success);
    REQUIRE(condStatus == Status::Success);
    REQUIRE(idStatus == Status::Success);

    // [example_events_colbased_stop_snippet]
    io->stopRecording();
    io->close();
    // [example_events_colbased_stop_snippet]
  }

  // -------------------------------------------------------------------------
  // Variant 3: Adding a MeaningsTable for an event_type column
  // -------------------------------------------------------------------------
  SECTION("event acquisition with MeaningsTable for event_type column")
  {
    std::string path = getTestFilePath("exampleEventsMeaningsTable.nwb");

    // [example_events_meanings_io_snippet]
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    // [example_events_meanings_io_snippet]
    REQUIRE(io->isOpen());

    // [example_events_meanings_nwbfile_snippet]
    auto nwbfile = NWB::NWBFile::create(io);
    Status initStatus = nwbfile->initialize(generateUuid());
    AQNWB::checkStatus(initStatus, "NWBFile initialization");
    // [example_events_meanings_nwbfile_snippet]
    REQUIRE(initStatus == Status::Success);

    // [example_events_meanings_create_table_snippet]
    // Create an EventsTable for behavioral events that have an "event_type"
    // column storing integer codes.  We will attach a MeaningsTable to that
    // column so that readers can look up what each integer code means.
    float timestampResolution = 1.0f / 30000.0f;
    auto columnSpecs =
        NWB::EventsTable::createDefaultDataSpecs(timestampResolution,
                                                 -1.0f,  // omit duration column
                                                 false,  // no annotation column
                                                 100);  // row chunk size

    // Add an integer "event_type" column that stores event-type codes.
    IO::ArrayDataSetConfig eventTypeConfig(
        BaseDataType::I32,  // integer event-type codes
        SizeArray {0},  // initial size (0 = extensible / chunked)
        SizeArray {100});  // chunk size
    columnSpecs.push_back(NWB::VectorData::createDataSpec(
        "event_type",
        eventTypeConfig,
        "Integer code identifying the type of behavioral event."));

    auto eventsTable =
        nwbfile->createEventsTable("behavioral_events",
                                   "Behavioral events with typed event codes.",
                                   "Behavioral monitoring system",
                                   columnSpecs);
    // [example_events_meanings_create_table_snippet]
    REQUIRE(eventsTable != nullptr);

    // [example_events_meanings_create_meanings_table_snippet]
    // Before startRecording(), create a MeaningsTable for the "event_type"
    // column.  The MeaningsTable is stored at
    // "meanings_tables/event_type_meanings" inside the EventsTable group and
    // maps each integer code to a human-readable label.
    auto meaningsTable = eventsTable->createMeaningsTable("event_type");
    // [example_events_meanings_create_meanings_table_snippet]
    REQUIRE(meaningsTable != nullptr);

    // [example_events_meanings_start_snippet]
    Status startStatus = io->startRecording();
    // [example_events_meanings_start_snippet]
    REQUIRE(startStatus == Status::Success);

    // [example_events_meanings_write_events_snippet]
    // Write event rows using addRows().  Each row supplies a timestamp and the
    // integer event_type code.
    std::vector<AQNWB::Types::RowData> eventRows = {
        {{"timestamp", 0.100f}, {"event_type", 1}},
        {{"timestamp", 0.350f}, {"event_type", 2}},
        {{"timestamp", 0.700f}, {"event_type", 1}},
        {{"timestamp", 1.200f}, {"event_type", 3}},
    };
    Status eventsStatus = eventsTable->addRows(eventRows);
    // [example_events_meanings_write_events_snippet]
    REQUIRE(eventsStatus == Status::Success);

    // [example_events_meanings_write_meanings_snippet]
    // Populate the MeaningsTable: list every possible event_type code together
    // with its human-readable meaning.  All possible values should be present
    // even if they do not appear in the recorded data.
    std::vector<AQNWB::Types::RowData> meaningsRows = {
        {{"value", 1}, {"meaning", "lick"}},
        {{"value", 2}, {"meaning", "reward_delivery"}},
        {{"value", 3}, {"meaning", "air_puff"}},
    };
    Status meaningsStatus = meaningsTable->addRows(meaningsRows);
    io->flush();  // optional, flush data to disk
    // [example_events_meanings_write_meanings_snippet]
    REQUIRE(meaningsStatus == Status::Success);

    // [example_events_meanings_stop_snippet]
    io->stopRecording();
    io->close();
    // [example_events_meanings_stop_snippet]
  }
}
