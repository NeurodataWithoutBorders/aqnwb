#include <catch2/catch_test_macros.hpp>

#include "Channel.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("LinkTimeSeriesExamples", "[timeseries][link]")
{
  SECTION("Link TimeSeries data for time alignment")
  {
    // [example_link_timeseries_setup]
    // Create an NWB file
    std::string path = getTestFilePath("testLinkTimeSeriesExample.nwb");
    auto io = std::make_shared<IO::HDF5::HDF5IO>(path);
    io->open();

    auto nwbfile = NWB::NWBFile::create(io);
    nwbfile->initialize(generateUuid());

    // Create electrodes table
    auto electrodesTable = nwbfile->createElectrodesTable();
    REQUIRE(electrodesTable != nullptr);

    // Add electrode
    std::vector<std::string> location = {"CA1"};
    std::vector<std::string> group = {"/general/extracellular_ephys/electrodes"};
    std::vector<float> groupPosition = {0.0f, 0.0f, 0.0f};
    auto devicePath = nwbfile->createDevicePath("Device");
    auto groupPath = nwbfile->createElectrodeGroupPath(
        "my_electrode_group", devicePath, "my description", location[0]);
    electrodesTable->initialize();
    electrodesTable->addElectrodes(
        1, location, group, groupPosition, groupPath);
    // [example_link_timeseries_setup]

    // [example_link_timeseries_original]
    // Create the original ElectricalSeries with actual data during acquisition
    SizeType numSamples = 1000;
    SizeType numChannels = 1;
    IO::BaseDataType dataType = IO::BaseDataType::F32;

    // Define the channels to record
    std::vector<Types::ChannelVector> recordingArrays = {
        {{0}, "ElectricalSeries", {numSamples}}};
    std::vector<std::string> recordingNames = {"raw_voltage"};

    // Create the ElectricalSeries using NWBFile helper
    Status status = nwbfile->createElectricalSeries(
        recordingArrays, recordingNames, dataType, "/acquisition");
    REQUIRE(status == Status::Success);

    // Get the created ElectricalSeries
    auto originalSeries = io->getRecordingContainers()->getContainer<
        NWB::ElectricalSeries>("/acquisition/ElectricalSeries#raw_voltage");
    REQUIRE(originalSeries != nullptr);

    // Write data to the original series
    std::vector<float> data(numSamples);
    std::vector<double> timestamps(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
      data[i] = static_cast<float>(i) * 0.1f;  // Mock voltage data
      timestamps[i] = static_cast<double>(i) * 0.001;  // 1 ms sampling
    }

    originalSeries->writeChannel(0, numSamples, data.data(), timestamps.data());
    // [example_link_timeseries_original]

    // [example_link_timeseries_processing_module]
    // Create a ProcessingModule for time-aligned data
    auto processingModule =
        nwbfile->createProcessingModule("time_alignment");
    REQUIRE(processingModule != nullptr);
    processingModule->initialize(
        "Time-aligned data relative to stimulus onset");
    // [example_link_timeseries_processing_module]

    // [example_link_timeseries_linked]
    // Create a linked ElectricalSeries in the ProcessingModule
    auto linkedSeries =
        processingModule->createNWBDataInterface<NWB::ElectricalSeries>(
            "aligned_voltage");
    REQUIRE(linkedSeries != nullptr);

    // Create link configuration pointing to the original data
    std::string linkTarget =
        "/acquisition/ElectricalSeries#raw_voltage/data";
    IO::LinkArrayDataSetConfig linkConfig(linkTarget);

    // Get the channel vector from the electrodes table
    Types::ChannelVector channelVector = {{0}, "ElectricalSeries", {numSamples}};

    // Initialize the linked ElectricalSeries using the link configuration
    // Note: ElectricalSeries::initialize automatically queries shape and
    // chunking from the linked dataset to properly configure related datasets
    linkedSeries->initialize(linkConfig,  // Use link instead of creating new data
                            channelVector,
                            "Time-aligned voltage data",
                            1.0f,  // conversion
                            -1.0f,  // resolution
                            0.0f);  // offset

    // Write only the new timestamps (data is linked)
    std::vector<double> newTimestamps(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
      newTimestamps[i] =
          static_cast<double>(i) * 0.001 + 5.0;  // Offset by 5 seconds
    }

    auto timestampRecorder = linkedSeries->recordTimestamps();
    timestampRecorder->writeDataBlock(
        {numSamples}, {0}, IO::BaseDataType::F64, newTimestamps.data());
    // [example_link_timeseries_linked]

    // [example_link_timeseries_reference]
    // Create a link to the original series in the ProcessingModule to make the
    // relationship explicit
    std::string originalSeriesPath =
        "/acquisition/ElectricalSeries#raw_voltage";
    std::string referenceLinkPath =
        "/processing/time_alignment/original_series";
    io->createLink(referenceLinkPath, originalSeriesPath);
    // [example_link_timeseries_reference]

    // [example_link_timeseries_cleanup]
    io->stopRecording();
    io->close();
    // [example_link_timeseries_cleanup]

    // Verify the file was created
    REQUIRE(std::filesystem::exists(path));
  }
}
