#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Types.hpp"
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
    io->createGroup("/general/extracellular_ephys/array0");

    std::vector<SizeType> channelIDs = {0, 1, 2};
    std::vector<Channel> channels = {
        Channel("ch0", "array0", channelIDs[0], 0),
        Channel("ch1", "array0", channelIDs[1], 1),
        Channel("ch2", "array0", channelIDs[2], 2),
    };

    NWB::ElectrodeTable electrodeTable(path, io);
    electrodeTable.initialize();
    electrodeTable.addElectrodes(channels);
    electrodeTable.finalize();

    // Check if id datasets are created correctly
    SizeType numChannels = 3;
    std::unique_ptr<BaseRecordingData> id_data = io->getDataSet(path + "id");
    std::unique_ptr<HDF5::HDF5RecordingData> idDataset(
        dynamic_cast<HDF5::HDF5RecordingData*>(id_data.release()));
    int* buffer = new int[numChannels];
    readH5DataBlock(idDataset->getDataSet(), BaseDataType::I32, buffer);
    std::vector<SizeType> read_channels(buffer, buffer + numChannels);
    delete[] buffer;
    REQUIRE(channelIDs == read_channels);
  }

  SECTION("test initialization with empty channels")
  {
    std::vector<Channel> channels = {};

    std::string filename = getTestFilePath("electrodeTableNoData.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<HDF5::HDF5IO>(filename);
    io->open();
    NWB::ElectrodeTable electrodeTable(path, io);
    electrodeTable.initialize();
  }

  SECTION("test table creation with multiple arrays")
  {
    // TODO
  }
}
