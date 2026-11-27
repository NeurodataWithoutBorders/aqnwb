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
#include "nwb/base/Images.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/image/GrayscaleImage.hpp"
#include "nwb/image/ImageMaskSeries.hpp"
#include "nwb/image/IndexSeries.hpp"
#include "nwb/image/OpticalSeries.hpp"
#include "nwb/image/RGBAImage.hpp"
#include "nwb/image/RGBImage.hpp"
#include "nwb/ophys/OphysUtils.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("Images, IndexSeries, OpticalSeries, and ImageMaskSeries", "[ophys]")
{
  auto registry = NWB::RegisteredType::getRegistry();
  for (const std::string& type : {"core::Images",
                                  "core::Image",
                                  "core::ImageReferences",
                                  "core::GrayscaleImage",
                                  "core::RGBImage",
                                  "core::RGBAImage",
                                  "core::IndexSeries",
                                  "core::OpticalSeries",
                                  "core::ImageMaskSeries",
                                  "core::MotionCorrection",
                                  "core::CorrectedImageStack"})
  {
    INFO(type);
    REQUIRE(registry.count(type) == 1);
  }

  std::string path = getTestFilePath("testOphysStimulusImages.nwb");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  std::string currentTime = getCurrentTime();
  REQUIRE(nwbfile->initialize(generateUuid(),
                              "stimulus images session",
                              "",
                              currentTime,
                              currentTime,
                              getTestSubjectSpec())
          == Status::Success);

  // [example_images_snippet]
  // A collection of stimulus images in /stimulus/templates. The images are
  // part of the spec, since a collection requires at least one image.
  std::vector<uint8_t> grayPixels(4 * 3, 100);
  std::vector<uint8_t> rgbPixels(4 * 3 * 3, 200);
  auto images = NWB::Images::create("/stimulus/templates/scenes", io);
  Status status = images->initialize(
      NWB::Images::Spec()
          .withDescription("Natural scenes")
          .withImage<NWB::GrayscaleImage>(
              "gray",
              NWB::Image::Spec()
                  .withData(IO::ArrayDataSetConfig(
                      IO::BaseDataType::U8, {4, 3}, {4, 3}))
                  .withDescription("A gray image")
                  .withResolution(70.0f),
              grayPixels.data())
          .withImage<NWB::RGBImage>(
              "rgb",
              NWB::Image::Spec()
                  .withData(IO::ArrayDataSetConfig(
                      IO::BaseDataType::U8, {4, 3, 3}, {4, 3, 3}))
                  .withDescription("An RGB image"),
              rgbPixels.data())
          .withOrderOfImages(std::vector<std::string> {"gray", "rgb"}));

  // The order in which the images were presented
  auto indexSeries =
      nwbfile->createAquisitionSeries<NWB::IndexSeries>("presented_scenes");
  status = status
      && indexSeries->initialize(
          NWB::IndexSeries::Spec()
              .withData(
                  IO::ArrayDataSetConfig(IO::BaseDataType::U32, {0}, {100}))
              .withDescription("Indices of the presented scenes")
              .withIndexedImages(images));
  std::vector<uint32_t> indices = {0, 1, 1, 0};
  std::vector<double> timestamps = {0.0, 0.5, 1.2, 1.5};
  status =
      status && indexSeries->writeIndices(4, indices.data(), timestamps.data());
  // [example_images_snippet]
  REQUIRE(status == Status::Success);

  // Invalid images and specs
  REQUIRE(images->addImage<NWB::RGBImage>(
              "invalid_rgb",
              NWB::Image::Spec().withData(
                  IO::ArrayDataSetConfig(IO::BaseDataType::U8, {4, 3}, {4, 3})),
              grayPixels.data())
          == nullptr);  // RGB images are 3D
  REQUIRE_FALSE(io->objectExists("/stimulus/templates/scenes/invalid_rgb"));
  REQUIRE(images->addImage<NWB::RGBAImage>(
              "invalid_rgba",
              NWB::Image::Spec().withData(IO::ArrayDataSetConfig(
                  IO::BaseDataType::U8, {4, 3, 3}, {4, 3, 3})),
              rgbPixels.data())
          == nullptr);  // RGBA images have 4 channels
  // Images can be added to an initialized collection without an order of the
  // images, since the order must list all images
  REQUIRE(images->addImage<NWB::GrayscaleImage>(
              "gray2",
              NWB::Image::Spec().withData(
                  IO::ArrayDataSetConfig(IO::BaseDataType::U8, {4, 3}, {4, 3})),
              grayPixels.data())
          == nullptr);
  auto unordered = NWB::Images::create("/stimulus/templates/extra", io);
  REQUIRE(unordered->initialize(
              NWB::Images::Spec()
                  .withDescription("Extra images")
                  .withImage<NWB::GrayscaleImage>(
                      "gray",
                      NWB::Image::Spec().withData(IO::ArrayDataSetConfig(
                          IO::BaseDataType::U8, {4, 3}, {4, 3})),
                      grayPixels.data()))
          == Status::Success);
  REQUIRE(unordered->addImage<NWB::GrayscaleImage>(
              "gray2",
              NWB::Image::Spec().withData(
                  IO::ArrayDataSetConfig(IO::BaseDataType::U8, {4, 3}, {4, 3})),
              grayPixels.data())
          != nullptr);
  REQUIRE(NWB::Image::Spec().validate() == Status::Failure);  // no data
  REQUIRE(NWB::Image::Spec()
              .withData(IO::ArrayDataSetConfig(
                  IO::BaseDataType::V_STR, {4, 3}, {4, 3}))
              .validate()
          == Status::Failure);  // not numeric
  REQUIRE(NWB::Images::Spec().withDescription("no images").validate()
          == Status::Failure);  // at least one image
  REQUIRE(NWB::Images::Spec()
              .withDescription("duplicate")
              .withImage<NWB::GrayscaleImage>(
                  "a",
                  NWB::Image::Spec().withData(IO::ArrayDataSetConfig(
                      IO::BaseDataType::U8, {4, 3}, {4, 3})),
                  grayPixels.data())
              .withImage<NWB::GrayscaleImage>(
                  "a",
                  NWB::Image::Spec().withData(IO::ArrayDataSetConfig(
                      IO::BaseDataType::U8, {4, 3}, {4, 3})),
                  grayPixels.data())
              .validate()
          == Status::Failure);  // duplicate names
  // An image with an invalid shape for its type fails before the collection
  // is created
  auto invalidImages = NWB::Images::create("/stimulus/templates/invalid", io);
  REQUIRE(invalidImages->initialize(
              NWB::Images::Spec()
                  .withDescription("invalid")
                  .withImage<NWB::RGBImage>(
                      "rgb",
                      NWB::Image::Spec().withData(IO::ArrayDataSetConfig(
                          IO::BaseDataType::U8, {4, 3}, {4, 3})),
                      grayPixels.data()))
          == Status::Failure);
  REQUIRE_FALSE(io->objectExists("/stimulus/templates/invalid"));
  REQUIRE(images->setOrderOfImages({"missing"}) == Status::Failure);
  REQUIRE(NWB::Images::Spec().validate() == Status::Failure);
  REQUIRE(
      NWB::IndexSeries::Spec()
          .withData(IO::ArrayDataSetConfig(IO::BaseDataType::U32, {0}, {10}))
          .validate()
      == Status::Failure);  // missing indexed images
  REQUIRE(
      NWB::IndexSeries::Spec()
          .withData(IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}))
          .withIndexedImages(images)
          .validate()
      == Status::Failure);  // data must be uint32
  REQUIRE(
      NWB::IndexSeries::Spec()
          .withData(IO::ArrayDataSetConfig(IO::BaseDataType::U32, {0}, {10}))
          .withIndexedImages(images)
          .withUnit("seconds")
          .validate()
      == Status::Failure);  // the unit is fixed to N/A

  // OpticalSeries with RGB frames and an ImageMaskSeries for the frames
  auto opticalSeries =
      nwbfile->createAquisitionSeries<NWB::OpticalSeries>("movie");
  REQUIRE(opticalSeries->initialize(
              NWB::OpticalSeries::Spec()
                  .withData(IO::ArrayDataSetConfig(
                      IO::BaseDataType::U8, {0, 2, 2, 3}, {1, 2, 2, 3}))
                  .withUnit("n.a.")
                  .withDescription("A movie presented to the subject")
                  .withStartingTime(0.0)
                  .withStartingTimeRate(10.0f)
                  .withDistance(0.7f)
                  .withFieldOfView(std::vector<float> {0.3f, 0.2f})
                  .withOrientation("lower left corner is the origin"))
          == Status::Success);
  std::vector<uint8_t> movieFrames(5 * 2 * 2 * 3, 50);
  REQUIRE(opticalSeries->writeFrames(5, movieFrames.data()) == Status::Success);
  REQUIRE(NWB::OpticalSeries::Spec()
              .withData(IO::ArrayDataSetConfig(
                  IO::BaseDataType::U8, {0, 4, 3, 2}, {1, 4, 3, 2}))
              .withUnit("n.a.")
              .validate()
          == Status::Failure);  // 4D data must have 3 channels

  auto maskSeries =
      nwbfile->createAquisitionSeries<NWB::ImageMaskSeries>("movie_mask");
  REQUIRE(maskSeries->initialize(
              NWB::ImageMaskSeries::Spec()
                  .withData(IO::ArrayDataSetConfig(
                      IO::BaseDataType::U8, {0, 2, 2}, {1, 2, 2}))
                  .withUnit("n.a.")
                  .withDescription("Mask of the movie")
                  .withStartingTime(0.0)
                  .withStartingTimeRate(10.0f)
                  .withMaskedImageSeries(opticalSeries))
          == Status::Success);
  std::vector<uint8_t> mask(5 * 2 * 2, 1);
  REQUIRE(maskSeries->writeFrames(5, mask.data()) == Status::Success);
  REQUIRE(NWB::ImageMaskSeries::Spec()
              .withData(IO::ArrayDataSetConfig(
                  IO::BaseDataType::U8, {0, 4, 3}, {1, 4, 3}))
              .withUnit("n.a.")
              .validate()
          == Status::Failure);  // missing masked ImageSeries
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readImages = NWB::RegisteredType::create<NWB::Images>(
      "/stimulus/templates/scenes", readIO);
  REQUIRE(readImages->readDescription()->values().data[0] == "Natural scenes");
  auto readGray = std::dynamic_pointer_cast<NWB::GrayscaleImage>(
      NWB::RegisteredType::create("/stimulus/templates/scenes/gray", readIO));
  REQUIRE(readGray != nullptr);
  REQUIRE(readGray->readData<uint8_t>()->values().data == grayPixels);
  REQUIRE(std::abs(readGray->readResolution()->values().data[0] - 70.0f)
          < 1e-6f);
  REQUIRE(readImages->readOrderOfImages() != nullptr);
  auto readIndex = NWB::RegisteredType::create<NWB::IndexSeries>(
      "/acquisition/presented_scenes", readIO);
  REQUIRE(readIndex->readData<uint32_t>()->values().data == indices);
  REQUIRE(readIndex->readDataUnit()->values().data[0] == "N/A");
  REQUIRE(readIndex->readIndexedImages()->readDescription()->values().data[0]
          == "Natural scenes");
  auto readOptical = NWB::RegisteredType::create<NWB::OpticalSeries>(
      "/acquisition/movie", readIO);
  REQUIRE(std::abs(readOptical->readDistance()->values().data[0] - 0.7f)
          < 1e-6f);
  REQUIRE(readOptical->readOrientation()->values().data[0]
          == "lower left corner is the origin");
  auto readMask = NWB::RegisteredType::create<NWB::ImageMaskSeries>(
      "/acquisition/movie_mask", readIO);
  REQUIRE(readMask->readMaskedImageSeries() != nullptr);
  readIO->close();
}

TEST_CASE("MotionCorrection", "[ophys]")
{
  std::string path = getTestFilePath("testOphysMotionCorrection.nwb");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  std::string currentTime = getCurrentTime();
  REQUIRE(nwbfile->initialize(generateUuid(),
                              "motion correction session",
                              "",
                              currentTime,
                              currentTime,
                              getTestSubjectSpec())
          == Status::Success);
  auto microscope = NWB::Device::create("/general/devices/microscope", io);
  REQUIRE(
      microscope->initialize(NWB::Device::Spec().withDescription("microscope"))
      == Status::Success);
  auto imagingPlane = NWB::Ophys::createImagingPlane(
      *nwbfile,
      "imaging_plane",
      NWB::ImagingPlane::Spec()
          .withExcitationLambda(920.0f)
          .withIndicator("GCaMP6s")
          .withLocation("VISp")
          .withDevice(microscope)
          .withOpticalChannel("green",
                              NWB::OpticalChannel::Spec()
                                  .withDescription("green channel")
                                  .withEmissionLambda(510.0f)));
  const SizeArray frameShape = {2, 2};
  auto raw = NWB::Ophys::createTwoPhotonSeries(
      *nwbfile,
      "TwoPhotonSeries",
      NWB::TwoPhotonSeries::Spec()
          .withData(NWB::Ophys::getDefaultImageSeriesDataConfig(
              IO::BaseDataType::F32, frameShape))
          .withUnit("n.a.")
          .withStartingTime(0.0)
          .withStartingTimeRate(30.0f)
          .withImagingPlane(imagingPlane));
  REQUIRE(raw != nullptr);
  std::vector<float> rawFrames(5 * 2 * 2, 2.0f);
  REQUIRE(raw->writeFrames(5, rawFrames.data()) == Status::Success);

  // [example_motion_correction_snippet]
  // Store the motion-corrected frames and the x/y translation of each frame
  // in /processing/ophys/MotionCorrection
  auto stack = NWB::Ophys::createCorrectedImageStack(
      *nwbfile,
      "CorrectedImageStack",
      NWB::CorrectedImageStack::Spec()
          .withOriginal(raw)
          .withCorrected(
              NWB::ImageSeries::Spec()
                  .withData(NWB::Ophys::getDefaultImageSeriesDataConfig(
                      IO::BaseDataType::F32, frameShape))
                  .withUnit("n.a.")
                  .withStartingTime(0.0)
                  .withStartingTimeRate(30.0f))
          .withXyTranslation(NWB::TimeSeries::Spec()
                                 .withData(IO::ArrayDataSetConfig(
                                     IO::BaseDataType::F32, {0, 2}, {100, 2}))
                                 .withUnit("pixels")
                                 .withStartingTime(0.0)
                                 .withStartingTimeRate(30.0f)));
  std::vector<float> frames(5 * 2 * 2, 1.0f);
  std::vector<float> translation = {
      0.5f, -0.5f, 1.0f, 0.0f, 0.25f, 0.5f, 0.0f, 0.0f, 1.5f, -1.0f};
  Status status = stack->readCorrected()->writeFrames(5, frames.data());
  status = status
      && stack->readXyTranslation()->writeData(
          {5, 2}, {0, 0}, translation.data());
  // [example_motion_correction_snippet]
  REQUIRE(status == Status::Success);

  REQUIRE(NWB::CorrectedImageStack::Spec()
              .withCorrected(NWB::ImageSeries::Spec())
              .validate()
          == Status::Failure);  // missing original and nested specs
  REQUIRE(
      NWB::CorrectedImageStack::Spec()
          .withOriginal(raw)
          .withCorrected(
              NWB::ImageSeries::Spec()
                  .withData(NWB::Ophys::getDefaultImageSeriesDataConfig(
                      IO::BaseDataType::F32, frameShape))
                  .withUnit("n.a."))
          .withXyTranslation(NWB::TimeSeries::Spec()
                                 .withData(IO::ArrayDataSetConfig(
                                     IO::BaseDataType::F32, {0, 3}, {10, 3}))
                                 .withUnit("pixels"))
          .validate()
      == Status::Failure);  // xy_translation must have 2 columns

  // The data of the child series is checked before the stack is created
  auto invalidStack = NWB::CorrectedImageStack::create(
      "/processing/ophys/MotionCorrection/InvalidStack", io);
  REQUIRE(invalidStack->initialize(
              NWB::CorrectedImageStack::Spec()
                  .withOriginal(raw)
                  .withCorrected(
                      NWB::ImageSeries::Spec()
                          .withData(IO::LinkArrayDataSetConfig("/missing/data"))
                          .withUnit("n.a."))
                  .withXyTranslation(
                      NWB::TimeSeries::Spec()
                          .withData(IO::ArrayDataSetConfig(
                              IO::BaseDataType::F32, {0, 2}, {100, 2}))
                          .withUnit("pixels")))
          == Status::Failure);
  REQUIRE_FALSE(
      io->objectExists("/processing/ophys/MotionCorrection/InvalidStack"));
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readStack = NWB::RegisteredType::create<NWB::CorrectedImageStack>(
      "/processing/ophys/MotionCorrection/CorrectedImageStack", readIO);
  REQUIRE(readStack != nullptr);
  REQUIRE(readStack->readCorrected()->readData<float>()->values().shape
          == SizeArray {5, 2, 2});
  REQUIRE(readStack->readXyTranslation()->readData<float>()->values().data
          == translation);
  REQUIRE(readStack->readOriginal()->readDataUnit()->values().data[0]
          == "n.a.");
  readIO->close();
}

TEST_CASE("IndexSeries checks", "[ophys]")
{
  std::string path = getTestFilePath("testOphysIndexChecks.h5");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  std::string currentTime = getCurrentTime();
  REQUIRE(nwbfile->initialize(generateUuid(),
                              "IndexSeries checks",
                              "",
                              currentTime,
                              currentTime,
                              getTestSubjectSpec())
          == Status::Success);
  auto microscope = NWB::Device::create("/general/devices/microscope", io);
  REQUIRE(
      microscope->initialize(NWB::Device::Spec().withDescription("microscope"))
      == Status::Success);
  auto planeSpec = NWB::ImagingPlane::Spec()
                       .withExcitationLambda(920.0f)
                       .withIndicator("GCaMP6s")
                       .withLocation("VISp")
                       .withDevice(microscope)
                       .withOpticalChannel("green",
                                           NWB::OpticalChannel::Spec()
                                               .withDescription("green channel")
                                               .withEmissionLambda(510.0f));
  auto imagingPlane =
      NWB::Ophys::createImagingPlane(*nwbfile, "imaging_plane", planeSpec);
  REQUIRE(imagingPlane != nullptr);

  SECTION("IndexSeries requires the order of the images")
  {
    std::vector<uint8_t> pixels(4, 0);
    auto images = NWB::Images::create("/stimulus/templates/unordered", io);
    REQUIRE(images->initialize(
                NWB::Images::Spec()
                    .withDescription("unordered")
                    .withImage<NWB::GrayscaleImage>(
                        "gray",
                        NWB::Image::Spec().withData(IO::ArrayDataSetConfig(
                            IO::BaseDataType::U8, {2, 2}, {2, 2})),
                        pixels.data()))
            == Status::Success);
    auto indexSeries =
        nwbfile->createAquisitionSeries<NWB::IndexSeries>("indices");
    REQUIRE(indexSeries->initialize(NWB::IndexSeries::Spec()
                                        .withData(IO::ArrayDataSetConfig(
                                            IO::BaseDataType::U32, {0}, {10}))
                                        .withIndexedImages(images))
            == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/acquisition/indices"));
  }
  io->close();
}
