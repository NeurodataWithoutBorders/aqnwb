// [example_all]
#include <catch2/catch_test_macros.hpp>

#include "Utils.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "testUtils.hpp"

#ifdef AQNWB_HAVE_REMFILE_VFD
TEST_CASE("remfile read example", "[hdf5io]")
{
  SECTION("remfileDocsExample")
  {
    // [example_remfile_setup]
    std::string s3Url =
        "https://dandiarchive.s3.amazonaws.com/blobs/fec/8a6/"
        "fec8a690-2ece-4437-8877-8a002ff8bd8a";
    auto readio = std::make_shared<AQNWB::IO::HDF5::HDF5IO>(s3Url);
    Status status = readio->openRemote();
    // [example_remfile_setup]
    REQUIRE(status == Status::Success);
    REQUIRE(readio->isOpen());
    REQUIRE(readio->canModifyObjects() == false);

    // [example_remfile_nwbfile_read]
    // Note, no data is actually downloaded here.
    auto nwbFile = AQNWB::NWB::NWBFile::create("/", readio);
    // [example_remfile_nwbfile_read]

    // [example_remfile_find_objects]
    // Find all TimeSeries objects in the file
    auto timeSeriesFound = nwbFile->findOwnedTypes(
        {"core::TimeSeries"}, IO::SearchMode::CONTINUE_ON_TYPE);
    REQUIRE(timeSeriesFound.size() == 1);
    // Open the TimeSeries object
    auto timeSeries =
        AQNWB::NWB::TimeSeries::create(timeSeriesFound.begin()->first, readio);
    REQUIRE(timeSeries != nullptr);
    // [example_remfile_find_objects]
    // [example_remfile_read_data]
    // Read the first 10 samples from the first channel of the TimeSeries.data
    auto readWrapper = timeSeries->readData<int16_t>();
    SizeArray start = {0, 0};
    SizeArray count = {10, 1};
    auto dataSlice = readWrapper->values(start, count);
    REQUIRE(dataSlice.data.size() == 10);
    readio->close();
    // [example_remfile_read_data]
  }
}
#endif