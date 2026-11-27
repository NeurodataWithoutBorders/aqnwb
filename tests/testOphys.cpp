#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/image/ImageSeries.hpp"
#include "nwb/ophys/ImagingPlane.hpp"
#include "nwb/ophys/OnePhotonSeries.hpp"
#include "nwb/ophys/OphysUtils.hpp"
#include "nwb/ophys/OpticalChannel.hpp"
#include "nwb/ophys/TwoPhotonSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

namespace
{
/**
 * @brief Create and initialize an NWBFile for the ophys tests.
 */
std::shared_ptr<NWB::NWBFile> createOphysTestFile(
    const std::shared_ptr<IO::BaseIO>& io)
{
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  std::string currentTime = getCurrentTime();
  REQUIRE(nwbfile->initialize(generateUuid(),
                              "ophys test",
                              "",
                              currentTime,
                              currentTime,
                              getTestSubjectSpec())
          == Status::Success);
  return nwbfile;
}

/**
 * @brief Get a valid spec of an optical channel.
 */
NWB::OpticalChannel::Spec getTestOpticalChannelSpec()
{
  return NWB::OpticalChannel::Spec()
      .withDescription("green channel")
      .withEmissionLambda(510.0f);
}

/**
 * @brief Get a valid spec of an imaging plane for the given device.
 */
NWB::ImagingPlane::Spec getTestImagingPlaneSpec(
    const std::shared_ptr<NWB::Device>& device)
{
  return NWB::ImagingPlane::Spec()
      .withExcitationLambda(920.0f)
      .withIndicator("GCaMP6s")
      .withLocation("VISp")
      .withDevice(device)
      .withOpticalChannel("green", getTestOpticalChannelSpec());
}
}  // namespace

TEST_CASE("registered ophys types", "[ophys]")
{
  auto registry = NWB::RegisteredType::getRegistry();
  for (const std::string& type : {"core::ImageSeries",
                                  "core::TwoPhotonSeries",
                                  "core::OnePhotonSeries",
                                  "core::ImagingPlane",
                                  "core::OpticalChannel"})
  {
    INFO(type);
    REQUIRE(registry.find(type) != registry.end());
  }
}

TEST_CASE("ophys spec validation", "[ophys]")
{
  std::string path = getTestFilePath("testOphysSpecValidation.h5");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  auto device = NWB::Device::Spec().createAndInitialize("/device", io);
  REQUIRE(device != nullptr);

  SECTION("OpticalChannel")
  {
    REQUIRE(getTestOpticalChannelSpec().validate() == Status::Success);
    REQUIRE(NWB::OpticalChannel::Spec().withEmissionLambda(500.0f).validate()
            == Status::Failure);  // missing description
    REQUIRE(NWB::OpticalChannel::Spec().withDescription("d").validate()
            == Status::Failure);  // missing emission wavelength
  }

  SECTION("ImagingPlane")
  {
    REQUIRE(getTestImagingPlaneSpec(device).validate() == Status::Success);
    REQUIRE(getTestImagingPlaneSpec(nullptr).validate() == Status::Failure);
    REQUIRE(getTestImagingPlaneSpec(device).withIndicator("").validate()
            == Status::Failure);
    REQUIRE(getTestImagingPlaneSpec(device)
                .withOriginCoords(std::vector<float> {1.0f})
                .validate()
            == Status::Failure);
    REQUIRE(getTestImagingPlaneSpec(device).withOpticalChannels({}).validate()
            == Status::Failure);  // no optical channel
    REQUIRE(getTestImagingPlaneSpec(device)
                .withOpticalChannel("red", NWB::OpticalChannel::Spec())
                .validate()
            == Status::Failure);  // invalid optical channel
  }

  SECTION("TwoPhotonSeries")
  {
    auto imagingPlane = getTestImagingPlaneSpec(device).createAndInitialize(
        "/imaging_plane", io);
    REQUIRE(imagingPlane != nullptr);
    auto validSpec = NWB::TwoPhotonSeries::Spec()
                         .withData(NWB::Ophys::getDefaultImageSeriesDataConfig(
                             IO::BaseDataType::U16, {8, 8}))
                         .withUnit("n.a.")
                         .withImagingPlane(imagingPlane);
    REQUIRE(validSpec.validate() == Status::Success);
    REQUIRE(NWB::TwoPhotonSeries::Spec(validSpec)
                .withImagingPlane(nullptr)
                .validate()
            == Status::Failure);
    REQUIRE(NWB::TwoPhotonSeries::Spec(validSpec)
                .withData(IO::ArrayDataSetConfig(
                    IO::BaseDataType::U16, {0, 8}, {1, 8}))
                .validate()
            == Status::Failure);  // 2D data
    REQUIRE(NWB::TwoPhotonSeries::Spec(validSpec)
                .withData(IO::ArrayDataSetConfig(
                    IO::BaseDataType::V_STR, {0, 8, 8}, {1, 8, 8}))
                .validate()
            == Status::Failure);  // non-numeric data
    REQUIRE(NWB::TwoPhotonSeries::Spec(validSpec)
                .withFieldOfView(std::vector<float> {1.0f})
                .validate()
            == Status::Failure);
    REQUIRE(
        NWB::TwoPhotonSeries::Spec(validSpec).withFormat("external").validate()
        == Status::Failure);  // missing external files

    // Invalid specs do not create any objects
    auto series = NWB::TwoPhotonSeries::create("/invalid_series", io);
    REQUIRE(series->initialize(
                NWB::TwoPhotonSeries::Spec(validSpec).withImagingPlane(nullptr))
            == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/invalid_series"));
  }
  io->close();
}

TEST_CASE("ImagingPlane", "[ophys]")
{
  std::string path = getTestFilePath("testOphysImagingPlane.nwb");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  auto nwbfile = createOphysTestFile(io);
  auto device = NWB::Device::Spec()
                    .withDescription("microscope")
                    .createAndInitialize("/general/devices/microscope", io);
  REQUIRE(device != nullptr);

  auto imagingPlane = NWB::Ophys::createImagingPlane(
      *nwbfile,
      "plane",
      getTestImagingPlaneSpec(device)
          .withDescription("a plane")
          .withImagingRate(30.0f)
          .withOriginCoords(std::vector<float> {1.0f, 2.0f, 3.0f})
          .withOriginCoordsUnit("millimeters")
          .withGridSpacing(std::vector<float> {0.5f, 0.5f})
          .withReferenceFrame("relative to bregma")
          .withOpticalChannel("red",
                              NWB::OpticalChannel::Spec()
                                  .withDescription("red channel")
                                  .withEmissionLambda(600.0f)));
  REQUIRE(imagingPlane != nullptr);
  REQUIRE(imagingPlane->getPath() == "/general/optophysiology/plane");

  // The same name cannot be used twice
  REQUIRE(NWB::Ophys::createImagingPlane(
              *nwbfile, "plane", getTestImagingPlaneSpec(device))
          == nullptr);
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readPlane = NWB::RegisteredType::create<NWB::ImagingPlane>(
      "/general/optophysiology/plane", readIO);
  REQUIRE(readPlane != nullptr);
  REQUIRE(readPlane->readDescription()->values().data[0] == "a plane");
  REQUIRE(std::abs(readPlane->readExcitationLambda()->values().data[0] - 920.0f)
          < 1e-6f);
  auto excitationLambda = readPlane->readExcitationLambda()->values();
  REQUIRE(excitationLambda.shape.empty());  // scalar dataset
  REQUIRE(std::abs(readPlane->readImagingRate()->values().data[0] - 30.0f)
          < 1e-6f);
  REQUIRE(readPlane->readIndicator()->values().data[0] == "GCaMP6s");
  REQUIRE(readPlane->readLocation()->values().data[0] == "VISp");
  REQUIRE(readPlane->readOriginCoords()->values().data
          == std::vector<float> {1.0f, 2.0f, 3.0f});
  REQUIRE(readPlane->readOriginCoordsUnit()->values().data[0] == "millimeters");
  REQUIRE(readPlane->readGridSpacingUnit()->values().data[0] == "meters");
  REQUIRE(readPlane->readReferenceFrame()->values().data[0]
          == "relative to bregma");
  REQUIRE(readPlane->readDevice()->readDescription()->values().data[0]
          == "microscope");

  auto green = readPlane->readOpticalChannel("green");
  REQUIRE(green != nullptr);
  REQUIRE(green->readDescription()->values().data[0] == "green channel");
  REQUIRE(std::abs(green->readEmissionLambda()->values().data[0] - 510.0f)
          < 1e-6f);
  REQUIRE(readPlane->readOpticalChannel("red") != nullptr);
  readIO->close();
}

TEST_CASE("TwoPhotonSeries and OnePhotonSeries", "[ophys]")
{
  std::string path = getTestFilePath("testOphysImageSeries.nwb");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  auto nwbfile = createOphysTestFile(io);
  auto device = NWB::Device::Spec()
                    .withDescription("microscope")
                    .createAndInitialize("/general/devices/microscope", io);
  auto imagingPlane = NWB::Ophys::createImagingPlane(
      *nwbfile, "plane", getTestImagingPlaneSpec(device));
  REQUIRE(imagingPlane != nullptr);

  const SizeArray frameShape = {4, 3};
  const SizeType frameSize = 12;

  // TwoPhotonSeries with explicit timestamps
  auto twoPhoton = NWB::Ophys::createTwoPhotonSeries(
      *nwbfile,
      "TwoPhotonSeries",
      NWB::TwoPhotonSeries::Spec()
          .withData(NWB::Ophys::getDefaultImageSeriesDataConfig(
              IO::BaseDataType::F32, frameShape))
          .withUnit("n.a.")
          .withDescription("two-photon data")
          .withImagingPlane(imagingPlane)
          .withDevice(device)
          .withPmtGain(2.5f)
          .withScanLineRate(15000.0f)
          .withFieldOfView(std::vector<float> {4.0e-4f, 3.0e-4f})
          .withDimension(std::vector<int32_t> {4, 3}));
  REQUIRE(twoPhoton != nullptr);
  REQUIRE(twoPhoton->getFrameShape() == frameShape);

  // OnePhotonSeries with a regular sampling rate
  auto onePhoton = NWB::Ophys::createOnePhotonSeries(
      *nwbfile,
      "OnePhotonSeries",
      NWB::OnePhotonSeries::Spec()
          .withData(NWB::Ophys::getDefaultImageSeriesDataConfig(
              IO::BaseDataType::U8, frameShape))
          .withUnit("n.a.")
          .withImagingPlane(imagingPlane)
          .withStartingTime(0.0)
          .withStartingTimeRate(20.0f)
          .withExposureTime(0.01f)
          .withBinning(2)
          .withPower(1.5f)
          .withIntensity(0.3f));
  REQUIRE(onePhoton != nullptr);

  // Write frames in two blocks
  std::vector<float> twoPhotonFrames(3 * frameSize);
  std::vector<uint8_t> onePhotonFrames(3 * frameSize);
  for (SizeType i = 0; i < twoPhotonFrames.size(); ++i) {
    twoPhotonFrames[i] = static_cast<float>(i);
    onePhotonFrames[i] = static_cast<uint8_t>(i);
  }
  std::vector<double> timestamps = {0.0, 0.1, 0.25};
  for (int block = 0; block < 2; ++block) {
    std::transform(timestamps.begin(),
                   timestamps.end(),
                   timestamps.begin(),
                   [block](double timestamp)
                   { return timestamp + 0.5 * static_cast<double>(block); });
    REQUIRE(
        NWB::Ophys::writeImageSeriesFrames(io->getRecordingObjects(),
                                           twoPhoton->getRecordingObjectIndex(),
                                           3,
                                           twoPhotonFrames.data(),
                                           timestamps.data())
        == Status::Success);
    REQUIRE(onePhoton->writeFrames(3, onePhotonFrames.data())
            == Status::Success);
  }
  REQUIRE(twoPhoton->getNumFramesRecorded() == 6);

  // Writing to an object that is not an ImageSeries fails
  REQUIRE(NWB::Ophys::writeImageSeriesFrames(
              io->getRecordingObjects(),
              imagingPlane->getRecordingObjectIndex(),
              3,
              twoPhotonFrames.data())
          == Status::Failure);
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readTwoPhoton = NWB::RegisteredType::create<NWB::TwoPhotonSeries>(
      "/acquisition/TwoPhotonSeries", readIO);
  REQUIRE(readTwoPhoton != nullptr);
  auto data = readTwoPhoton->readData<float>()->values();
  REQUIRE(data.shape == SizeArray {6, 4, 3});
  // The second block starts with the first frame again
  REQUIRE(std::abs(data.data[frameSize * 3]) < 1e-6f);
  REQUIRE(std::abs(data.data[frameSize * 6 - 1]
                   - static_cast<float>(3 * frameSize - 1))
          < 1e-6f);
  REQUIRE(readTwoPhoton->readTimestamps<double>()->values().data.size() == 6);
  REQUIRE(readTwoPhoton->readDataUnit()->values().data[0] == "n.a.");
  REQUIRE(readTwoPhoton->readFormat()->values().data[0] == "raw");
  REQUIRE(readTwoPhoton->readDimension()->values().data
          == std::vector<int32_t> {4, 3});
  REQUIRE(std::abs(readTwoPhoton->readPmtGain()->values().data[0] - 2.5f)
          < 1e-6f);
  REQUIRE(readTwoPhoton->readFieldOfView()->values().data.size() == 2);
  REQUIRE(readTwoPhoton->readImagingPlane()->readIndicator()->values().data[0]
          == "GCaMP6s");
  REQUIRE(readTwoPhoton->readDevice()->readDescription()->values().data[0]
          == "microscope");

  auto readOnePhoton = NWB::RegisteredType::create<NWB::OnePhotonSeries>(
      "/acquisition/OnePhotonSeries", readIO);
  REQUIRE(readOnePhoton != nullptr);
  REQUIRE(readOnePhoton->readData<uint8_t>()->values().shape
          == SizeArray {6, 4, 3});
  REQUIRE(readOnePhoton->readStartingTime()->exists());
  REQUIRE_FALSE(readOnePhoton->readTimestamps()->exists());
  REQUIRE(readOnePhoton->readBinning()->values().data[0] == 2);
  REQUIRE(std::abs(readOnePhoton->readExposureTime()->values().data[0] - 0.01f)
          < 1e-6f);
  REQUIRE_FALSE(readOnePhoton->readPmtGain()->exists());

  // Read the series via the generic create method
  auto generic =
      NWB::RegisteredType::create("/acquisition/TwoPhotonSeries", readIO);
  REQUIRE(generic->getFullTypeName() == "core::TwoPhotonSeries");
  readIO->close();
}

TEST_CASE("ImageSeries with external files", "[ophys]")
{
  std::string path = getTestFilePath("testOphysExternalImageSeries.h5");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  auto nwbfile = createOphysTestFile(io);

  auto series = nwbfile->createAquisitionSeries<NWB::ImageSeries>("movie");
  REQUIRE(
      series->initialize(NWB::ImageSeries::Spec()
                             .withData(IO::ArrayDataSetConfig(
                                 IO::BaseDataType::U8, {0, 0, 0}, {1, 1, 1}))
                             .withUnit("n.a.")
                             .withDescription("behavior video")
                             .withFormat("external")
                             .withExternalFile(std::vector<std::string> {
                                 "video1.avi", "video2.avi"})
                             .withExternalFileStartingFrame({0, 100})
                             .withNumSamples(250)
                             .withStartingTime(0.0)
                             .withStartingTimeRate(30.0f))
      == Status::Success);
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readSeries = NWB::RegisteredType::create<NWB::ImageSeries>(
      "/acquisition/movie", readIO);
  REQUIRE(readSeries->readFormat()->values().data[0] == "external");
  REQUIRE(readSeries->readExternalFile()->values().data
          == std::vector<std::string> {"video1.avi", "video2.avi"});
  REQUIRE(readSeries->readExternalFileStartingFrame()->values().data
          == std::vector<int32_t> {0, 100});
  REQUIRE(readSeries->readNumSamples()->values().data[0] == 250);
  REQUIRE(readSeries->readDevice() == nullptr);
  readIO->close();
}

TEST_CASE("getDefaultImageSeriesDataConfig", "[ophys]")
{
  auto config2D = NWB::Ophys::getDefaultImageSeriesDataConfig(
      IO::BaseDataType::U16, {512, 256});
  REQUIRE(config2D.getType() == IO::BaseDataType::U16);
  REQUIRE(config2D.getShape() == SizeArray {0, 512, 256});
  REQUIRE(config2D.getChunking() == SizeArray {1, 512, 256});

  auto config3D = NWB::Ophys::getDefaultImageSeriesDataConfig(
      IO::BaseDataType::F32, {64, 64, 8});
  REQUIRE(config3D.getShape() == SizeArray {0, 64, 64, 8});
  REQUIRE(config3D.getChunking() == SizeArray {1, 64, 64, 8});
}
