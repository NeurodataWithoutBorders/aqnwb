
#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Types.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/NWBRecording.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("writeContinuousData", "[recording]")
{
  SECTION("test data and timestamps stream")
  {
    // get file path and remove if exists
    std::string path = getTestFilePath("testContinuous");
    if (fs::exists(path + "Recording1.nwb")) {
      fs::remove(path + "Recording1.nwb");
    }

    // setup mock data
    SizeType numChannels = 4;
    SizeType numSamples = 300;
    SizeType samplesRecorded = 0;
    SizeType bufferSize = numSamples / 10;
    std::vector<float> dataBuffer(bufferSize);
    std::vector<double> timestampsBuffer(bufferSize);

    std::vector<Types::ChannelGroup> mockRecordingArrays =
        getMockChannelArrays();
    std::vector<std::vector<float>> mockData =
        getMockData2D(numSamples, numChannels);
    std::vector<double> mockTimestamps = getMockTimestamps(numSamples);

    // open files
    NWB::NWBRecording nwbRecording;
    nwbRecording.openFile(path, "Recording", 1, mockRecordingArrays);

    // run recording
    bool isRecording = true;
    while (isRecording) {
      // write data to the file for each channel
      for (SizeType i = 0; i < mockRecordingArrays.size(); ++i) {
        const auto& channelGroup = mockRecordingArrays[i];
        for (const auto& channel : channelGroup) {
          // copy data into buffer
          std::copy(mockData[channel.globalIndex].begin() + samplesRecorded,
                    mockData[channel.globalIndex].begin() + samplesRecorded
                        + bufferSize,
                    dataBuffer.begin());
          std::copy(mockTimestamps.begin() + samplesRecorded,
                    mockTimestamps.begin() + samplesRecorded + bufferSize,
                    timestampsBuffer.begin());

          // write timseries data
          std::vector<SizeType> positionOffset = {samplesRecorded,
                                                  channel.localIndex};
          nwbRecording.writeTimeseriesData(i,
                                           channel,
                                           dataBuffer.data(),
                                           timestampsBuffer.data(),
                                           dataBuffer.size(),
                                           positionOffset);
        }
      }
      // check if recording is done
      samplesRecorded += dataBuffer.size();
      if (samplesRecorded >= numSamples) {
        isRecording = false;
      }
    }
    nwbRecording.closeFile();

    // check contents of data
    std::string dataPath = "/acquisition/array0/data";
    std::unique_ptr<H5::H5File> file =
        std::make_unique<H5::H5File>(path + "Recording1.nwb", H5F_ACC_RDONLY);
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
        dataOut[i][j] =
            buffer[j * numChannelsToRead + i] * (32767.0f * 0.000002f);
      }
    }
    delete[] buffer;
    REQUIRE_THAT(dataOut[0], Catch::Matchers::Approx(mockData[0]).margin(1));
    REQUIRE_THAT(dataOut[1], Catch::Matchers::Approx(mockData[1]).margin(1));

    // check contents of timestamps
    std::string timestampsPath = "/acquisition/array0/timestamps";
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

  SECTION("add a new recording number to the same file", "[recording]")
  {
    // TODO
  }
}
