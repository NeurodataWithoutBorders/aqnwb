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
    std::string tablePath = "/events";
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

      auto eventsTable = AQNWB::NWB::EventsTable::create(tablePath, io);
      REQUIRE(eventsTable != nullptr);

      Status initStatus =
          eventsTable->initialize(description,
                                  sourceDescription,
                                  timestampResolution,
                                  durationResolution,
                                  true,  // createAnnotationColumn
                                  100  // rowChunkSize
          );
      REQUIRE(initStatus == Status::Success);

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
      SizeArray idShape = {ids.size()};
      SizeArray idChunking = {ids.size()};
      IO::ArrayDataSetConfig idConfig(BaseDataType::I32, idShape, idChunking);
      auto elementIDs = NWB::ElementIdentifiers::create(tablePath + "/id", io);
      elementIDs->initialize(idConfig);
      Status setRowIDsStatus = eventsTable->setRowIDs(elementIDs, ids);
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

  SECTION("test EventsTable initialize fails after IO deletion")
  {
    auto io = createIO("HDF5", getTestFilePath("testEventsTableNoIO.h5"));
    auto eventsTable = AQNWB::NWB::EventsTable::create("/events", io);
    REQUIRE(eventsTable != nullptr);

    io.reset();

    Status initStatus = eventsTable->initialize("Missing IO", "", 0.01f);
    REQUIRE(initStatus == Status::Failure);
  }
}
