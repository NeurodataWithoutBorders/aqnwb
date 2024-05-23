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

  // initialize nwbfile object and create base structure
  NWB::NWBFile nwbfile(generateUuid(),
                       std::make_unique<HDF5::HDF5IO>(filename));
  nwbfile.initialize();
  nwbfile.finalize();
}

TEST_CASE("startRecording", "[nwb]")
{
  std::string filename = getTestFilePath("test_recording.nwb");

  // initialize nwbfile object and create base structure
  NWB::NWBFile nwbfile(generateUuid(),
                       std::make_unique<HDF5::HDF5IO>(filename));
  nwbfile.initialize();

  // start recording
  std::vector<Types::ChannelGroup> mockArrays = getMockChannelArrays();
  Status result = nwbfile.startRecording(mockArrays);

  // write timeseries data
  std::vector<float> mockData = {1.0, 2.0, 3.0, 4.0, 5.0};
  std::vector<float> mockTimestamps = {0.1, 0.2, 0.3, 0.4, 0.5};
  nwbfile.writeTimeseriesData(0, 0, 5, BaseDataType::F32, mockData.data());
  nwbfile.writeTimeseriesTimestamps(0, 5, BaseDataType::F32, mockTimestamps.data());
  nwbfile.writeTimeseriesData(1, 0, 5, BaseDataType::F32, mockData.data());
  nwbfile.writeTimeseriesTimestamps(1, 5, BaseDataType::F32, mockTimestamps.data());

  nwbfile.finalize();

  REQUIRE(result == Status::Success);
}
