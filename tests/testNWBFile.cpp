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
  std::string filename = getTestFilePath("testStartRecording.nwb");

  // initialize nwbfile object and create base structure
  NWB::NWBFile nwbfile(generateUuid(),
                       std::make_unique<HDF5::HDF5IO>(filename));
  nwbfile.initialize();

  // start recording
  std::vector<Types::ChannelGroup> mockArrays = getMockChannelArrays(1, 2);
  Status result = nwbfile.startRecording(mockArrays, BaseDataType::F32);

  // write timeseries data
  std::vector<float> mockData = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  std::vector<double> mockTimestamps = {0.1, 0.2, 0.3, 0.4, 0.5};
  std::vector<SizeType> positionOffset = {0, 0};
  std::vector<SizeType> dataShape = {mockData.size(), 0};
  nwbfile.writeTimeseries(0,
                          dataShape,
                          positionOffset,
                          mockData.data(),
                          BaseDataType::F32,
                          mockTimestamps.data());
  nwbfile.writeTimeseries(1,
                          dataShape,
                          positionOffset,
                          mockData.data(),
                          BaseDataType::F32,
                          mockTimestamps.data());

  nwbfile.finalize();

  REQUIRE(result == Status::Success);
}
