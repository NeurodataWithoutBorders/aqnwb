#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/event/TimestampVectorData.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("TimestampVectorData", "[event]")
{
  SECTION(
      "test TimestampVectorData is registered as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("core::TimestampVectorData") != registry.end());
  }

  SECTION("test TimestampVectorData initialize, write, and read")
  {
    std::string path = getTestFilePath("testTimestampVectorData.h5");
    std::string dataPath = "/timestamps";
    std::string description = "Test timestamp data";
    float resolution = 1.0f / 30000.0f;
    std::vector<float> timestamps = {0.0f, 0.1f, 0.25f, 0.5f, 0.75f};
    SizeArray dataShape = {timestamps.size()};
    SizeArray chunking = {timestamps.size()};
    SizeArray positionOffset = {0};

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      IO::ArrayDataSetConfig config(BaseDataType::F32, dataShape, chunking);
      auto timestampVectorData =
          CORE::TimestampVectorData::create(dataPath, io);
      REQUIRE(timestampVectorData != nullptr);

      Status initStatus =
          timestampVectorData->initialize(resolution, description, config);
      REQUIRE(initStatus == Status::Success);

      auto dataRecorder = timestampVectorData->recordData();
      REQUIRE(dataRecorder != nullptr);

      Status writeStatus = dataRecorder->writeDataBlock(
          dataShape, positionOffset, BaseDataType::F32, timestamps.data());
      REQUIRE(writeStatus == Status::Success);
      io->close();
    }

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto readDataUntyped = NWB::RegisteredType::create(dataPath, io);
      REQUIRE(readDataUntyped != nullptr);

      auto readTimestampVectorData =
          std::dynamic_pointer_cast<CORE::TimestampVectorData>(readDataUntyped);
      REQUIRE(readTimestampVectorData != nullptr);

      REQUIRE(readTimestampVectorData->getNamespace() == "core");
      REQUIRE(readTimestampVectorData->getTypeName() == "TimestampVectorData");

      auto namespaceData = readTimestampVectorData->readNamespace();
      REQUIRE(namespaceData->values().data[0] == "core");

      auto neurodataTypeData = readTimestampVectorData->readNeurodataType();
      REQUIRE(neurodataTypeData->values().data[0] == "TimestampVectorData");

      auto descriptionData = readTimestampVectorData->readDescription();
      REQUIRE(descriptionData->values().data[0] == description);

      auto unitData = readTimestampVectorData->readUnit();
      REQUIRE(unitData->values().data[0] == "seconds");

      auto resolutionData = readTimestampVectorData->readResolution();
      REQUIRE(resolutionData->values().data[0] == Catch::Approx(resolution));

      auto timestampData = readTimestampVectorData->readData();
      auto writtenValues = timestampData->values().data;
      REQUIRE(writtenValues.size() == timestamps.size());
      for (size_t i = 0; i < writtenValues.size(); ++i) {
        REQUIRE(writtenValues[i] == Catch::Approx(timestamps[i]));
      }

      io->close();
    }
  }

  SECTION("test TimestampVectorData findOwnedTypes")
  {
    std::string path = getTestFilePath("testTimestampVectorDataOwnedTypes.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    SizeArray dataShape = {4};
    SizeArray chunking = {4};
    IO::ArrayDataSetConfig config(BaseDataType::F32, dataShape, chunking);
    auto timestampVectorData =
        CORE::TimestampVectorData::create("/timestamps", io);
    REQUIRE(timestampVectorData != nullptr);

    Status initStatus =
        timestampVectorData->initialize(0.001f, "Owned types test", config);
    REQUIRE(initStatus == Status::Success);

    auto ownedTypes = timestampVectorData->findOwnedTypes();
    REQUIRE(ownedTypes.empty());

    io->close();
  }

  SECTION("test TimestampVectorData initialize fails after IO deletion")
  {
    SizeArray dataShape = {3};
    SizeArray chunking = {3};
    IO::ArrayDataSetConfig config(BaseDataType::F32, dataShape, chunking);

    auto io =
        createIO("HDF5", getTestFilePath("testTimestampVectorDataNoIO.h5"));
    auto timestampVectorData =
        CORE::TimestampVectorData::create("/timestamps", io);
    REQUIRE(timestampVectorData != nullptr);

    io.reset();

    Status initStatus =
        timestampVectorData->initialize(0.01f, "Missing IO", config);
    REQUIRE(initStatus == Status::Failure);
  }
}
