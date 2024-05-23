
#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Types.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"
#include "nwb/NWBRecording.hpp"

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
    int numChannels = 4;
    int numSamples = 1000;
    int samplesRecorded = 0;
    std::size_t bufferSize = numSamples / 10;
    std::vector<float> dataBuffer(bufferSize);
    std::vector<double> timestampsBuffer(bufferSize);

    std::vector<Types::ChannelGroup> mockRecordingArrays = getMockChannelArrays();
    std::vector<std::vector<float>> mockData = getMockData(numChannels, numSamples);
    std::vector<double> mockTimestamps = getMockTimestamps(numSamples);

    // open files
    NWB::NWBRecording nwbRecording;
    nwbRecording.openFiles(path, "Recording", 1, mockRecordingArrays);

    // run recording
    bool isRecording = true;
    while (isRecording)
    {
      // write data to the file for each channel
    for (size_t i = 0; i < mockRecordingArrays.size(); ++i) {
      const auto& channelGroup = mockRecordingArrays[i];
      for (const auto& channel : channelGroup) {
        // copy data into buffer
        std::copy(mockData[channel.globalIndex].begin(), mockData[channel.globalIndex].begin() + numSamples / 10, dataBuffer.begin());
        std::copy(mockTimestamps.begin(), mockTimestamps.begin() + numSamples / 10, timestampsBuffer.begin());

        // write timseries data
        nwbRecording.writeTimeseriesData(i, channel, dataBuffer.data(), timestampsBuffer.data(), dataBuffer.size());

        // check if recording is done
        samplesRecorded += dataBuffer.size();
        if (samplesRecorded > numSamples)
        {
          isRecording = false;
        }
      }
      }
    }
  }

  // TODO - what other tests should be added here?
  SECTION("test data and timestamps stream")
  {

  }
}