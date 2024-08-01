#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Utils.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("saveNWBFile", "[nwb]")
{
  std::string filename = getTestFilePath("testSaveNWBFile.nwb");

  // initialize nwbfile object and create base structure
  NWB::NWBFile nwbfile(generateUuid(),
                       std::make_unique<HDF5::HDF5IO>(filename));
  nwbfile.initialize();
  nwbfile.finalize();
}

TEST_CASE("createElectricalSeries", "[nwb]")
{
  std::string filename = getTestFilePath("createElectricalSeries.nwb");

  // initialize nwbfile object and create base structure
  NWB::NWBFile nwbfile(generateUuid(),
                       std::make_unique<HDF5::HDF5IO>(filename));
  nwbfile.initialize();

  // create Electrical Series
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1, 2);
  Status resultCreate =
      nwbfile.createElectricalSeries(mockArrays, BaseDataType::F32);

  // start Recording
  Status resultStart = nwbfile.startRecording();

  // write timeseries data
  std::vector<float> mockData = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  std::vector<double> mockTimestamps = {0.1, 0.2, 0.3, 0.4, 0.5};
  std::vector<SizeType> positionOffset = {0, 0};
  std::vector<SizeType> dataShape = {mockData.size(), 0};

  NWB::TimeSeries* ts0 = nwbfile.getTimeSeries(0);
  ts0->writeData(
      dataShape, positionOffset, mockData.data(), mockTimestamps.data());
  NWB::TimeSeries* ts1 = nwbfile.getTimeSeries(1);
  ts1->writeData(
      dataShape, positionOffset, mockData.data(), mockTimestamps.data());

  nwbfile.finalize();

  REQUIRE(resultCreate == Status::Success);
}

TEST_CASE("setRecordingMode", "[nwb]")
{
  std::string filename = getTestFilePath("testRecordingMode.nwb");

  // initialize nwbfile object and create base structure
  NWB::NWBFile nwbfile(generateUuid(),
                       std::make_unique<HDF5::HDF5IO>(filename));
  nwbfile.initialize();

  // start recording
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1, 2);
  Status result = nwbfile.createElectricalSeries(mockArrays, BaseDataType::F32);

  // start Recording
  Status resultStart = nwbfile.startRecording();
  REQUIRE(resultStart == Status::Success);

  // test that additional initialization / dataset creation fails after starting
  // the recording
  Status resultInitializePostStart = nwbfile.initialize();
  REQUIRE(resultInitializePostStart == Status::Failure);

  Status resultCreatePostStart =
      nwbfile.createElectricalSeries(mockArrays, BaseDataType::F32);
  REQUIRE(resultCreatePostStart == Status::Failure);

  // stop recording
  nwbfile.stopRecording();
  nwbfile.finalize();
}