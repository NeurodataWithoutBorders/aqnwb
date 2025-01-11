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
    std::string path = getTestFilePath("testData.h5");

    // Create the HDF5 file to write to
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // create BaseRecordingData to pass to Data.initialize
    std::unique_ptr<BaseRecordingData> columnDataset =
        io->createArrayDataSet(dataType, dataShape, chunking, dataPath);

    // setup Data object
    NWB::ElementIdentifiers columnData = NWB::ElementIdentifiers(dataPath, io);
    columnData.initialize(std::move(columnDataset));

    // Write data to file
    Status writeStatus = columnData.m_dataset->writeDataBlock(
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
}
