#include <unordered_map>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RecordingContainers.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/misc/AnnotationSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("saveNWBFile", "[nwb]")
{
  std::string filename = getTestFilePath("testSaveNWBFile.nwb");

  // initialize nwbfile object and create base structure
  auto io = std::make_shared<IO::HDF5::HDF5IO>(filename);
  NWB::NWBFile nwbfile(io);
  nwbfile.initialize(generateUuid());
  nwbfile.finalize();
}

TEST_CASE("initialize", "[nwb]")
{
  std::string filename = getTestFilePath("testInitializeNWBFile.nwb");

  // initialize nwbfile object and create base structure
  auto io = std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);

  // bad session start time
  Status initStatus = nwbfile.initialize(generateUuid(),
                                         "test file",
                                         "no collection",
                                         "bad time",
                                         AQNWB::getCurrentTime());
  REQUIRE(initStatus == Status::Failure);

  // bad timestamp reference time
  initStatus = nwbfile.initialize(generateUuid(),
                                  "test file",
                                  "no collection",
                                  AQNWB::getCurrentTime(),
                                  "bad time");
  REQUIRE(initStatus == Status::Failure);

  // check that regular init with current times works
  initStatus = nwbfile.initialize(generateUuid());
  REQUIRE(initStatus == Status::Success);
  REQUIRE(nwbfile.isInitialized());
  nwbfile.finalize();
  io->close();
}

TEST_CASE("createElectrodesTable", "[nwb]")
{
  std::string filename = getTestFilePath("createElectrodesTable.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);
  nwbfile.initialize(generateUuid());

  // create the Electrodes Table
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1, 2);
  Status resultCreate = nwbfile.createElectrodesTable(mockArrays);
  REQUIRE(resultCreate == Status::Success);
}

TEST_CASE("createElectricalSeriesWithSubsetOfElectrodes", "[nwb]")
{
  std::string filename =
      getTestFilePath("createElectricalSeriesWithSubset.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);
  nwbfile.initialize(generateUuid());

  // Create electrode table with full set of electrodes (4 channels)
  std::vector<Types::ChannelVector> allElectrodes = getMockChannelArrays(4, 1);
  Status resultCreateTable = nwbfile.createElectrodesTable(allElectrodes);
  REQUIRE(resultCreateTable == Status::Success);

  // Create electrical series with subset of electrodes (2 channels)
  SizeType numChannels = 2;
  std::vector<Types::ChannelVector> recordingElectrodes =
      getMockChannelArrays(numChannels, 1);
  std::vector<std::string> recordingNames =
      getMockChannelArrayNames("esdata", 1);
  std::unique_ptr<NWB::RecordingContainers> recordingContainers =
      std::make_unique<NWB::RecordingContainers>();
  Status resultCreateES =
      nwbfile.createElectricalSeries(recordingElectrodes,
                                     recordingNames,
                                     BaseDataType::F32,
                                     recordingContainers.get());
  REQUIRE(resultCreateES == Status::Success);

  // Write some test data to verify recording works
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  std::vector<float> mockData = {1.0f, 2.0f, 3.0f};
  std::vector<double> mockTimestamps = {0.1, 0.2, 0.3};
  std::vector<SizeType> positionOffset = {0, 0};
  std::vector<SizeType> dataShape = {mockData.size(), 0};

  for (size_t i = 0; i < recordingElectrodes.size(); ++i) {
    for (size_t j = 0; j < recordingElectrodes[i].size(); ++j) {
      recordingContainers->writeElectricalSeriesData(0,
                                                     recordingElectrodes[i][j],
                                                     mockData.size(),
                                                     mockData.data(),
                                                     mockTimestamps.data());
    }
  }

  nwbfile.finalize();
}

TEST_CASE("createElectricalSeriesFailsWithoutElectrodesTable", "[nwb]")
{
  std::string filename = getTestFilePath("createElectricalSeriesNoTable.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);
  nwbfile.initialize(generateUuid());

  // Attempt to create electrical series without creating electrodes table first
  std::vector<Types::ChannelVector> recordingElectrodes =
      getMockChannelArrays(1, 2);
  std::vector<std::string> recordingNames =
      getMockChannelArrayNames("esdata", 1);
  Status resultCreateES = nwbfile.createElectricalSeries(
      recordingElectrodes, recordingNames, BaseDataType::F32);
  REQUIRE(resultCreateES == Status::Failure);

  nwbfile.finalize();
}

TEST_CASE("createElectricalSeriesFailsWithOutOfRangeIndices", "[nwb]")
{
  std::string filename =
      getTestFilePath("createElectricalSeriesOutOfRange.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);
  nwbfile.initialize(generateUuid());

  // Create electrode table with 2 channels
  std::vector<Types::ChannelVector> tableElectrodes =
      getMockChannelArrays(2, 1);
  Status resultCreateTable = nwbfile.createElectrodesTable(tableElectrodes);
  REQUIRE(resultCreateTable == Status::Success);

  // Attempt to create electrical series with channels having higher indices
  // Create mock channels with global indices > 1 (out of range of table)
  std::vector<Types::ChannelVector> recordingElectrodes =
      getMockChannelArrays(4, 1);

  std::vector<std::string> recordingNames =
      getMockChannelArrayNames("esdata", 1);
  Status resultCreateES = nwbfile.createElectricalSeries(
      recordingElectrodes, recordingNames, BaseDataType::F32);
  REQUIRE(resultCreateES == Status::Failure);
}

TEST_CASE("createElectricalSeries", "[nwb]")
{
  std::string filename = getTestFilePath("createElectricalSeries.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);
  nwbfile.initialize(generateUuid());

  // create the Electrodes Table
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays();
  nwbfile.createElectrodesTable(mockArrays);

  // create Electrical Series
  std::vector<std::string> mockChannelNames =
      getMockChannelArrayNames("esdata");
  std::unique_ptr<NWB::RecordingContainers> recordingContainers =
      std::make_unique<NWB::RecordingContainers>();
  Status resultCreate =
      nwbfile.createElectricalSeries(mockArrays,
                                     mockChannelNames,
                                     BaseDataType::F32,
                                     recordingContainers.get());
  REQUIRE(resultCreate == Status::Success);

  // start recording
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // write timeseries data
  std::vector<float> mockData = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  std::vector<double> mockTimestamps = {0.1, 0.3, 0.4, 0.5, 0.8};
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

  io->flush();

  // test searching for all NWBFile objects
  std::unordered_set<std::string> typesToSearch = {"core::NWBFile"};
  std::unordered_map<std::string, std::string> found_types =
      io->findTypes("/", typesToSearch, IO::SearchMode::CONTINUE_ON_TYPE);
  REQUIRE(found_types.size() == 1);  // We should have a single NWBFile

  // test searching for all ElectricalSeries objects
  std::unordered_set<std::string> typesToSearch2 = {"core::ElectricalSeries"};
  std::unordered_map<std::string, std::string> found_types2 =
      io->findTypes("/", typesToSearch2, IO::SearchMode::CONTINUE_ON_TYPE);
  REQUIRE(found_types2.size() == 2);  // We should have esdata1 and esdata2
  for (const auto& pair : found_types2) {
    // only ElectricalSeries should be found
    REQUIRE(pair.second == "core::ElectricalSeries");
    // only esdata1 or esdata2 should be in the list
    REQUIRE(((pair.first == "/acquisition/esdata1")
             || (pair.first == "/acquisition/esdata0")));
  }

  // finalize the nwb file
  nwbfile.finalize();
}

TEST_CASE("createMultipleEcephysDatasets", "[nwb]")
{
  std::string filename = getTestFilePath("createESandSES.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<HDF5::HDF5IO> io = std::make_shared<HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);
  nwbfile.initialize(generateUuid());

  // create ElectrodesTable
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(2, 2);
  nwbfile.createElectrodesTable(mockArrays);

  // create Electrical Series
  std::vector<std::string> mockChannelNames =
      getMockChannelArrayNames("esdata");
  std::unique_ptr<NWB::RecordingContainers> recordingContainers =
      std::make_unique<NWB::RecordingContainers>();
  Status resultCreateES =
      nwbfile.createElectricalSeries(mockArrays,
                                     mockChannelNames,
                                     BaseDataType::F32,
                                     recordingContainers.get());
  REQUIRE(resultCreateES == Status::Success);

  // create SpikeEventSeries
  SizeType numSamples = 5;
  std::vector<std::string> mockSpikeChannelNames =
      getMockChannelArrayNames("spikedata");
  Status resultCreateSES =
      nwbfile.createSpikeEventSeries(mockArrays,
                                     mockSpikeChannelNames,
                                     BaseDataType::F32,
                                     recordingContainers.get());
  REQUIRE(resultCreateSES == Status::Success);

  // start recording
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // write electrical series data
  std::vector<float> mockData = {
      1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  std::vector<double> mockTimestamps = {
      0.1, 0.3, 0.4, 0.5, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3};
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

  // write spike event series data
  SizeType numEvents = 10;
  NWB::SpikeEventSeries* ses0 =
      static_cast<NWB::SpikeEventSeries*>(recordingContainers->getContainer(2));
  NWB::SpikeEventSeries* ses1 =
      static_cast<NWB::SpikeEventSeries*>(recordingContainers->getContainer(3));
  for (SizeType i = 0; i < numEvents; ++i) {
    ses0->writeSpike(
        numSamples, mockArrays.size(), mockData.data(), &mockTimestamps[i]);
    ses1->writeSpike(
        numSamples, mockArrays.size(), mockData.data(), &mockTimestamps[i]);
  }

  nwbfile.finalize();
}

TEST_CASE("createAnnotationSeries", "[nwb]")
{
  std::string filename = getTestFilePath("createAnnotationSeries.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);
  nwbfile.initialize(generateUuid());

  // create Annotation Series
  std::vector<std::string> mockAnnotationNames = {"annotations1",
                                                  "annotations2"};
  std::unique_ptr<NWB::RecordingContainers> recordingContainers =
      std::make_unique<NWB::RecordingContainers>();
  Status resultCreate = nwbfile.createAnnotationSeries(
      mockAnnotationNames, recordingContainers.get());
  REQUIRE(resultCreate == Status::Success);

  // start recording
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // write annotation data
  std::vector<std::string> mockAnnotations = {
      "Start recording", "Subject moved", "End recording"};
  std::vector<double> mockTimestamps = {0.1, 0.5, 1.0};
  std::vector<SizeType> positionOffset = {0};
  SizeType dataShape = mockAnnotations.size();

  // write to both annotation series
  recordingContainers->writeAnnotationSeriesData(
      0, dataShape, mockAnnotations, mockTimestamps.data());
  recordingContainers->writeAnnotationSeriesData(
      1, dataShape, mockAnnotations, mockTimestamps.data());

  // test searching for all AnnotationSeries objects
  std::unordered_set<std::string> typesToSearch = {"core::AnnotationSeries"};
  std::unordered_map<std::string, std::string> found_types =
      io->findTypes("/", typesToSearch, IO::SearchMode::CONTINUE_ON_TYPE);
  REQUIRE(found_types.size()
          == 2);  // We should have annotations1 and annotations2
  for (const auto& pair : found_types) {
    // only AnnotationSeries should be found
    REQUIRE(pair.second == "core::AnnotationSeries");
    // only annotations1 or annotations2 should be in the list
    REQUIRE(((pair.first == "/acquisition/annotations1")
             || (pair.first == "/acquisition/annotations2")));
  }

  nwbfile.finalize();
}

TEST_CASE("setCanModifyObjectsMode", "[nwb]")
{
  std::string filename = getTestFilePath("testCanModifyObjectsMode.nwb");

  // initialize nwbfile object and create base structure with HDF5IO object
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  NWB::NWBFile nwbfile(io);
  Status initStatus = nwbfile.initialize(generateUuid());
  REQUIRE(initStatus == Status::Success);

  // start recording
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // test that modifying the file structure after starting the recording fails
  // Status resultInitializePostStart = nwbfile.initialize(generateUuid());
  Status resultInitializePostStart = io->createGroup("/new_group");
  REQUIRE(io->canModifyObjects() == false);
  REQUIRE(resultInitializePostStart == Status::Failure);

  // test that dataset creation fails after starting the recording
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1, 2);
  std::vector<std::string> mockChannelNames =
      getMockChannelArrayNames("esdata");
  nwbfile.createElectrodesTable(mockArrays);  // create the Electrodes Table
  Status resultCreatePostStart = nwbfile.createElectricalSeries(
      mockArrays, mockChannelNames, BaseDataType::F32);
  REQUIRE(resultCreatePostStart == Status::Failure);

  // stop recording
  nwbfile.finalize();
}
