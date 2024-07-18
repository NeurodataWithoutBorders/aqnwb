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
  std::vector<float> mockData = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  std::vector<float> mockTimestamps = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
  std::vector<SizeType> positionOffset = {0};
  std::vector<SizeType> dataShape = {5};
  nwbfile.writeTimeseries(0,
                          dataShape,
                          positionOffset,
                          BaseDataType::F32,
                          mockData.data(),
                          BaseDataType::F32,
                          mockTimestamps.data());
  nwbfile.writeTimeseries(1,
                          dataShape,
                          positionOffset,
                          BaseDataType::F32,
                          mockData.data(),
                          BaseDataType::F32,
                          mockTimestamps.data());

  nwbfile.finalize();

  REQUIRE(result == Status::Success);
}
