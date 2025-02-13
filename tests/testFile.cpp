#include <catch2/catch_test_macros.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("ElectrodeTable", "[ecephys]")
{
  SECTION("test initialization and read")
  {
    std::string filename = getTestFilePath("electrodeTable.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<IO::HDF5::HDF5IO>(filename);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");
    io->createGroup("/general/extracellular_ephys/array0");

    std::vector<SizeType> channelIDs = {0, 1, 2};
    std::vector<Channel> channels = {
        Channel("ch0", "array0", 0, channelIDs[0], 0),
        Channel("ch1", "array0", 0, channelIDs[1], 1),
        Channel("ch2", "array0", 0, channelIDs[2], 2),
    };

    NWB::ElectrodeTable electrodeTable(io);
    electrodeTable.initialize();
    electrodeTable.addElectrodes(channels);
    electrodeTable.finalize();

    // Confirm that the column names are created correctly
    auto readColNames = electrodeTable.readColNames()->values().data;
    std::vector<std::string> expectedColNames = {
        "location", "group", "group_name"};
    REQUIRE(readColNames == expectedColNames);

    // Check if id datasets are created correctly
    SizeType numChannels = 3;
    std::unique_ptr<BaseRecordingData> id_data =
        io->getDataSet(NWB::ElectrodeTable::electrodeTablePath + "/id");
    std::unique_ptr<IO::HDF5::HDF5RecordingData> idDataset(
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(id_data.release()));
    int* buffer = new int[numChannels];
    readH5DataBlock(idDataset->getDataSet(), BaseDataType::I32, buffer);
    std::vector<SizeType> read_channels(buffer, buffer + numChannels);
    delete[] buffer;
    REQUIRE(channelIDs == read_channels);

    // Test reading the location data
    auto readLocation = electrodeTable.readLocationColumn();
    REQUIRE(readLocation != nullptr);
    auto readLocationData = readLocation->readData();
    auto readLocationValues = readLocationData->values().data;
    REQUIRE(readLocationValues.size() == 3);
    std::vector<std::string> expectedLocations = {
        "unknown", "unknown", "unknown"};
    REQUIRE(readLocationValues == expectedLocations);

    // Test reading the groupName data
    auto readGroupName = electrodeTable.readGroupNameColumn();
    REQUIRE(readGroupName != nullptr);
    auto readGroupNameData = readGroupName->readData();
    auto readGroupNameValues = readGroupNameData->values().data;
    REQUIRE(readGroupNameValues.size() == 3);
    std::vector<std::string> expectedGroupNames = {
        "array0", "array0", "array0"};
    REQUIRE(readGroupNameValues == expectedGroupNames);

    // Test reading the id column
    std::shared_ptr<NWB::ElementIdentifiers> readId =
        electrodeTable.readIdColumn();
    REQUIRE(readId != nullptr);
    auto readIdData = readId->readData();
    auto readIdValues = readIdData->values().data;
    REQUIRE(readIdValues.size() == 3);
    std::vector<int> expectedIdValues = {0, 1, 2};
    REQUIRE(readIdValues == expectedIdValues);

    // Test reading columns via the generic readColumn method
    auto readGroupName2 = electrodeTable.readColumn<std::string>("group_name");
    REQUIRE(readGroupName2 != nullptr);
    auto readGroupNameData2 = readGroupName2->readData();
    auto readGroupNameValues2 = readGroupNameData2->values().data;
    REQUIRE(readGroupNameValues.size() == 3);
    REQUIRE(readGroupNameValues == expectedGroupNames);

    // Test reading id column via the generic readColumn method as VectorData
    std::shared_ptr<NWB::VectorData<int>> readId2 =
        electrodeTable.readColumn<int>("id");
    REQUIRE(readId2 != nullptr);
    auto readIdData2 = readId2->readData();
    auto readIdValues2 = readIdData2->values().data;
    REQUIRE(readIdValues.size() == 3);
    REQUIRE(readIdValues == expectedIdValues);
  }

  SECTION("test initialization with empty channels")
  {
    std::vector<Channel> channels = {};

    std::string filename = getTestFilePath("electrodeTableNoData.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<IO::HDF5::HDF5IO>(filename);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");
    NWB::ElectrodeTable electrodeTable(io);
    electrodeTable.initialize();
  }

  SECTION("test table creation with multiple arrays")
  {
    // TODO
  }
}
