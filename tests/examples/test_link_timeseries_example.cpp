#include <catch2/catch_test_macros.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/base/TimeSeries.hpp"
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

    // [example_link_timeseries_setup]

    // [example_link_timeseries_original]
    // Create the original TimeSeries with actual data during acquisition
    auto originalSeries =
        NWB::TimeSeries::create("/acquisition/original_series", io);
    REQUIRE(originalSeries != nullptr);

    // Generate sample data
    SizeType numSamples = 1000;
    std::vector<float> data(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
      data[i] = static_cast<float>(i) * 0.1f;  // Mock data
    }

    // Create configuration for the original data
    IO::ArrayDataSetConfig dataConfig(
        IO::BaseDataType::F32,  // Data type
        SizeArray {0},  // Shape: extendable in time dimension
        SizeArray {1},  // Chunking
        SizeArray {numSamples});  // Max dimensions

    // Initialize the TimeSeries with data
    originalSeries->initialize(
        dataConfig,  // Data configuration
        "mV",  // Unit
        "Original voltage recording");  // Description

    // Write data using constant sampling rate (1000 Hz = 1ms sampling)
    double startingTime = 0.0;  // Start at 0 seconds
    double samplingRate = 1000.0;  // 1000 Hz
    originalSeries->writeTimestampedData(
        numSamples, data.data(), startingTime, samplingRate);
    // [example_link_timeseries_original]

    // [example_link_timeseries_processing_module]
    // Create a ProcessingModule for time-aligned data
    auto processingModule = nwbfile->createProcessingModule("time_alignment");
    REQUIRE(processingModule != nullptr);
    processingModule->initialize(
        "Time-aligned data relative to stimulus onset");
    // [example_link_timeseries_processing_module]

    // [example_link_timeseries_linked]
    // Create a linked TimeSeries in the ProcessingModule
    auto linkedSeries =
        processingModule->createNWBDataInterface<NWB::TimeSeries>(
            "aligned_voltage");
    REQUIRE(linkedSeries != nullptr);

    // Create link configuration pointing to the original data
    std::string linkTarget = "/acquisition/original_series/data";
    IO::LinkArrayDataSetConfig linkConfig(linkTarget);

    // Initialize the linked TimeSeries using the link configuration
    // Note: TimeSeries::initialize automatically queries shape and chunking
    // from the linked dataset
    linkedSeries->initialize(
        linkConfig,  // Use link instead of creating new data
        "mV",  // Same unit as original
        "Time-aligned voltage data");  // Description

    // Write the adjusted timestamps (data is linked)
    // Simulate time alignment with small adjustments to demonstrate irregular
    // sampling that would result from aligning to stimulus events
    std::vector<double> newTimestamps(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
      // Base offset of 5 seconds plus small jitter to make timestamps irregular
      double baseTime = 5.0 + static_cast<double>(i) * 0.001;
      double jitter = (i % 10) * 0.00001;  // Small periodic adjustment
      newTimestamps[i] = baseTime + jitter;
    }

    auto timestampRecorder = linkedSeries->recordTimestamps();
    timestampRecorder->writeDataBlock(
        {numSamples}, {0}, IO::BaseDataType::F64, newTimestamps.data());
    // [example_link_timeseries_linked]

    // [example_link_timeseries_reference]
    // Create a link to the original series in the ProcessingModule to make the
    // relationship explicit
    std::string originalSeriesPath = "/acquisition/original_series";
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
