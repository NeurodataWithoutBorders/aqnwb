#include <catch2/catch_test_macros.hpp>

#include "Channel.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("LinkTimeSeriesExamples", "[timeseries][link]")
{
  SECTION("Link TimeSeries data for time alignment")
  {
    std::string path = getTestFilePath("testLinkTimeSeriesExample.h5");

    // [example_link_timeseries_setup]
    // Create an NWB file
    auto io = std::make_shared<IO::HDF5::HDF5IO>(path);
    io->open();

    auto nwbfile = NWB::NWBFile::create(io);
    nwbfile->initialize(generateUuid());
    // [example_link_timeseries_setup]

    // [example_link_timeseries_original]
    // Create the original TimeSeries with actual data
    auto originalSeries =
        NWB::TimeSeries::create("/acquisition/original_series", io);

    SizeType numSamples = 1000;
    IO::BaseDataType dataType = IO::BaseDataType::F32;
    IO::ArrayDataSetConfig dataConfig(dataType, SizeArray {0}, SizeArray {100});

    originalSeries->initialize(dataConfig,
                               "volts",
                               "Original electrical recording",
                               "Recorded from electrode array",
                               1.0f,  // conversion
                               -1.0f,  // resolution
                               0.0f  // offset
    );

    // Write data to the original series
    std::vector<float> data(numSamples);
    std::vector<double> timestamps(numSamples);
    for (size_t i = 0; i < numSamples; ++i) {
      data[i] = static_cast<float>(i) * 0.1f;
      timestamps[i] = static_cast<double>(i) * 0.001;  // 1 ms sampling
    }

    originalSeries->writeData(
        {numSamples}, {0}, data.data(), timestamps.data());
    // [example_link_timeseries_original]

    // [example_link_timeseries_linked]
    // Create a linked TimeSeries with different timestamps
    auto linkedSeries =
        NWB::TimeSeries::create("/acquisition/resampled_series", io);

    // Create link configuration pointing to the original data
    std::string linkTarget = "/acquisition/original_series/data";
    IO::LinkArrayDataSetConfig linkConfig(linkTarget);

    linkedSeries->initialize(
        linkConfig,  // Use link instead of creating new data
        "volts",
        "Resampled version with same data but different timestamps",
        "Time-aligned to stimulus onset",
        1.0f,
        -1.0f,
        0.0f);

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

    // [example_link_timeseries_cleanup]
    io->stopRecording();
    io->close();
    // [example_link_timeseries_cleanup]

    // Verify the file was created
    REQUIRE(std::filesystem::exists(path));
  }
}
