#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RecordingObjects.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/misc/AnnotationSeries.hpp"
#include "spec/core.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("NWBFile registered", "[nwb]")
{
  auto registry = AQNWB::NWB::RegisteredType::getRegistry();
  REQUIRE(registry.find("core::NWBFile") != registry.end());
}

TEST_CASE("saveNWBFile", "[nwb]")
{
  std::string filename = getTestFilePath("testSaveNWBFile.nwb");

  // initialize nwbfile object and create base structure
  auto io = std::make_shared<IO::HDF5::HDF5IO>(filename);
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());
  nwbfile->finalize();  // Good practive since we don't call stop recording, but
                        // not essential
  io->close();
}

TEST_CASE("initialize", "[nwb]")
{
  std::string filename = getTestFilePath("testInitializeNWBFile.nwb");

  // initialize nwbfile object and create base structure
  auto io = std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);

  // bad session start time
  Status initStatus = nwbfile->initialize(generateUuid(),
                                          "test file",
                                          "no collection",
                                          "bad time",
                                          AQNWB::getCurrentTime());
  REQUIRE(initStatus == Status::Failure);

  // bad timestamp reference time
  initStatus = nwbfile->initialize(generateUuid(),
                                   "test file",
                                   "no collection",
                                   AQNWB::getCurrentTime(),
                                   "bad time");
  REQUIRE(initStatus == Status::Failure);

  // check that regular init with current times works
  initStatus = nwbfile->initialize(generateUuid());
  REQUIRE(initStatus == Status::Success);
  REQUIRE(nwbfile->isInitialized());

  // Since we didn't create any typed objects within the NWBFile, we should
  // have no owned types
  auto result = nwbfile->findOwnedTypes();
  REQUIRE(result.size() == 0);

  nwbfile->finalize();  // Good practive since we don't call stop recording, but
                        // not essential
  io->close();  // close the io
}

TEST_CASE("createElectrodesTable", "[nwb]")
{
  std::string filename = getTestFilePath("createElectrodesTable.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // create the Electrodes Table
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1, 2);
  Status resultCreate = nwbfile->createElectrodesTable(mockArrays);
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
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // Create electrode table with full set of electrodes (4 channels)
  std::vector<Types::ChannelVector> allElectrodes = getMockChannelArrays(4, 1);
  Status resultCreateTable = nwbfile->createElectrodesTable(allElectrodes);
  REQUIRE(resultCreateTable == Status::Success);

  // Create electrical series with subset of electrodes (2 channels)
  SizeType numChannels = 2;
  std::vector<Types::ChannelVector> recordingElectrodes =
      getMockChannelArrays(numChannels, 1);
  std::vector<std::string> recordingNames =
      getMockChannelArrayNames("esdata", 1);
  auto recordingObjects = io->getRecordingObjects();
  SizeType sizeBefore = recordingObjects->size();
  Status resultCreateES = nwbfile->createElectricalSeries(
      recordingElectrodes, recordingNames, BaseDataType::F32);
  SizeType sizeAfter = recordingObjects->size();
  std::vector<SizeType> containerIndices(sizeAfter - sizeBefore);
  std::iota(containerIndices.begin(), containerIndices.end(), sizeBefore);
  REQUIRE(resultCreateES == Status::Success);

  // Write some test data to verify recording works
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  std::vector<float> mockData = {1.0f, 2.0f, 3.0f};
  std::vector<double> mockTimestamps = {0.1, 0.2, 0.4};
  std::vector<SizeType> positionOffset = {0, 0};
  std::vector<SizeType> dataShape = {mockData.size(), 0};

  for (size_t i = 0; i < recordingElectrodes.size(); ++i) {
    for (size_t j = 0; j < recordingElectrodes[i].size(); ++j) {
      recordingObjects->writeElectricalSeriesData(0,
                                                  recordingElectrodes[i][j],
                                                  mockData.size(),
                                                  mockData.data(),
                                                  mockTimestamps.data());
    }
  }

  io->stopRecording();
}

TEST_CASE("createElectricalSeriesFailsWithoutElectrodesTable", "[nwb]")
{
  std::string filename = getTestFilePath("createElectricalSeriesNoTable.h5");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // Attempt to create electrical series without creating electrodes table first
  std::vector<Types::ChannelVector> recordingElectrodes =
      getMockChannelArrays(1, 2);
  std::vector<std::string> recordingNames =
      getMockChannelArrayNames("esdata", 1);
  Status resultCreateES = nwbfile->createElectricalSeries(
      recordingElectrodes, recordingNames, BaseDataType::F32);
  REQUIRE(resultCreateES == Status::Failure);

  nwbfile->finalize();  // Good practive since we don't call stop recording, but
                        // not essential
  io->close();
}

TEST_CASE("createElectricalSeriesFailsWithOutOfRangeIndices", "[nwb]")
{
  std::string filename = getTestFilePath("createElectricalSeriesOutOfRange.h5");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // Create electrode table with 2 channels
  std::vector<Types::ChannelVector> tableElectrodes =
      getMockChannelArrays(2, 1);
  Status resultCreateTable = nwbfile->createElectrodesTable(tableElectrodes);
  REQUIRE(resultCreateTable == Status::Success);

  // Attempt to create electrical series with channels having higher indices
  // Create mock channels with global indices > 1 (out of range of table)
  std::vector<Types::ChannelVector> recordingElectrodes =
      getMockChannelArrays(4, 1);

  std::vector<std::string> recordingNames =
      getMockChannelArrayNames("esdata", 1);
  Status resultCreateES = nwbfile->createElectricalSeries(
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
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // create the Electrodes Table
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays();
  nwbfile->createElectrodesTable(mockArrays);

  // create Electrical Series
  std::vector<std::string> mockChannelNames =
      getMockChannelArrayNames("esdata");
  auto recordingObjects = io->getRecordingObjects();
  SizeType sizeBefore = recordingObjects->size();
  Status resultCreate = nwbfile->createElectricalSeries(
      mockArrays, mockChannelNames, BaseDataType::F32);
  SizeType sizeAfter = recordingObjects->size();
  std::vector<SizeType> containerIndices(sizeAfter - sizeBefore);
  std::iota(containerIndices.begin(), containerIndices.end(), sizeBefore);
  REQUIRE(resultCreate == Status::Success);

  // start recording
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // write timeseries data
  std::vector<float> mockData = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  std::vector<double> mockTimestamps = {0.1, 0.3, 0.4, 0.5, 0.8};
  std::vector<SizeType> positionOffset = {0, 0};
  std::vector<SizeType> dataShape = {mockData.size(), 0};

  auto ts0 = std::dynamic_pointer_cast<NWB::TimeSeries>(
      recordingObjects->getRecordingObject(containerIndices[0]));
  ts0->writeData(
      dataShape, positionOffset, mockData.data(), mockTimestamps.data());
  auto ts1 = std::dynamic_pointer_cast<NWB::TimeSeries>(
      recordingObjects->getRecordingObject(containerIndices[1]));
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

  // Check that we can find all the types that we created
  // - /general/extracellular_ephys/array0 : core::ElectrodeGroup
  // - /general/devices/array1 : core::Device
  // - /general/extracellular_ephys/electrodes : core::DynamicTable
  // - /acquisition/esdata1 : core::ElectricalSeries
  // - /general/devices/array0 : core::Device
  // - /general/extracellular_ephys/array1 : core::ElectrodeGroup
  // - /acquisition/esdata0 : core::ElectricalSeries
  auto result = nwbfile->findOwnedTypes();
  REQUIRE(result.size() == 7);

  // finalize the nwb file
  io->stopRecording();
}

TEST_CASE("createMultipleEcephysDatasets", "[nwb]")
{
  std::string filename = getTestFilePath("createESandSES.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<HDF5::HDF5IO> io = std::make_shared<HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = AQNWB::NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // create ElectrodesTable
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(2, 2);
  nwbfile->createElectrodesTable(mockArrays);

  // create Electrical Series
  std::vector<std::string> mockChannelNames =
      getMockChannelArrayNames("esdata");
  auto recordingObjects = io->getRecordingObjects();
  SizeType sizeBefore = recordingObjects->size();
  Status resultCreateES = nwbfile->createElectricalSeries(
      mockArrays, mockChannelNames, BaseDataType::F32);
  SizeType sizeAfter = recordingObjects->size();
  std::vector<SizeType> containerIndices(sizeAfter - sizeBefore);
  std::iota(containerIndices.begin(), containerIndices.end(), sizeBefore);
  REQUIRE(resultCreateES == Status::Success);

  // create SpikeEventSeries
  SizeType numSamples = 5;
  std::vector<std::string> mockSpikeChannelNames =
      getMockChannelArrayNames("spikedata");
  sizeBefore = recordingObjects->size();
  Status resultCreateSES = nwbfile->createSpikeEventSeries(
      mockArrays, mockSpikeChannelNames, BaseDataType::F32);
  sizeAfter = recordingObjects->size();
  for (SizeType i = sizeBefore; i < sizeAfter; ++i) {
    containerIndices.push_back(i);
  }
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

  auto ts0 = std::dynamic_pointer_cast<NWB::TimeSeries>(
      recordingObjects->getRecordingObject(containerIndices[0]));
  ts0->writeData(
      dataShape, positionOffset, mockData.data(), mockTimestamps.data());
  auto ts1 = std::dynamic_pointer_cast<NWB::TimeSeries>(
      recordingObjects->getRecordingObject(containerIndices[1]));
  ts1->writeData(
      dataShape, positionOffset, mockData.data(), mockTimestamps.data());

  // write spike event series data
  SizeType numEvents = 10;
  auto ses0 = std::dynamic_pointer_cast<NWB::SpikeEventSeries>(
      recordingObjects->getRecordingObject(containerIndices[2]));
  auto ses1 = std::dynamic_pointer_cast<NWB::SpikeEventSeries>(
      recordingObjects->getRecordingObject(containerIndices[3]));
  for (SizeType i = 0; i < numEvents; ++i) {
    ses0->writeSpike(
        numSamples, mockArrays.size(), mockData.data(), &mockTimestamps[i]);
    ses1->writeSpike(
        numSamples, mockArrays.size(), mockData.data(), &mockTimestamps[i]);
  }

  io->stopRecording();
}

TEST_CASE("createAnnotationSeries", "[nwb]")
{
  std::string filename = getTestFilePath("createAnnotationSeries.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // create Annotation Series
  std::vector<std::string> mockAnnotationNames = {"annotations1",
                                                  "annotations2"};
  auto recordingObjects = io->getRecordingObjects();
  SizeType sizeBefore = recordingObjects->size();
  Status resultCreate = nwbfile->createAnnotationSeries(mockAnnotationNames);
  SizeType sizeAfter = recordingObjects->size();
  std::vector<SizeType> containerIndices(sizeAfter - sizeBefore);
  std::iota(containerIndices.begin(), containerIndices.end(), sizeBefore);
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
  recordingObjects->writeAnnotationSeriesData(
      containerIndices[0], dataShape, mockAnnotations, mockTimestamps.data());
  recordingObjects->writeAnnotationSeriesData(
      containerIndices[1], dataShape, mockAnnotations, mockTimestamps.data());

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

  io->stopRecording();
}

TEST_CASE("setCanModifyObjectsMode", "[nwb]")
{
  std::string filename = getTestFilePath("testCanModifyObjectsMode.nwb");

  // initialize nwbfile object and create base structure with HDF5IO object
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  Status initStatus = nwbfile->initialize(generateUuid());
  REQUIRE(initStatus == Status::Success);

  // start recording
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // test that modifying the file structure after starting the recording fails
  // Status resultInitializePostStart = nwbfile->initialize(generateUuid());
  Status resultInitializePostStart = io->createGroup("/new_group");
  REQUIRE(io->canModifyObjects() == false);
  REQUIRE(resultInitializePostStart == Status::Failure);

  // test that dataset creation fails after starting the recording
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1, 2);
  std::vector<std::string> mockChannelNames =
      getMockChannelArrayNames("esdata");
  nwbfile->createElectrodesTable(mockArrays);  // create the Electrodes Table
  Status resultCreatePostStart = nwbfile->createElectricalSeries(
      mockArrays, mockChannelNames, BaseDataType::F32);
  REQUIRE(resultCreatePostStart == Status::Failure);

  // stop recording
  io->stopRecording();
}

TEST_CASE("testAttributeAndDatasetFields", "[nwb]")
{
  std::string filename =
      getTestFilePath("testAttributeAndDatasetFieldsRead.nwb");

  // Initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);

  // Generate a unique identifier for the file
  std::string identifier = generateUuid();
  std::string description = "Test file for attribute and dataset fields";
  std::string dataCollection = "Test data collection";
  std::string sessionStartTime = AQNWB::getCurrentTime();
  std::string timestampsReferenceTime = AQNWB::getCurrentTime();

  // Initialize the file with our test values
  Status initStatus = nwbfile->initialize(identifier,
                                          description,
                                          dataCollection,
                                          sessionStartTime,
                                          timestampsReferenceTime);
  REQUIRE(initStatus == Status::Success);
  REQUIRE(nwbfile->isInitialized());

  // Test reading attribute fields (DEFINE_ATTRIBUTE_FIELD)
  auto nwbVersionData = nwbfile->readNWBVersion();
  REQUIRE(nwbVersionData->exists());
  REQUIRE(nwbVersionData->getStorageObjectType()
          == StorageObjectType::Attribute);
  std::string nwbVersion = nwbVersionData->values().data[0];
  REQUIRE(nwbVersion == AQNWB::SPEC::CORE::version);  // This should match the
                                                      // current NWB version

  // Test reading dataset fields (DEFINE_DATASET_FIELD)
  auto identifierData = nwbfile->readIdentifier();
  REQUIRE(identifierData->exists());
  REQUIRE(identifierData->getStorageObjectType() == StorageObjectType::Dataset);
  std::string readIdentifier = identifierData->values().data[0];
  REQUIRE(readIdentifier == identifier);

  auto sessionDescriptionData = nwbfile->readSessionDescription();
  REQUIRE(sessionDescriptionData->exists());
  REQUIRE(sessionDescriptionData->getStorageObjectType()
          == StorageObjectType::Dataset);
  std::string readDescription = sessionDescriptionData->values().data[0];
  REQUIRE(readDescription == description);

  auto sessionStartTimeData = nwbfile->readSessionStartTime();
  REQUIRE(sessionStartTimeData->exists());
  REQUIRE(sessionStartTimeData->getStorageObjectType()
          == StorageObjectType::Dataset);
  std::string readSessionStartTime = sessionStartTimeData->values().data[0];
  REQUIRE(readSessionStartTime == sessionStartTime);

  auto timestampsReferenceTimeData = nwbfile->readTimestampsReferenceTime();
  REQUIRE(timestampsReferenceTimeData->exists());
  REQUIRE(timestampsReferenceTimeData->getStorageObjectType()
          == StorageObjectType::Dataset);
  std::string readTimestampsReferenceTime =
      timestampsReferenceTimeData->values().data[0];
  REQUIRE(readTimestampsReferenceTime == timestampsReferenceTime);

  auto fileCreateDateData = nwbfile->readFileCreateDate();
  REQUIRE(fileCreateDateData->exists());
  REQUIRE(fileCreateDateData->getStorageObjectType()
          == StorageObjectType::Dataset);
  REQUIRE(fileCreateDateData->values().data.size() > 0);

  // Test record methods (from DEFINE_DATASET_FIELD)
  // Start the recording for the file
  Status resultStart = io->startRecording();
  REQUIRE(resultStart == Status::Success);

  // Test recordIdentifier method
  auto identifierRecorder = nwbfile->recordIdentifier();
  REQUIRE(identifierRecorder != nullptr);

  // Test recordSessionDescription method
  auto sessionDescriptionRecorder = nwbfile->recordSessionDescription();
  REQUIRE(sessionDescriptionRecorder != nullptr);

  // Test recordSessionStartTime method
  auto sessionStartTimeRecorder = nwbfile->recordSessionStartTime();
  REQUIRE(sessionStartTimeRecorder != nullptr);

  // Test recordTimestampsReferenceTime method
  auto timestampsReferenceTimeRecorder =
      nwbfile->recordTimestampsReferenceTime();
  REQUIRE(timestampsReferenceTimeRecorder != nullptr);

  // Test recordFileCreateDate method
  auto fileCreateDateRecorder = nwbfile->recordFileCreateDate();
  REQUIRE(fileCreateDateRecorder != nullptr);

  // Finalize the file and close the io
  io->stopRecording();
}
