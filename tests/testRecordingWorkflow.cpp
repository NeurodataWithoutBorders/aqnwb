
#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RecordingContainers.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("writeContinuousData", "[recording]")
{
  SECTION("test data and timestamps stream")
  {
    // 0. setup mock data
    SizeType numChannels = 4;
    SizeType numSamples = 100;
    SizeType samplesRecorded = 0;
    SizeType bufferSize = numSamples / 10;
    std::vector<float> dataBuffer(bufferSize);
    std::vector<double> timestampsBuffer(bufferSize);

    std::vector<Types::ChannelVector> mockRecordingArrays =
        getMockChannelArrays();
    std::vector<std::string> mockChannelNames =
        getMockChannelArrayNames("esdata");
    std::vector<std::vector<float>> mockData =
        getMockData2D(numSamples, numChannels);
    std::vector<double> mockTimestamps = getMockTimestamps(numSamples);

    // 1. create IO object
    std::string path = getTestFilePath("testContinuousRecording1.nwb");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // 2. create RecordingContainers object
    std::unique_ptr<NWB::RecordingContainers> recordingContainers =
        std::make_unique<NWB::RecordingContainers>();
    std::vector<SizeType> containerIndices = {};

    // 3. create NWBFile object
    std::unique_ptr<NWB::NWBFile> nwbfile = std::make_unique<NWB::NWBFile>(io);
    nwbfile->initialize(generateUuid());

    // 4. create an electrodes table.
    nwbfile->createElectrodesTable(mockRecordingArrays);

    // 5. create datasets and add to recording containers
    nwbfile->createElectricalSeries(mockRecordingArrays,
                                    mockChannelNames,
                                    BaseDataType::F32,
                                    recordingContainers.get(),
                                    containerIndices);

    // 6. start the recording
    io->startRecording();

    // 7. write data during the recording
    bool isRecording = true;
    while (isRecording) {
      // write data to the file for each channel
      for (SizeType i = 0; i < mockRecordingArrays.size(); ++i) {
        const auto& channelVector = mockRecordingArrays[i];
        for (const auto& channel : channelVector) {
          // copy data into buffer
          std::copy(
              mockData[channel.getGlobalIndex()].begin()
                  + static_cast<std::ptrdiff_t>(samplesRecorded),
              mockData[channel.getGlobalIndex()].begin()
                  + static_cast<std::ptrdiff_t>(samplesRecorded + bufferSize),
              dataBuffer.begin());
          std::copy(
              mockTimestamps.begin()
                  + static_cast<std::ptrdiff_t>(samplesRecorded),
              mockTimestamps.begin()
                  + static_cast<std::ptrdiff_t>(samplesRecorded + bufferSize),
              timestampsBuffer.begin());

          // write timeseries data
          std::vector<SizeType> positionOffset = {samplesRecorded,
                                                  channel.getLocalIndex()};
          std::vector<SizeType> dataShape = {dataBuffer.size(), 1};

          recordingContainers->writeTimeseriesData(i,
                                                   channel,
                                                   dataShape,
                                                   positionOffset,
                                                   dataBuffer.data(),
                                                   timestampsBuffer.data());
        }
      }
      // check if recording is done
      samplesRecorded += dataBuffer.size();
      if (samplesRecorded >= numSamples) {
        isRecording = false;
      }
    }

    // 8. stop the recording and finalize the file
    io->stopRecording();
    nwbfile->finalize();

    // check contents of data
    std::string dataPath = "/acquisition/esdata0/data";
    std::unique_ptr<H5::H5File> file =
        std::make_unique<H5::H5File>(path, H5F_ACC_RDONLY);
    std::unique_ptr<H5::DataSet> dataset =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath));
    SizeType numChannelsToRead = numChannels / 2;

    float* buffer = new float[numSamples * numChannelsToRead];

    H5::DataSpace fSpace = dataset->getSpace();
    hsize_t dims[1] = {numSamples * numChannelsToRead};
    H5::DataSpace mSpace(1, dims);
    dataset->read(buffer, H5::PredType::NATIVE_FLOAT, mSpace, fSpace);

    std::vector<std::vector<float>> dataOut(numChannelsToRead,
                                            std::vector<float>(numSamples));
    for (SizeType i = 0; i < numChannelsToRead; ++i) {
      for (SizeType j = 0; j < numSamples; ++j) {
        dataOut[i][j] = buffer[j * numChannelsToRead + i];
      }
    }
    delete[] buffer;
    REQUIRE_THAT(dataOut[0], Catch::Matchers::Approx(mockData[0]).margin(1));
    REQUIRE_THAT(dataOut[1], Catch::Matchers::Approx(mockData[1]).margin(1));

    // check contents of timestamps
    std::string timestampsPath = "/acquisition/esdata0/timestamps";
    std::unique_ptr<H5::DataSet> tsDataset =
        std::make_unique<H5::DataSet>(file->openDataSet(timestampsPath));
    double* tsBuffer = new double[numSamples];

    H5::DataSpace tsfSpace = tsDataset->getSpace();
    tsDataset->read(tsBuffer, H5::PredType::NATIVE_DOUBLE, tsfSpace, tsfSpace);

    std::vector<double> timestampsOut(tsBuffer, tsBuffer + numSamples);
    delete[] tsBuffer;
    double tolerance = 1e-9;
    REQUIRE_THAT(timestampsOut,
                 Catch::Matchers::Approx(mockTimestamps).margin(tolerance));
  }
}
