#include <numeric>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/nwbio_utils.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ophys/OphysUtils.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("ophysWorkflowExamples", "[ophys]")
{
  SECTION("two-photon imaging workflow")
  {
    std::string path = getTestFilePath("exampleOphysRecording.nwb");

    // [example_ophys_nwbfile_snippet]
    std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
    io->open();
    auto nwbfile = NWB::NWBFile::create(io);
    std::string currentTime = getCurrentTime();
    Status status = nwbfile->initialize(generateUuid(),
                                        "a two-photon imaging session",
                                        "",
                                        currentTime,
                                        currentTime,
                                        getTestSubjectSpec());
    // [example_ophys_nwbfile_snippet]
    REQUIRE(status == Status::Success);

    // [example_ophys_device_snippet]
    // Create the microscope used for imaging
    auto microscope = NWB::Device::create("/general/devices/microscope", io);
    REQUIRE(microscope->initialize(NWB::Device::Spec()
                                       .withDescription("Two-photon microscope")
                                       .withManufacturer("Example Inc."))
            == Status::Success);
    // [example_ophys_device_snippet]
    REQUIRE(microscope != nullptr);

    // [example_ophys_imaging_plane_snippet]
    // Create the imaging plane with one optical channel
    auto imagingPlane = NWB::Ophys::createImagingPlane(
        *nwbfile,
        "imaging_plane",
        NWB::ImagingPlane::Spec()
            .withDescription("Imaging plane in layer 2/3 of V1")
            .withExcitationLambda(920.0f)
            .withIndicator("GCaMP6s")
            .withLocation("VISp")
            .withImagingRate(30.0f)
            .withGridSpacing(std::vector<float> {2.0e-6f, 2.0e-6f})
            .withDevice(microscope)
            .withOpticalChannel("green",
                                NWB::OpticalChannel::Spec()
                                    .withDescription("Green channel")
                                    .withEmissionLambda(510.0f)));
    // [example_ophys_imaging_plane_snippet]
    REQUIRE(imagingPlane != nullptr);

    // [example_ophys_two_photon_series_snippet]
    // Create the TwoPhotonSeries for frames of 32 x 24 pixels acquired at
    // 30 Hz
    const SizeArray frameShape = {32, 24};
    auto series = NWB::Ophys::createTwoPhotonSeries(
        *nwbfile,
        "TwoPhotonSeries",
        NWB::TwoPhotonSeries::Spec()
            .withData(NWB::Ophys::getDefaultImageSeriesDataConfig(
                IO::BaseDataType::U16, frameShape))
            .withUnit("n.a.")
            .withDescription("Raw two-photon imaging data")
            .withStartingTime(0.0)
            .withStartingTimeRate(30.0f)
            .withImagingPlane(imagingPlane)
            .withDevice(microscope)
            .withFieldOfView(std::vector<float> {1.28e-4f, 0.96e-4f}));
    // [example_ophys_two_photon_series_snippet]
    REQUIRE(series != nullptr);

    // [example_ophys_record_snippet]
    // Start the recording (e.g., to enable SWMR mode for HDF5)
    io->startRecording();

    // Write frames as they are acquired
    const SizeType containerIndex = series->getRecordingObjectIndex();
    const SizeType numFramesPerBlock = 10;
    std::vector<uint16_t> frames(numFramesPerBlock * 32 * 24);
    for (SizeType block = 0; block < 4; ++block) {
      std::iota(frames.begin(), frames.end(), static_cast<uint16_t>(block));
      // No timestamps are needed since the series uses a starting time and
      // rate. Pass the timestamps of the frames as last argument otherwise.
      status = IO::appendTimeSeriesData(io->getRecordingObjects(),
                                        containerIndex,
                                        numFramesPerBlock,
                                        frames.data());
      REQUIRE(status == Status::Success);
    }

    io->stopRecording();
    io->close();
    // [example_ophys_record_snippet]

    // Check the data that has been written
    std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
    readIO->open(IO::FileMode::ReadOnly);
    auto readSeries = NWB::RegisteredType::create<NWB::TwoPhotonSeries>(
        "/acquisition/TwoPhotonSeries", readIO);
    auto data = readSeries->readData<uint16_t>()->values();
    REQUIRE(data.shape == SizeArray {40, 32, 24});
    REQUIRE(readSeries->readStartingTime()->exists());
    REQUIRE(readSeries->readImagingPlane()->readIndicator()->values().data[0]
            == "GCaMP6s");
    readIO->close();
  }
}
