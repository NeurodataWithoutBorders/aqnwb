#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/event/DurationVectorData.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("DurationVectorData", "[event]")
{
  SECTION(
      "test DurationVectorData is registered as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("core::DurationVectorData") != registry.end());
  }

  SECTION("test DurationVectorData initialize, write, and read")
  {
    std::string path = getTestFilePath("testDurationVectorData.h5");
    std::string dataPath = "/durations";
    std::string description = "Test duration data";
    float resolution = 1.0f / 30000.0f;
    std::vector<float> durations = {0.0f, 0.05f, 0.25f, 0.5f, 1.25f};
    SizeArray dataShape = {durations.size()};
    SizeArray chunking = {durations.size()};
    SizeArray positionOffset = {0};

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      IO::ArrayDataSetConfig config(BaseDataType::F32, dataShape, chunking);
      auto durationVectorData =
          AQNWB::NWB::DurationVectorData::create(dataPath, io);
      REQUIRE(durationVectorData != nullptr);

      Status initStatus =
          durationVectorData->initialize(config, description, resolution);
      REQUIRE(initStatus == Status::Success);

      auto dataRecorder = durationVectorData->recordData();
      REQUIRE(dataRecorder != nullptr);

      Status writeStatus = dataRecorder->writeDataBlock(
          dataShape, positionOffset, BaseDataType::F32, durations.data());
      REQUIRE(writeStatus == Status::Success);
      io->close();
    }

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto readDataUntyped = NWB::RegisteredType::create(dataPath, io);
      REQUIRE(readDataUntyped != nullptr);

      auto readDurationVectorData =
          std::dynamic_pointer_cast<AQNWB::NWB::DurationVectorData>(
              readDataUntyped);
      REQUIRE(readDurationVectorData != nullptr);

      REQUIRE(readDurationVectorData->getNamespace() == "core");
      REQUIRE(readDurationVectorData->getTypeName() == "DurationVectorData");

      auto namespaceData = readDurationVectorData->readNamespace();
      REQUIRE(namespaceData->values().data[0] == "core");

      auto neurodataTypeData = readDurationVectorData->readNeurodataType();
      REQUIRE(neurodataTypeData->values().data[0] == "DurationVectorData");

      auto descriptionData = readDurationVectorData->readDescription();
      REQUIRE(descriptionData->values().data[0] == description);

      auto unitData = readDurationVectorData->readUnit();
      REQUIRE(unitData->values().data[0] == "seconds");

      auto resolutionData = readDurationVectorData->readResolution();
      REQUIRE(resolutionData->values().data[0] == Catch::Approx(resolution));

      auto durationData = readDurationVectorData->readData();
      auto writtenValues = durationData->values().data;
      REQUIRE(writtenValues.size() == durations.size());
      for (size_t i = 0; i < writtenValues.size(); ++i) {
        REQUIRE(writtenValues[i] == Catch::Approx(durations[i]));
      }

      io->close();
    }
  }

  SECTION("test DurationVectorData findOwnedTypes")
  {
    std::string path = getTestFilePath("testDurationVectorDataOwnedTypes.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    SizeArray dataShape = {4};
    SizeArray chunking = {4};
    IO::ArrayDataSetConfig config(BaseDataType::F32, dataShape, chunking);
    auto durationVectorData =
        AQNWB::NWB::DurationVectorData::create("/durations", io);
    REQUIRE(durationVectorData != nullptr);

    Status initStatus =
        durationVectorData->initialize(config, "Owned types test", 0.001f);
    REQUIRE(initStatus == Status::Success);

    auto ownedTypes = durationVectorData->findOwnedTypes();
    REQUIRE(ownedTypes.empty());

    io->close();
  }

  SECTION("test DurationVectorData initialize fails after IO deletion")
  {
    SizeArray dataShape = {3};
    SizeArray chunking = {3};
    IO::ArrayDataSetConfig config(BaseDataType::F32, dataShape, chunking);

    auto io =
        createIO("HDF5", getTestFilePath("testDurationVectorDataNoIO.h5"));
    auto durationVectorData =
        AQNWB::NWB::DurationVectorData::create("/durations", io);
    REQUIRE(durationVectorData != nullptr);

    io.reset();

    Status initStatus =
        durationVectorData->initialize(config, "Missing IO", 0.01f);
    REQUIRE(initStatus == Status::Failure);
  }
}
