#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("ElementIdentifiers", "[base]")
{
  SECTION(
      "test ElementIdentifiers is registered as a subclass of RegisteredType")
  {
    auto registry = AQNWB::NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("hdmf-common::ElementIdentifiers") != registry.end());
  }

  SECTION("test ElementIdentifiers write/read")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/vdata";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    SizeArray positionOffset = {0};
    BaseDataType dataType = BaseDataType::I32;
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string path = getTestFilePath("testElementIdentifiers.h5");

    // Create the HDF5 file to write to
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // create config for Data.initialize
    IO::ArrayDataSetConfig config(dataType, dataShape, chunking);

    // setup Data object
    auto columnData = NWB::ElementIdentifiers::create(dataPath, io);
    columnData->initialize(config);

    // Write data to file
    Status writeStatus = columnData->recordData()->writeDataBlock(
        dataShape, positionOffset, dataType, data.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();
    io->close();

    // Read data back from file
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open(FileMode::ReadOnly);

    // Read all fields using the standard read methods
    auto readRegisteredType = NWB::RegisteredType::create(dataPath, readio);
    auto readElementIdentifiers =
        std::dynamic_pointer_cast<NWB::ElementIdentifiers>(readRegisteredType);

    // Read the "namespace" attribute via the readNamespace field
    auto namespaceData = readElementIdentifiers->readNamespace();
    std::string namespaceStr = namespaceData->values().data[0];
    REQUIRE(namespaceStr == "hdmf-common");

    // Read the "neurodata_type" attribute via the readNeurodataType field
    auto neurodataTypeData = readElementIdentifiers->readNeurodataType();
    std::string neurodataTypeStr = neurodataTypeData->values().data[0];
    REQUIRE(neurodataTypeStr == "ElementIdentifiers");

    // Read the data via the readElementIdentifiers field
    auto dataData = readElementIdentifiers->readData();
    auto dataBlockInt = dataData->values();
    REQUIRE(dataBlockInt.data == data);
  }

  SECTION("test record methods from DEFINE_DATASET_FIELD")
  {
    // Prepare test data
    SizeType numSamples = 10;
    std::string dataPath = "/element_identifiers_record_test";
    SizeArray dataShape = {numSamples};
    SizeArray chunking = {numSamples};
    BaseDataType dataType = BaseDataType::I32;
    std::string path = getTestFilePath("testElementIdentifiersRecord.h5");

    // Create the HDF5 file to write to
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // create config for Data.initialize
    IO::ArrayDataSetConfig config(dataType, dataShape, chunking);

    // setup ElementIdentifiers object
    auto elementIdentifiers = NWB::ElementIdentifiers::create(dataPath, io);
    elementIdentifiers->initialize(config);

    // Test recordData method
    auto dataRecorder = elementIdentifiers->recordData();
    REQUIRE(dataRecorder != nullptr);

    io->close();
  }
}
