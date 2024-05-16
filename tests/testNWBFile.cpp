#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Utils.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("saveNWBFile", "[nwb]")
{
  std::string filename = getTestFilePath("test_nwb_file.nwb");

  NWB::NWBFile nwbfile(generateUuid(),
                       std::make_unique<HDF5::HDF5IO>(filename));
  nwbfile.initialize();
  nwbfile.finalize();
}

TEST_CASE("startRecording", "[nwb]")
{
  std::string filename = getTestFilePath("test_recording.nwb");

  NWB::NWBFile nwbfile(generateUuid(),
                       std::make_unique<HDF5::HDF5IO>(filename));
  nwbfile.initialize();

std::vector<Types::ChannelGroup> mockArrays = getMockTestArrays();

  Status result = nwbfile.startRecording(mockArrays);
  nwbfile.finalize();

  REQUIRE(result == Status::Success);
}
