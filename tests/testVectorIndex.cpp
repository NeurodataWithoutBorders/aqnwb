#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "nwb/hdmf/table/VectorIndex.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("VectorIndex", "[table]")
{
  SECTION("is registered as a subclass of RegisteredType")
  {
    auto registry = NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("hdmf-common::VectorIndex") != registry.end());
  }

  SECTION("initializes, writes, and reads its target")
  {
    const std::string path = getTestFilePath("testVectorIndex.h5");
    const std::string targetPath = "/ragged_values";
    const std::string indexPath = targetPath + "_index";
    const std::string targetDescription = "Ragged array values";
    const std::string indexDescription = "End indices for ragged array rows";
    const std::vector<int> targetValues = {10, 11, 20, 21, 22, 30};
    const std::vector<uint32_t> indices = {2, 5, 6};

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto target = NWB::VectorData::create(targetPath, io);
      REQUIRE(target != nullptr);
      REQUIRE(target->initialize(IO::ArrayDataSetConfig(BaseDataType::I32,
                                                        {targetValues.size()},
                                                        {targetValues.size()}),
                                 targetDescription)
              == Status::Success);

      auto vectorIndex = NWB::VectorIndex::create(indexPath, io);
      REQUIRE(vectorIndex != nullptr);
      REQUIRE(vectorIndex->initialize(
                  IO::ArrayDataSetConfig(
                      BaseDataType::U32, {indices.size()}, {indices.size()}),
                  indexDescription,
                  targetPath)
              == Status::Success);

      auto targetRecorder = target->recordData();
      auto indexRecorder = vectorIndex->recordData();
      REQUIRE(targetRecorder != nullptr);
      REQUIRE(indexRecorder != nullptr);

      REQUIRE(io->startRecording() == Status::Success);
      REQUIRE(targetRecorder->writeDataBlock({targetValues.size()},
                                             {0},
                                             BaseDataType::I32,
                                             targetValues.data())
              == Status::Success);
      REQUIRE(indexRecorder->writeDataBlock(
                  {indices.size()}, {0}, BaseDataType::U32, indices.data())
              == Status::Success);
      REQUIRE(io->stopRecording() == Status::Success);
      io->close();
    }

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();

      auto readData = NWB::RegisteredType::create(indexPath, io);
      auto vectorIndex = std::dynamic_pointer_cast<NWB::VectorIndex>(readData);
      REQUIRE(vectorIndex != nullptr);
      REQUIRE(vectorIndex->getNamespace() == "hdmf-common");
      REQUIRE(vectorIndex->getTypeName() == "VectorIndex");
      REQUIRE(vectorIndex->readDescription()->values().data[0]
              == indexDescription);
      REQUIRE(vectorIndex->readData()->values().data == indices);

      auto target = vectorIndex->readTarget();
      REQUIRE(target != nullptr);
      REQUIRE(target->getPath() == targetPath);
      REQUIRE(target->readDescription()->values().data[0] == targetDescription);
      auto typedTarget = NWB::VectorDataTyped<int>::fromVectorData(target);
      REQUIRE(typedTarget != nullptr);
      REQUIRE(typedTarget->readData()->values().data == targetValues);

      io->close();
    }
  }

  SECTION("rejects non-unsigned and non-one-dimensional configurations")
  {
    const std::string path = getTestFilePath("testVectorIndexValidation.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto vectorIndex = NWB::VectorIndex::create("/index", io);
    REQUIRE(vectorIndex != nullptr);

    REQUIRE_THROWS_AS(vectorIndex->initialize(
                          IO::ArrayDataSetConfig(BaseDataType::I32, {3}, {3}),
                          "Invalid index type",
                          "/target"),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(
        vectorIndex->initialize(
            IO::ArrayDataSetConfig(BaseDataType::U32, {2, 2}, {2, 2}),
            "Invalid index dimensions",
            "/target"),
        std::invalid_argument);

    io->close();
  }

  SECTION("test appendData")
  {
    const std::string path = getTestFilePath("testVectorIndexAppendData.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    const std::string targetPath = "/ragged_values";
    const std::string indexPath = targetPath + "_index";

    auto target = NWB::VectorData::create(targetPath, io);
    target->initialize(IO::ArrayDataSetConfig(BaseDataType::I32, {0}, {10}),
                       "Target");

    auto vectorIndex = NWB::VectorIndex::create(indexPath, io);
    vectorIndex->initialize(
        IO::ArrayDataSetConfig(BaseDataType::U32, {0}, {10}),
        "Index",
        targetPath);
    vectorIndex->setTargetColumn(target);

    std::vector<int> vec1 = {1, 2};
    std::vector<int> vec2 = {3, 4, 5};
    std::vector<int> vec3 = {6};

    size_t elementsAppended = 0;
    REQUIRE(vectorIndex->appendData(vec1, elementsAppended) == Status::Success);
    REQUIRE(elementsAppended == 2);
    REQUIRE(vectorIndex->appendData(vec2, elementsAppended) == Status::Success);
    REQUIRE(elementsAppended == 3);
    REQUIRE(vectorIndex->appendData(vec3, elementsAppended) == Status::Success);
    REQUIRE(elementsAppended == 1);

    io->flush();

    auto readData = NWB::RegisteredType::create(indexPath, io);
    auto readVectorIndex =
        std::dynamic_pointer_cast<NWB::VectorIndex>(readData);
    REQUIRE(readVectorIndex != nullptr);

    auto indices = readVectorIndex->readData()->values().data;
    REQUIRE(indices.size() == 3);
    REQUIRE(indices[0] == 2);
    REQUIRE(indices[1] == 5);
    REQUIRE(indices[2] == 6);

    io->close();
  }
}
