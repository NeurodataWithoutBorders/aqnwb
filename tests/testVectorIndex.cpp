#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
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

  SECTION("DataSpec initialization")
  {
    const std::string path = getTestFilePath("testVectorIndexDataSpec.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto target = NWB::VectorData::create("/target", io);
    target->initialize(IO::ArrayDataSetConfig(BaseDataType::I32, {0}, {10}),
                       "Target");

    auto vectorIndex = NWB::VectorIndex::create("/index", io);
    NWB::VectorIndex::DataSpec spec(
        "index",
        IO::ArrayDataSetConfig(BaseDataType::U32, {0}, {10}),
        "Index desc",
        "/target");

    REQUIRE(spec.initialize(*vectorIndex) == Status::Success);
    REQUIRE(vectorIndex->readDescription()->values().data[0] == "Index desc");

    // Test failure with wrong type
    auto wrongData = NWB::VectorData::create("/wrong", io);
    REQUIRE(spec.initialize(*wrongData) == Status::Failure);

    io->close();
  }

  SECTION("getTargetColumn lazy loading")
  {
    const std::string path = getTestFilePath("testVectorIndexLazyLoad.h5");
    const std::string targetPath = "/target";
    const std::string indexPath = "/index";

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();
      auto target = NWB::VectorData::create(targetPath, io);
      target->initialize(IO::ArrayDataSetConfig(BaseDataType::I32, {0}, {10}),
                         "Target");
      auto vectorIndex = NWB::VectorIndex::create(indexPath, io);
      Status status = vectorIndex->initialize(
          IO::ArrayDataSetConfig(BaseDataType::U32, {0}, {10}),
          "Index",
          targetPath);
      REQUIRE(status == Status::Success);
      io->close();
    }

    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();
      auto readData = NWB::RegisteredType::create(indexPath, io);
      auto vectorIndex = std::dynamic_pointer_cast<NWB::VectorIndex>(readData);
      REQUIRE(vectorIndex != nullptr);

      // m_targetColumn should be null initially, getTargetColumn should read it
      auto target = vectorIndex->getTargetColumn();
      REQUIRE(target != nullptr);
      REQUIRE(target->getPath() == targetPath);

      // Second call should return cached
      auto target2 = vectorIndex->getTargetColumn();
      REQUIRE(target == target2);

      io->close();
    }
  }

  SECTION("readIndexedCellValues")
  {
    const std::string path = getTestFilePath("testVectorIndexReadIndexed.h5");
    const std::string targetPath = "/target";
    const std::string indexPath = "/index";

    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

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
    std::vector<int> vec2 = {};  // Empty vector
    std::vector<int> vec3 = {3, 4, 5};
    std::vector<int> vec4 = {6};

    size_t elementsAppended = 0;
    vectorIndex->appendData(vec1, elementsAppended);
    vectorIndex->appendData(vec2, elementsAppended);
    vectorIndex->appendData(vec3, elementsAppended);
    vectorIndex->appendData(vec4, elementsAppended);

    io->flush();

    // Read all
    auto allCells = vectorIndex->readIndexedCellValues();
    REQUIRE(allCells.size() == 4);
    REQUIRE(allCells[0].get<std::vector<int>>() == vec1);
    REQUIRE(allCells[1].get<std::vector<uint8_t>>().empty());
    REQUIRE(allCells[2].get<std::vector<int>>() == vec3);
    REQUIRE(allCells[3].get<std::vector<int>>() == vec4);

    // Read slice (start=1, count=2)
    auto sliceCells = vectorIndex->readIndexedCellValues(1, 2);
    REQUIRE(sliceCells.size() == 2);
    REQUIRE(sliceCells[0].get<std::vector<uint8_t>>().empty());
    REQUIRE(sliceCells[1].get<std::vector<int>>() == vec3);

    // Read out of bounds
    REQUIRE_THROWS(vectorIndex->readIndexedCellValues(10, 2));

    io->close();
  }

  SECTION("appendData and initializeAppendState with different data types")
  {
    std::vector<BaseDataType::Type> types = {BaseDataType::Type::T_U8,
                                             BaseDataType::Type::T_U16,
                                             BaseDataType::Type::T_U32,
                                             BaseDataType::Type::T_U64};

    std::vector<int> vec1 = {1, 2};
    std::vector<int> vec2 = {3, 4, 5};

    for (auto type : types) {
      const std::string path =
          getTestFilePath("testVectorIndexTypes_"
                          + std::to_string(static_cast<int>(type)) + ".h5");
      const std::string targetPath = "/target";
      const std::string indexPath = "/index";

      {
        std::shared_ptr<BaseIO> io = createIO("HDF5", path);
        io->open();

        auto target = NWB::VectorData::create(targetPath, io);
        target->initialize(IO::ArrayDataSetConfig(BaseDataType::I32, {0}, {10}),
                           "Target");

        auto vectorIndex = NWB::VectorIndex::create(indexPath, io);
        vectorIndex->initialize(
            IO::ArrayDataSetConfig(BaseDataType(type), {0}, {10}),
            "Index",
            targetPath);
        vectorIndex->setTargetColumn(target);

        size_t elementsAppended = 0;
        REQUIRE(vectorIndex->appendData(vec1, elementsAppended)
                == Status::Success);
        REQUIRE(elementsAppended == 2);

        io->close();
      }

      // Re-open and append more to test initializeAppendState
      {
        std::shared_ptr<BaseIO> io = createIO("HDF5", path);
        io->open();

        auto readData = NWB::RegisteredType::create(indexPath, io);
        auto vectorIndex =
            std::dynamic_pointer_cast<NWB::VectorIndex>(readData);
        REQUIRE(vectorIndex != nullptr);

        size_t elementsAppended = 0;
        // This will trigger initializeAppendState
        REQUIRE(vectorIndex->appendData(vec2, elementsAppended)
                == Status::Success);
        REQUIRE(elementsAppended == 3);

        io->flush();

        // Verify indices
        if (type == BaseDataType::Type::T_U8) {
          auto indices = vectorIndex->readData<uint8_t>()->values().data;
          REQUIRE(indices.size() == 2);
          REQUIRE(indices[0] == 2);
          REQUIRE(indices[1] == 5);
        } else if (type == BaseDataType::Type::T_U16) {
          auto indices = vectorIndex->readData<uint16_t>()->values().data;
          REQUIRE(indices.size() == 2);
          REQUIRE(indices[0] == 2);
          REQUIRE(indices[1] == 5);
        } else if (type == BaseDataType::Type::T_U32) {
          auto indices = vectorIndex->readData<uint32_t>()->values().data;
          REQUIRE(indices.size() == 2);
          REQUIRE(indices[0] == 2);
          REQUIRE(indices[1] == 5);
        } else if (type == BaseDataType::Type::T_U64) {
          auto indices = vectorIndex->readData<uint64_t>()->values().data;
          REQUIRE(indices.size() == 2);
          REQUIRE(indices[0] == 2);
          REQUIRE(indices[1] == 5);
        }

        // Test readIndexedCellValues to cover extractIndexValue for all types
        auto cells = vectorIndex->readIndexedCellValues();
        REQUIRE(cells.size() == 2);
        REQUIRE(cells[0].get<std::vector<int>>() == vec1);
        REQUIRE(cells[1].get<std::vector<int>>() == vec2);

        // Test readIndexedCellValues with start/count/stride/block
        auto cells2 = vectorIndex->readIndexedCellValues(
            0, 0);  // count 0 should return empty
        REQUIRE(cells2.empty());
        // Stride of 0 should throw
        REQUIRE_THROWS(vectorIndex->readIndexedCellValues(0, 2, 0));
        // Block of 0 should throw
        REQUIRE_THROWS(vectorIndex->readIndexedCellValues(0, 2, 1, 0));

        io->close();
      }
    }
  }

  SECTION("Uninitialized VectorIndex")
  {
    const std::string path = getTestFilePath("testVectorIndexUninitialized.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto vectorIndex = NWB::VectorIndex::create("/index", io);

    std::vector<int> vec = {1, 2};
    size_t elementsAppended = 0;

    // appendData on uninitialized index should fail
    REQUIRE(vectorIndex->appendData(vec, elementsAppended) == Status::Failure);

    // readIndexedCellValues on uninitialized index should throw runtime_error
    REQUIRE_THROWS_AS(vectorIndex->readIndexedCellValues(), std::runtime_error);

    io->close();
  }

  SECTION("Initialize in read-only mode")
  {
    const std::string path = getTestFilePath("testVectorIndexReadOnly.h5");
    {
      std::shared_ptr<BaseIO> io = createIO("HDF5", path);
      io->open();
      io->close();
    }

    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open(AQNWB::IO::FileMode::ReadOnly);

    auto vectorIndex = NWB::VectorIndex::create("/index", io);
    REQUIRE(vectorIndex->initialize(
                IO::ArrayDataSetConfig(BaseDataType::U32, {0}, {10}),
                "Index",
                "/target")
            == Status::Failure);

    io->close();
  }

  SECTION("Index out of bounds of target")
  {
    const std::string path = getTestFilePath("testVectorIndexOutOfBounds.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto target = NWB::VectorData::create("/target", io);
    target->initialize(IO::ArrayDataSetConfig(BaseDataType::I32, {0}, {10}),
                       "Target");
    auto vectorIndex = NWB::VectorIndex::create("/index", io);
    vectorIndex->initialize(
        IO::ArrayDataSetConfig(BaseDataType::U32, {0}, {10}),
        "Index",
        "/target");

    // Append 2 elements to target
    std::vector<int> vec = {1, 2};
    size_t elementsAppended = 0;
    vectorIndex->appendData(vec, elementsAppended);
    // Manually write an index that is out of bounds (e.g., 5)
    {
      auto indexRecorder = vectorIndex->recordData();
      io->startRecording();
      std::vector<uint32_t> badIndices = {5};
      indexRecorder->writeDataBlock(
          {1}, {1}, BaseDataType::U32, badIndices.data());
      io->flush();
    }
    REQUIRE_THROWS_AS(vectorIndex->readIndexedCellValues(), std::out_of_range);

    io->stopRecording();
    io->close();
  }

  SECTION("Invalid index data (decreasing indices)")
  {
    const std::string path = getTestFilePath("testVectorIndexInvalidData.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    auto target = NWB::VectorData::create("/target", io);
    target->initialize(IO::ArrayDataSetConfig(BaseDataType::I32, {0}, {10}),
                       "Target");

    auto vectorIndex = NWB::VectorIndex::create("/index", io);
    vectorIndex->initialize(
        IO::ArrayDataSetConfig(BaseDataType::U32, {0}, {10}),
        "Index",
        "/target");

    // Manually write invalid index data (decreasing indices)
    {
      auto indexRecorder = vectorIndex->recordData();
      io->startRecording();
      std::vector<uint32_t> badIndices = {5, 2};  // Decreasing indices
      indexRecorder->writeDataBlock(
          {2}, {0}, BaseDataType::U32, badIndices.data());
      io->flush();
    }

    REQUIRE_THROWS_AS(vectorIndex->readIndexedCellValues(),
                      std::invalid_argument);
    io->stopRecording();
    io->close();
  }
}
