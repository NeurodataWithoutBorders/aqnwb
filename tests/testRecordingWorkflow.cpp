
#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/RecordingObjects.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "io/nwbio_utils.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/file/ElectrodesTable.hpp"
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

    // 2. RecordingObjects are now automatically managed by the IO object
    std::vector<SizeType> containerIndices = {};

    // 3. create NWBFile object
    auto nwbfile = NWB::NWBFile::create(io);
    nwbfile->initialize(generateUuid());

    // 4. create an electrodes table.
    nwbfile->createElectrodesTable(mockRecordingArrays);

    // 5. create datasets (automatically added to recording objects)
    std::vector<SizeType> containerIndexes = {};
    nwbfile->createElectricalSeries(mockRecordingArrays,
                                    mockChannelNames,
                                    BaseDataType::F32,
                                    containerIndexes);

    // 6. start the recording
    io->startRecording();

    // 7. write data during the recording
    bool isRecording = true;
    while (isRecording) {
      SizeType remainingSamples = numSamples - samplesRecorded;
      SizeType samplesToWrite = std::min(bufferSize, remainingSamples);

      // write data to the file for each channel
      for (SizeType i = 0; i < mockRecordingArrays.size(); ++i) {
        SizeType recordingObjectIndex = containerIndexes[i];
        const auto& channelVector = mockRecordingArrays[i];
        for (const auto& channel : channelVector) {
          // copy data into buffer
          std::copy(mockData[channel.getGlobalIndex()].begin()
                        + static_cast<std::ptrdiff_t>(samplesRecorded),
                    mockData[channel.getGlobalIndex()].begin()
                        + static_cast<std::ptrdiff_t>(samplesRecorded
                                                      + samplesToWrite),
                    dataBuffer.begin());
          std::copy(mockTimestamps.begin()
                        + static_cast<std::ptrdiff_t>(samplesRecorded),
                    mockTimestamps.begin()
                        + static_cast<std::ptrdiff_t>(samplesRecorded
                                                      + samplesToWrite),
                    timestampsBuffer.begin());

          // write timeseries data
          std::vector<SizeType> positionOffset = {samplesRecorded,
                                                  channel.getLocalIndex()};
          std::vector<SizeType> dataShape = {samplesToWrite, 1};

          auto recordingObjects = io->getRecordingObjects();
          IO::writeTimeseriesData(recordingObjects,
                                  recordingObjectIndex,
                                  channel,
                                  dataShape,
                                  positionOffset,
                                  dataBuffer.data(),
                                  timestampsBuffer.data());
        }
      }
      // check if recording is done
      samplesRecorded += samplesToWrite;
      if (samplesRecorded >= numSamples) {
        isRecording = false;
      }
    }

    // 8. stop the recording and finalize the file
    io->stopRecording();
    io->close();

    // check contents of data
    std::string dataPath = "/acquisition/esdata0/data";
    std::unique_ptr<H5::H5File> file =
        std::make_unique<H5::H5File>(path, H5F_ACC_RDONLY);
    std::unique_ptr<H5::DataSet> dataset =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath));
    SizeType numChannelsToRead = numChannels / 2;

    std::vector<float> buffer(numSamples * numChannelsToRead);
    H5::DataSpace fSpace = dataset->getSpace();
    hsize_t dims[1] = {numSamples * numChannelsToRead};
    H5::DataSpace mSpace(1, dims);
    dataset->read(buffer.data(), H5::PredType::NATIVE_FLOAT, mSpace, fSpace);

    std::vector<std::vector<float>> dataOut(numChannelsToRead,
                                            std::vector<float>(numSamples));
    for (SizeType i = 0; i < numChannelsToRead; ++i) {
      for (SizeType j = 0; j < numSamples; ++j) {
        dataOut[i][j] = buffer[static_cast<size_t>(j) * numChannelsToRead + i];
      }
    }
    REQUIRE_THAT(dataOut[0], Catch::Matchers::Approx(mockData[0]).margin(1.0));
    REQUIRE_THAT(dataOut[1], Catch::Matchers::Approx(mockData[1]).margin(1.0));

    // check contents of timestamps
    std::string timestampsPath = "/acquisition/esdata0/timestamps";
    std::unique_ptr<H5::DataSet> tsDataset =
        std::make_unique<H5::DataSet>(file->openDataSet(timestampsPath));
    std::vector<double> tsBuffer(numSamples);

    H5::DataSpace tsfSpace = tsDataset->getSpace();
    tsDataset->read(
        tsBuffer.data(), H5::PredType::NATIVE_DOUBLE, tsfSpace, tsfSpace);

    std::vector<double> timestampsOut(tsBuffer.begin(), tsBuffer.end());
    double tolerance = 1e-9;
    REQUIRE_THAT(timestampsOut,
                 Catch::Matchers::Approx(mockTimestamps).margin(tolerance));
  }
}
