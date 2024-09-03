#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Utils.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RecordingContainers.hpp"
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
  std::shared_ptr<HDF5::HDF5IO> io = std::make_shared<HDF5::HDF5IO>(filename);
  NWB::NWBFile nwbfile(generateUuid(), io);
  nwbfile.initialize();

  // create Electrical Series
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1, 2);
  std::unique_ptr<NWB::RecordingContainers> recordingContainers =
      std::make_unique<NWB::RecordingContainers>();
  Status resultCreate = nwbfile.createElectricalSeries(
      mockArrays, BaseDataType::F32, recordingContainers.get());
  REQUIRE(resultCreate == Status::Success);

  // start recording
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // write timeseries data
  std::vector<float> mockData = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  std::vector<double> mockTimestamps = {0.1, 0.2, 0.3, 0.4, 0.5};
  std::vector<SizeType> positionOffset = {0, 0};
  std::vector<SizeType> dataShape = {mockData.size(), 0};

  NWB::TimeSeries* ts0 =
      static_cast<NWB::TimeSeries*>(recordingContainers->getContainer(0));
  ts0->writeData(
      dataShape, positionOffset, mockData.data(), mockTimestamps.data());
  NWB::TimeSeries* ts1 =
      static_cast<NWB::TimeSeries*>(recordingContainers->getContainer(1));
  ts1->writeData(
      dataShape, positionOffset, mockData.data(), mockTimestamps.data());

  nwbfile.finalize();
}

TEST_CASE("setCanModifyObjectsMode", "[nwb]")
{
  std::string filename = getTestFilePath("testCanModifyObjectsMode.nwb");

  // initialize nwbfile object and create base structure with HDF5IO object
  std::shared_ptr<HDF5::HDF5IO> io = std::make_shared<HDF5::HDF5IO>(filename);
  NWB::NWBFile nwbfile(generateUuid(), io);
  nwbfile.initialize();

  // start recording
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // test that modifying the file structure after starting the recording fails
  Status resultInitializePostStart = nwbfile.initialize();
  REQUIRE(resultInitializePostStart == Status::Failure);

  // test that dataset creation fails after starting the recording
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1, 2);
  Status resultCreatePostStart =
      nwbfile.createElectricalSeries(mockArrays, BaseDataType::F32);
  REQUIRE(resultCreatePostStart == Status::Failure);

  // stop recording
  nwbfile.finalize();
}
