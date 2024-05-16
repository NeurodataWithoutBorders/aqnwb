
#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Types.hpp"
#include "Channel.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("ElectrodeTable", "[ecephys]")
{
  std::string path = "/electrodes/";
  SECTION("test initialization")
  {
    std::string filename = getTestFilePath("electrodeTable.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<HDF5::HDF5IO>(filename);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");
    io->createGroup("/general/extracellular_ephys/array1");

    std::vector<int> channelIDs = {0, 1, 2};
    std::vector<Channel> channels = {Channel("ch0", "array1", channelIDs[0], 0),
                                     Channel("ch1", "array1", channelIDs[1], 1),
                                     Channel("ch2", "array1", channelIDs[2], 2),};

    NWB::ElectrodeTable electrodeTable(path, io, channels);
    electrodeTable.initialize();
    electrodeTable.electrodeDataset->dataset =
        std::unique_ptr<BaseRecordingData>(io->createDataSet(
            BaseDataType::I32, SizeArray {1}, SizeArray {1}, path + "id"));

    electrodeTable.locationsDataset->dataset =
        std::unique_ptr<BaseRecordingData>(
            io->createDataSet(BaseDataType::STR(250),
                              SizeArray {0},
                              SizeArray {1},
                              path + "location"));
    electrodeTable.addElectrodes();

    // Check if id datasets are created correctly
    size_t numChannels = 3;
    BaseRecordingData* id_data = io->getDataSet(path + "id");
    int* buffer = new int[numChannels];
    static_cast<HDF5::HDF5RecordingData*>(id_data)->readDataBlock(
        BaseDataType::I32, buffer);
    std::vector<int> read_channels(buffer, buffer + numChannels);
    delete[] buffer;
    REQUIRE(channelIDs == read_channels);
  }

  SECTION("test initialization with empty channels")
  {
    std::vector<Channel> channels = {};

    std::string filename = getTestFilePath("electrodeTableNoData.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<HDF5::HDF5IO>(filename);
    io->open();
    NWB::ElectrodeTable electrodeTable(path, io, channels, "none");
    electrodeTable.initialize();
  }
}


TEST_CASE("ElectricalSeries", "[ecephys]")
{
  std::string filename = getTestFilePath("ElectricalSeries.h5");

  // setup recording info
  std::vector<Types::ChannelGroup> mockArrays = getMockTestArrays();

  std::string path = "/electrodes/";
  SECTION("test initialization")
  {
  }

  SECTION("test linking to electrode table region")
  {
  }
  
}
