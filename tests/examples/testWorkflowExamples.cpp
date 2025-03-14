
#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("workflowExamples")
{
  SECTION("write workflow")
  {
    // 0. setup mock data
    SizeType numChannels = 4;
    SizeType numSamples = 300;
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

    std::string path = getTestFilePath("exampleRecording.nwb");
    // [example_workflow_io_snippet]
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    REQUIRE(io->isOpen());
    // [example_workflow_io_snippet]

    // [example_workflow_recording_containers_snippet]
    std::unique_ptr<NWB::RecordingContainers> recordingContainers =
        std::make_unique<NWB::RecordingContainers>();
    // [example_workflow_recording_containers_snippet]

    // [example_workflow_nwbfile_snippet]
    std::unique_ptr<NWB::NWBFile> nwbfile = std::make_unique<NWB::NWBFile>(io);
    Status initStatus = nwbfile->initialize(generateUuid());
    REQUIRE(initStatus == Status::Success);
    // [example_workflow_nwbfile_snippet]

    // [example_workflow_electrodes_table_snippet]
    Status elecTableStatus =
        nwbfile->createElectrodesTable(mockRecordingArrays);
    REQUIRE(elecTableStatus == Status::Success);
    // [example_workflow_electrodes_table_snippet]

    // [example_workflow_datasets_snippet]
    std::vector<SizeType> containerIndexes;
    Status elecSeriesStatus =
        nwbfile->createElectricalSeries(mockRecordingArrays,
                                        mockChannelNames,
                                        BaseDataType::I16,
                                        recordingContainers.get(),
                                        containerIndexes);
    REQUIRE(elecSeriesStatus == Status::Success);
    // [example_workflow_datasets_snippet]

    // [example_workflow_start_snippet]
    Status startRecordingStatus = io->startRecording();
    REQUIRE(startRecordingStatus == Status::Success);
    // [example_workflow_start_snippet]

    // write data during the recording
    bool isRecording = true;
    while (isRecording) {
      // write data to the file for each channel
      for (SizeType i = 0; i < containerIndexes.size(); ++i) {
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
          std::unique_ptr<int16_t[]> intBuffer = transformToInt16(
              dataBuffer.size(), channel.getBitVolts(), dataBuffer.data());

          // [example_workflow_write_snippet]
          recordingContainers->writeTimeseriesData(containerIndexes[i],
                                                   channel,
                                                   dataShape,
                                                   positionOffset,
                                                   intBuffer.get(),
                                                   timestampsBuffer.data());
          io->flush();
          // [example_workflow_write_snippet]
        }
      }
      // check if recording is done
      samplesRecorded += dataBuffer.size();
      if (samplesRecorded >= numSamples) {
        isRecording = false;
      }
    }

    // [example_workflow_stop_snippet]
    io->stopRecording();
    nwbfile->finalize();
    io->close();
    // [example_workflow_stop_snippet]
  }
}
