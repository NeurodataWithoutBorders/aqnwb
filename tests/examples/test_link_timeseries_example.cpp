#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;
using namespace AQNWB::NWB;

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
    auto status = nwbfile->initialize(generateUuid());
    REQUIRE(status == Status::Success);
    // [example_link_timeseries_setup]

    // [example_link_timeseries_original]
    // Create the original TimeSeries with actual data during acquisition
    std::string originalSeriesPath =
        mergePaths(NWBFile::ACQUISITION_PATH, "original_series");
    auto originalSeries = NWB::TimeSeries::create(originalSeriesPath, io);
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
        SizeArray {1000});  // Chunking

    // Initialize the TimeSeries with data constant sampling rate
    double startingTime = 0.0;  // Start at 0 seconds
    float samplingRate = 1000.0;  // 1000 Hz
    status = originalSeries->initialize(
        dataConfig,  // Data configuration
        "m/s",  // unit for speed of the animal
        "Original speed recording of the animal",  // description
        "Coarse aligned with starting time but not aligned to "
        "stimulus events",  // comment
        1.0f,  // conversion
        -1.0f,  // resolution (not specified)
        0.0f,  // offset
        TimeSeries::ContinuityType::Continuous,  // continuity
        startingTime,  // starting time
        samplingRate  // sampling rate
    );
    REQUIRE(status == Status::Success);

    // Write data. No timestamps needed since we have regular sampling rate
    status = originalSeries->writeData({numSamples},  // dataShape
                                       {0},  // positionOffset
                                       data.data()  // dataInput
    );
    REQUIRE(status == Status::Success);
    // [example_link_timeseries_original]

    // [example_link_timeseries_processing_module]
    // Create a ProcessingModule for time-aligned data
    auto processingModule = nwbfile->createProcessingModule("time_alignment");
    REQUIRE(processingModule != nullptr);
    status = processingModule->initialize(
        "Time-aligned data relative to stimulus onset");
    REQUIRE(status == Status::Success);
    // [example_link_timeseries_processing_module]

    // [example_link_timeseries_linked]
    // Create a TimeSeries in the ProcessingModule for the time-aligned data
    // The TimeSeries will link to the original data and have its own timestamps
    // reflecting the post-hoc alignment to stimulus events
    auto linkedSeries =
        processingModule->createNWBDataInterface<NWB::TimeSeries>(
            "aligned_voltage");
    REQUIRE(linkedSeries != nullptr);

    // Create link configuration pointing to the original data
    std::string linkTarget = mergePaths(originalSeriesPath, "data");
    IO::LinkArrayDataSetConfig linkConfig(linkTarget);

    // Initialize the linked TimeSeries using the link configuration
    // Note: TimeSeries::initialize automatically queries shape and chunking
    // from the linked dataset to configure related datasets like timestamps
    // accordingly.
    status = linkedSeries->initialize(
        linkConfig,  // Use link instead of creating new data
        originalSeries->readDataUnit()
            ->values()
            .data[0],  // Same unit as original
        "Time-aligned voltage data",  // Description
        "Aligned to stimulus events with irregular timestamps",  // Comment
        1.0f,  // conversion
        -1.0f,  // resolution (not specified)
        0.0f,  // offset
        TimeSeries::ContinuityType::Continuous  // continuity
        // no sampling rate or starting time needed since we use timestamps
    );
    REQUIRE(status == Status::Success);

    // Simulate time alignment with small adjustments to demonstrate irregular
    // sampling that would result from aligning to stimulus events
    std::vector<double> newTimestamps(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
      // Base offset of 5 seconds plus small jitter to make timestamps irregular
      double baseTime = 5.0 + static_cast<double>(i) * 0.001;
      double jitter = static_cast<double>(i % 10) * 0.00001;
      newTimestamps[i] = baseTime + jitter;
    }

    // Write the adjusted timestamps to the aligned_voltage TimeSeries
    auto timestampRecorder = linkedSeries->recordTimestamps();
    status = timestampRecorder->writeDataBlock(
        {numSamples}, {0}, IO::BaseDataType::F64, newTimestamps.data());
    REQUIRE(status == Status::Success);
    // [example_link_timeseries_linked]

    // [example_link_timeseries_reference]
    // Create a link to the original series in the ProcessingModule to make the
    // relationship explicit
    std::string referenceLinkPath =
        mergePaths(processingModule->getPath(), "original_series_reference");
    status = io->createLink(referenceLinkPath, originalSeries->getPath());
    REQUIRE(status == Status::Success);
    // [example_link_timeseries_reference]

    // [example_link_timeseries_cleanup]
    io->stopRecording();
    io->close();
    // [example_link_timeseries_cleanup]

    // Verify the file was created
    REQUIRE(std::filesystem::exists(path));
  }
}
