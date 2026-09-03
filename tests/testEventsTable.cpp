#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/event/DurationVectorData.hpp"
#include "nwb/event/EventsTable.hpp"
#include "nwb/event/TimestampVectorData.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("EventsTable", "[event]")
{
  SECTION("test EventsTable is registered as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("core::EventsTable") != registry.end());
  }

  SECTION("test EventsTable initialize, write, and read")
  {
    std::string path = getTestFilePath("testEventsTable.h5");
    std::string tablePath = "/events/test_events";
    std::string description = "Test events table";
    std::string sourceDescription = "Test source description";
    float timestampResolution = 1.0f / 30000.0f;
    float durationResolution = 1.0f / 30000.0f;

    std::vector<float> timestamps = {0.0f, 0.1f, 0.25f, 0.5f, 0.75f};
    std::vector<float> durations = {0.01f, 0.02f, 0.03f, 0.04f, 0.05f};
    std::vector<std::string> annotations = {"a", "b", "c", "d", "e"};
    std::vector<int> ids = {1, 2, 3, 4, 5};

    SizeArray dataShape = {timestamps.size()};
    SizeArray positionOffset = {0};

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      // Verify events group does not exist initially
      REQUIRE(io->objectExists("/events") == false);

      auto eventsTable = AQNWB::NWB::EventsTable::create(tablePath, io);
      REQUIRE(eventsTable != nullptr);

      auto specs = NWB::EventsTable::createDefaultDataSpecs(
          timestampResolution, true, durationResolution, true, 100);
      Status initStatus =
          eventsTable->initialize(description, sourceDescription, specs);
      REQUIRE(initStatus == Status::Success);

      // Verify events group was created
      REQUIRE(io->objectExists("/events") == true);

      // Write timestamps
      auto timestampColumn = eventsTable->readTimestampColumn();
      REQUIRE(timestampColumn != nullptr);
      Status writeTimestampStatus =
          timestampColumn->recordData()->writeDataBlock(
              dataShape, positionOffset, BaseDataType::F32, timestamps.data());
      REQUIRE(writeTimestampStatus == Status::Success);

      // Write durations
      auto durationColumn = eventsTable->readDurationColumn();
      REQUIRE(durationColumn != nullptr);
      Status writeDurationStatus = durationColumn->recordData()->writeDataBlock(
          dataShape, positionOffset, BaseDataType::F32, durations.data());
      REQUIRE(writeDurationStatus == Status::Success);

      // Write annotations
      auto annotationColumn = eventsTable->readAnnotationColumn();
      REQUIRE(annotationColumn != nullptr);
      Status writeAnnotationStatus =
          annotationColumn->recordData()->writeDataBlock(
              dataShape, positionOffset, BaseDataType::V_STR, annotations);
      REQUIRE(writeAnnotationStatus == Status::Success);

      // Set row IDs
      Status setRowIDsStatus = eventsTable->setRowIDs(ids);
      REQUIRE(setRowIDsStatus == Status::Success);

      Status finalizeStatus = eventsTable->finalize();
      REQUIRE(finalizeStatus == Status::Success);

      io->close();
    }

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto readDataUntyped = NWB::RegisteredType::create(tablePath, io);
      REQUIRE(readDataUntyped != nullptr);

      auto readEventsTable =
          std::dynamic_pointer_cast<AQNWB::NWB::EventsTable>(readDataUntyped);
      REQUIRE(readEventsTable != nullptr);

      REQUIRE(readEventsTable->getNamespace() == "core");
      REQUIRE(readEventsTable->getTypeName() == "EventsTable");

      auto descriptionData = readEventsTable->readDescription();
      REQUIRE(descriptionData->values().data[0] == description);

      auto sourceDescriptionData = readEventsTable->readSourceDescription();
      REQUIRE(sourceDescriptionData->values().data[0] == sourceDescription);

      // readRows resolves and caches columns before the typed accessors below.
      const auto rows = readEventsTable->readRows();
      REQUIRE(rows.size() == timestamps.size());

      // Read timestamps
      auto readTimestampColumn = readEventsTable->readTimestampColumn();
      REQUIRE(readTimestampColumn != nullptr);
      auto readTimestampValues = readTimestampColumn->readData()->values().data;
      REQUIRE(readTimestampValues.size() == timestamps.size());
      for (size_t i = 0; i < readTimestampValues.size(); ++i) {
        REQUIRE(readTimestampValues[i] == Catch::Approx(timestamps[i]));
      }

      // Read durations
      auto readDurationColumn = readEventsTable->readDurationColumn();
      REQUIRE(readDurationColumn != nullptr);
      auto readDurationValues = readDurationColumn->readData()->values().data;
      REQUIRE(readDurationValues.size() == durations.size());
      for (size_t i = 0; i < readDurationValues.size(); ++i) {
        REQUIRE(readDurationValues[i] == Catch::Approx(durations[i]));
      }

      // Read annotations
      auto readAnnotationColumn = readEventsTable->readAnnotationColumn();
      REQUIRE(readAnnotationColumn != nullptr);
      auto readAnnotationColumnTyped =
          AQNWB::NWB::VectorDataTyped<std::string>::fromVectorData(
              readAnnotationColumn);
      REQUIRE(readAnnotationColumnTyped != nullptr);
      auto readAnnotationValues =
          readAnnotationColumnTyped->readData()->values().data;
      REQUIRE(readAnnotationValues.size() == annotations.size());
      for (size_t i = 0; i < readAnnotationValues.size(); ++i) {
        REQUIRE(readAnnotationValues[i] == annotations[i]);
      }

      // Read row IDs
      auto readIdsData =
          readEventsTable->readIdColumn()->readData()->values().data;
      REQUIRE(readIdsData == ids);

      io->close();
    }
  }

  SECTION("test EventsTable initialize with optional parameters")
  {
    std::string path = getTestFilePath("testEventsTableOptional.h5");
    std::string tablePath = "/events/test_events_optional";
    std::string description = "Test events table optional";

    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto eventsTable = AQNWB::NWB::EventsTable::create(tablePath, io);
    REQUIRE(eventsTable != nullptr);

    // Use std::nullopt for resolutions and sourceDescription
    auto specs = NWB::EventsTable::createDefaultDataSpecs(
        std::nullopt, false, std::nullopt, false, 100);
    Status initStatus =
        eventsTable->initialize(description, std::nullopt, specs);
    REQUIRE(initStatus == Status::Success);

    // Verify timestamp column exists but has no resolution attribute
    auto timestampColumn = eventsTable->readTimestampColumn();
    REQUIRE(timestampColumn != nullptr);
    auto timestampRes = timestampColumn->readResolution();
    REQUIRE(timestampRes->exists() == false);  // Should not exist

    // Verify duration column does not exist
    auto durationColumn = eventsTable->readDurationColumn();
    REQUIRE(durationColumn == nullptr);

    // Verify source_description attribute does not exist
    auto sourceDesc = eventsTable->readSourceDescription();
    REQUIRE(sourceDesc->exists() == false);

    io->close();
  }

  SECTION("test EventsTable initialize at non-events path")
  {
    std::string path = getTestFilePath("testEventsTableNonEventsPath.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // Verify events group does not exist initially
    REQUIRE(io->objectExists("/events") == false);
    REQUIRE(io->createGroup("/events2") == Status::Success);

    auto eventsTable =
        AQNWB::NWB::EventsTable::create("/events2/test_events", io);
    REQUIRE(eventsTable != nullptr);

    auto specs =
        NWB::EventsTable::createDefaultDataSpecs(0.01f, true, 0.01f, true, 100);
    Status initStatus = eventsTable->initialize(
        "Test events table", "Test source description", specs);
    REQUIRE(initStatus == Status::Success);

    // Verify events group was NOT created
    REQUIRE(io->objectExists("/events") == false);

    io->close();
  }

  SECTION("test EventsTable initialize fails after IO deletion")
  {
    auto io = createIO("HDF5", getTestFilePath("testEventsTableNoIO.h5"));
    auto eventsTable =
        AQNWB::NWB::EventsTable::create("/events/test_events", io);
    REQUIRE(eventsTable != nullptr);

    io.reset();

    auto specs =
        NWB::EventsTable::createDefaultDataSpecs(0.01f, true, 0.01f, true, 100);
    Status initStatus = eventsTable->initialize("Missing IO", "", specs);
    REQUIRE(initStatus == Status::Failure);
  }

  SECTION("test EventsTable validation")
  {
    std::string path = getTestFilePath("testEventsTableValidation.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto eventsTable =
        AQNWB::NWB::EventsTable::create("/events/test_events", io);

    // 1. Valid default specs contain a TimestampVectorData timestamp column.
    std::vector<NWB::DynamicTable::DataSpecPtr> validSpecs =
        NWB::EventsTable::createDefaultDataSpecs();
    REQUIRE(eventsTable->validateDataSpecs(validSpecs) == Status::Success);

    // 2. Timestamp must have the expected DataSpec type, shape, and dtype.
    auto wrongTimestampSpecType = validSpecs;
    wrongTimestampSpecType[1] = NWB::VectorData::createDataSpec(
        "timestamp",
        IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}),
        "timestamp");
    auto wrongTimestampShape = validSpecs;
    wrongTimestampShape[1] = NWB::TimestampVectorData::createDataSpec(
        "timestamp",
        IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0, 1}, {1, 1}),
        "timestamp");
    auto wrongTimestampDataType = validSpecs;
    wrongTimestampDataType[1] = NWB::TimestampVectorData::createDataSpec(
        "timestamp",
        IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10}),
        "timestamp");

    REQUIRE(eventsTable->validateDataSpecs(wrongTimestampSpecType)
            == Status::Failure);
    REQUIRE(eventsTable->validateDataSpecs(wrongTimestampShape)
            == Status::Failure);
    REQUIRE(eventsTable->validateDataSpecs(wrongTimestampDataType)
            == Status::Failure);

    REQUIRE_THROWS_AS(
        eventsTable->initialize("Test Events", "", wrongTimestampSpecType),
        std::invalid_argument);
    REQUIRE_THROWS_AS(
        eventsTable->initialize("Test Events", "", wrongTimestampShape),
        std::invalid_argument);
    REQUIRE_THROWS_AS(
        eventsTable->initialize("Test Events", "", wrongTimestampDataType),
        std::invalid_argument);

    io->close();
  }

  SECTION("test EventsTable initialize with defaulted columnSpecs")
  {
    std::string path = getTestFilePath("testEventsTableDefaultedSpecs.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto eventsTable =
        AQNWB::NWB::EventsTable::create("/events/test_events", io);
    REQUIRE(eventsTable != nullptr);

    // Call initialize with only description, allowing columnSpecs to default
    Status initStatus = eventsTable->initialize("Test events");
    REQUIRE(initStatus == Status::Success);

    // Verify timestamp column was created
    auto timestampColumn = eventsTable->readTimestampColumn();
    REQUIRE(timestampColumn != nullptr);

    io->close();
  }
}
