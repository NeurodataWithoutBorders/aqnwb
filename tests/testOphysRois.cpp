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
#include "nwb/ophys/OphysUtils.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("ophys ROI types", "[ophys]")
{
  auto registry = NWB::RegisteredType::getRegistry();
  for (const std::string& type : {"core::PlaneSegmentation",
                                  "core::ImageSegmentation",
                                  "core::RoiResponseSeries",
                                  "core::Fluorescence",
                                  "core::DfOverF"})
  {
    INFO(type);
    REQUIRE(registry.count(type) == 1);
  }

  std::string path = getTestFilePath("exampleOphysRois.nwb");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  std::string currentTime = getCurrentTime();
  REQUIRE(nwbfile->initialize(generateUuid(),
                              "ophys ROI session",
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
  REQUIRE(imagingPlane != nullptr);

  // [example_ophys_rois_snippet]
  // Create the PlaneSegmentation with an image_mask column for masks of
  // 8 x 6 pixels (in /processing/ophys/ImageSegmentation)
  const SizeArray maskShape = {8, 6};
  auto columns = NWB::PlaneSegmentation::createDefaultDataSpecs();
  columns.push_back(NWB::PlaneSegmentation::createImageMaskDataSpec(maskShape));
  auto planeSegmentation = NWB::Ophys::createPlaneSegmentation(
      *nwbfile,
      "PlaneSegmentation",
      NWB::PlaneSegmentation::Spec()
          .withDescription("ROIs detected during the acquisition")
          .withColumns(columns)
          .withImagingPlane(imagingPlane));

  // Add three ROIs, each defined by a mask with the weight of each pixel
  const SizeType numRois = 3;
  std::vector<float> masks(numRois * 8 * 6, 0.0f);
  for (SizeType roi = 0; roi < numRois; ++roi) {
    masks[roi * 48 + roi * 7] = 1.0f;  // one pixel per ROI
  }
  Status status = planeSegmentation->addImageMasks(numRois, masks.data());
  REQUIRE(status == Status::Success);

  // Create a RoiResponseSeries with the fluorescence of the three ROIs (in
  // /processing/ophys/Fluorescence) acquired at 30 Hz
  auto fluorescence = NWB::Ophys::createRoiResponseSeries(
      *nwbfile,
      NWB::Ophys::RoiResponseContainer::Fluorescence,
      "RoiResponseSeries",
      NWB::RoiResponseSeries::Spec()
          .withData(IO::ArrayDataSetConfig(
              IO::BaseDataType::F32, {0, numRois}, {300, numRois}))
          .withUnit("n.a.")
          .withDescription("Fluorescence of the ROIs")
          .withStartingTime(0.0)
          .withStartingTimeRate(30.0f)
          .withPlaneSegmentation(planeSegmentation)
          .withRoiIndices({0, 1, 2}));

  // Record the fluorescence during the acquisition (after
  // io->startRecording(), see the TwoPhotonSeries example)
  std::vector<float> samples(100 * numRois);
  for (SizeType i = 0; i < samples.size(); ++i) {
    samples[i] = static_cast<float>(i % 17);
  }
  status = NWB::Ophys::writeRoiResponseSeriesData(
      io->getRecordingObjects(),
      fluorescence->getRecordingObjectIndex(),
      100,
      samples.data());
  // [example_ophys_rois_snippet]
  REQUIRE(planeSegmentation != nullptr);
  REQUIRE(fluorescence != nullptr);
  REQUIRE(status == Status::Success);
  REQUIRE(fluorescence->writeSamples(100, samples.data()) == Status::Success);

  // A dF/F series for the first ROI only (1D data)
  auto dfOverF = NWB::Ophys::createRoiResponseSeries(
      *nwbfile,
      NWB::Ophys::RoiResponseContainer::DfOverF,
      "RoiResponseSeries",
      NWB::RoiResponseSeries::Spec()
          .withData(IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {300}))
          .withUnit("n.a.")
          .withDescription("dF/F of the first ROI")
          .withStartingTime(0.0)
          .withStartingTimeRate(30.0f)
          .withPlaneSegmentation(planeSegmentation)
          .withRoiIndices({0}));
  REQUIRE(dfOverF != nullptr);
  REQUIRE(dfOverF->writeSamples(200, samples.data()) == Status::Success);

  // Invalid specs
  REQUIRE(NWB::PlaneSegmentation::Spec().withDescription("d").validate()
          == Status::Failure);  // missing imaging plane
  REQUIRE(NWB::RoiResponseSeries::Spec()
              .withData(IO::ArrayDataSetConfig(
                  IO::BaseDataType::F32, {0, 2}, {10, 2}))
              .withUnit("n.a.")
              .withPlaneSegmentation(planeSegmentation)
              .withRoiIndices({0, 1, 2})
              .validate()
          == Status::Failure);  // 2 columns but 3 ROIs
  REQUIRE(
      NWB::RoiResponseSeries::Spec()
          .withData(IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}))
          .withUnit("n.a.")
          .withRoiIndices({0})
          .validate()
      == Status::Failure);  // missing plane segmentation
  auto invalidColumns = NWB::PlaneSegmentation::createDefaultDataSpecs();
  invalidColumns.push_back(NWB::VectorData::createDataSpec(
      "image_mask",
      IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}),
      "1D masks are invalid"));
  auto invalid = NWB::PlaneSegmentation::create(
      "/processing/ophys/ImageSegmentation/invalid", io);
  REQUIRE(invalid->validateDataSpecs(invalidColumns) == Status::Failure);
  REQUIRE(invalid->addImageMasks(1, masks.data()) == Status::Failure);

  io->close();

  // Read the data back
  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readPlane = NWB::RegisteredType::create<NWB::PlaneSegmentation>(
      "/processing/ophys/ImageSegmentation/PlaneSegmentation", readIO);
  REQUIRE(readPlane != nullptr);
  REQUIRE(readPlane->getNumberOfRows() == numRois);
  auto readMasks = readPlane->readImageMask()->readData<float>()->values();
  REQUIRE(readMasks.shape == SizeArray {3, 8, 6});
  REQUIRE(std::abs(readMasks.data[48 + 7] - 1.0f) < 1e-6f);
  REQUIRE(readPlane->readImagingPlane()->readIndicator()->values().data[0]
          == "GCaMP6s");

  auto readFluorescence = NWB::RegisteredType::create<NWB::RoiResponseSeries>(
      "/processing/ophys/Fluorescence/RoiResponseSeries", readIO);
  REQUIRE(readFluorescence->readData<float>()->values().shape
          == SizeArray {200, 3});
  auto rois = readFluorescence->readRois();
  REQUIRE(rois->readData<int32_t>()->values().data
          == std::vector<int32_t> {0, 1, 2});
  REQUIRE(rois->readTable()->readDescription()->values().data[0]
          == "ROIs detected during the acquisition");
  auto readDfOverF = NWB::RegisteredType::create<NWB::RoiResponseSeries>(
      "/processing/ophys/DfOverF/RoiResponseSeries", readIO);
  REQUIRE(readDfOverF->readData<float>()->values().shape == SizeArray {200});
  REQUIRE(NWB::RegisteredType::create("/processing/ophys/Fluorescence", readIO)
              ->getFullTypeName()
          == "core::Fluorescence");
  readIO->close();
}

TEST_CASE("PlaneSegmentation with pixel and voxel masks", "[ophys]")
{
  std::string path = getTestFilePath("testOphysPixelMasks.nwb");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  std::string currentTime = getCurrentTime();
  REQUIRE(nwbfile->initialize(generateUuid(),
                              "pixel mask session",
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

  // [example_ophys_pixel_masks_snippet]
  // A PlaneSegmentation with the ROIs defined by pixel masks
  auto pixelSegmentation = NWB::Ophys::createPlaneSegmentation(
      *nwbfile,
      "PixelSegmentation",
      NWB::PlaneSegmentation::Spec()
          .withDescription("ROIs defined by pixel masks")
          .withImagingPlane(imagingPlane)
          .withPixelMask(true));
  // Each ROI is a list of (x, y, weight) entries
  Status status = pixelSegmentation->addPixelMasks(
      {{{0, 0, 1.0f}, {0, 1, 0.5f}},
       {{5, 5, 1.0f}, {5, 6, 1.0f}, {6, 5, 1.0f}}});
  // [example_ophys_pixel_masks_snippet]
  REQUIRE(status == Status::Success);
  REQUIRE(pixelSegmentation->addPixelMasks({{{9, 9, 0.25f}}})
          == Status::Success);
  REQUIRE(pixelSegmentation->addImageMasks(1, nullptr)
          == Status::Failure);  // no image_mask column

  auto voxelSegmentation = NWB::Ophys::createPlaneSegmentation(
      *nwbfile,
      "VoxelSegmentation",
      NWB::PlaneSegmentation::Spec()
          .withDescription("ROIs defined by voxel masks")
          .withImagingPlane(imagingPlane)
          .withVoxelMask(true));
  REQUIRE(voxelSegmentation->addVoxelMasks(
              {{{1, 2, 3, 1.0f}}, {{4, 5, 6, 0.5f}, {7, 8, 9, 0.5f}}})
          == Status::Success);
  REQUIRE(voxelSegmentation->addPixelMasks({{{0, 0, 1.0f}}})
          == Status::Failure);  // no pixel_mask column
  REQUIRE(nwbfile->finalize() == Status::Success);
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readPixels = NWB::RegisteredType::create<NWB::PlaneSegmentation>(
      "/processing/ophys/ImageSegmentation/PixelSegmentation", readIO);
  REQUIRE(readPixels->getNumberOfRows() == 3);
  auto index = readPixels->readPixelMaskIndex()->readData<uint32_t>()->values();
  REQUIRE(index.data == std::vector<uint32_t> {2, 5, 6});
  auto block = readIO->readDataset(
      "/processing/ophys/ImageSegmentation/PixelSegmentation/pixel_mask");
  REQUIRE(block.baseDataType == NWB::PlaneSegmentation::getPixelMaskType());
  auto bytes = std::any_cast<std::vector<uint8_t>>(block.data);
  REQUIRE(bytes.size() == 6 * sizeof(NWB::PlaneSegmentation::PixelMaskEntry));
  const auto* entries =
      reinterpret_cast<const NWB::PlaneSegmentation::PixelMaskEntry*>(
          bytes.data());
  REQUIRE(entries[1].y == 1);
  REQUIRE(entries[4].x == 6);
  REQUIRE(std::abs(entries[5].weight - 0.25f) < 1e-6f);

  auto readVoxels = NWB::RegisteredType::create<NWB::PlaneSegmentation>(
      "/processing/ophys/ImageSegmentation/VoxelSegmentation", readIO);
  REQUIRE(readVoxels->getNumberOfRows() == 2);
  REQUIRE(readVoxels->readVoxelMaskIndex()->readData<uint32_t>()->values().data
          == std::vector<uint32_t> {1, 3});
  readIO->close();
}
