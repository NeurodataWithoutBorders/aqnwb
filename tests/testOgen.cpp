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
#include "nwb/ogen/OgenUtils.hpp"
#include "nwb/ogen/OptogeneticSeries.hpp"
#include "nwb/ogen/OptogeneticStimulusSite.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("registered ogen types", "[ogen]")
{
  auto registry = NWB::RegisteredType::getRegistry();
  REQUIRE(registry.count("core::OptogeneticSeries") == 1);
  REQUIRE(registry.count("core::OptogeneticStimulusSite") == 1);
}

TEST_CASE("ogen spec validation", "[ogen]")
{
  std::string path = getTestFilePath("testOgenSpecValidation.h5");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  auto device = NWB::Device::Spec().createAndInitialize("/device", io);
  REQUIRE(device != nullptr);

  auto siteSpec = NWB::OptogeneticStimulusSite::Spec()
                      .withDescription("site")
                      .withExcitationLambda(473.0f)
                      .withLocation("VISp")
                      .withDevice(device);
  REQUIRE(siteSpec.validate() == Status::Success);
  REQUIRE(NWB::OptogeneticStimulusSite::Spec(siteSpec)
              .withDevice(nullptr)
              .validate()
          == Status::Failure);
  REQUIRE(NWB::OptogeneticStimulusSite::Spec(siteSpec)
              .withExcitationLambda(0.0f)
              .validate()
          == Status::Failure);
  REQUIRE(
      NWB::OptogeneticStimulusSite::Spec(siteSpec).withLocation("").validate()
      == Status::Failure);

  auto site = siteSpec.createAndInitialize("/site", io);
  REQUIRE(site != nullptr);
  IO::ArrayDataSetConfig config(IO::BaseDataType::F32, {0}, {100});
  auto seriesSpec =
      NWB::OptogeneticSeries::Spec().withData(config).withSite(site);
  REQUIRE(seriesSpec.unit == "watts");
  REQUIRE(seriesSpec.validate() == Status::Success);
  REQUIRE(NWB::OptogeneticSeries::Spec(seriesSpec).withSite(nullptr).validate()
          == Status::Failure);
  REQUIRE(NWB::OptogeneticSeries::Spec(seriesSpec).withUnit("mW").validate()
          == Status::Failure);
  REQUIRE(NWB::OptogeneticSeries::Spec(seriesSpec)
              .withData(IO::ArrayDataSetConfig(
                  IO::BaseDataType::F32, {0, 2, 2}, {10, 2, 2}))
              .validate()
          == Status::Failure);  // 3D data
  io->close();
}

TEST_CASE("ogenWorkflowExamples", "[ogen]")
{
  std::string path = getTestFilePath("exampleOgenRecording.nwb");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  std::string currentTime = getCurrentTime();
  REQUIRE(nwbfile->initialize(generateUuid(),
                              "an optogenetics session",
                              "",
                              currentTime,
                              currentTime,
                              getTestSubjectSpec())
          == Status::Success);

  // [example_ogen_snippet]
  // Create the device and the stimulation site
  auto laser = NWB::Device::Spec()
                   .withDescription("473 nm laser")
                   .createAndInitialize("/general/devices/laser", io);
  auto site = NWB::Ogen::createOptogeneticStimulusSite(
      *nwbfile,
      "site",
      NWB::OptogeneticStimulusSite::Spec()
          .withDescription("Fiber above VISp")
          .withExcitationLambda(473.0f)
          .withLocation("VISp")
          .withDevice(laser));

  // Create the OptogeneticSeries with the applied power sampled at 1 kHz
  auto series = NWB::Ogen::createOptogeneticSeries(
      *nwbfile,
      "OptogeneticSeries",
      NWB::OptogeneticSeries::Spec()
          .withData(IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {1000}))
          .withDescription("Laser power")
          .withStartingTime(0.0)
          .withStartingTimeRate(1000.0f)
          .withSite(site));

  // Record the applied power (in watts) during the stimulation
  io->startRecording();
  std::vector<float> power(1000);
  for (SizeType i = 0; i < power.size(); ++i) {
    power[i] = (i % 100 < 10) ? 0.01f : 0.0f;  // 10 ms pulses at 10 Hz
  }
  for (int block = 0; block < 3; ++block) {
    Status status =
        NWB::Ogen::writeOptogeneticSeriesData(io->getRecordingObjects(),
                                              series->getRecordingObjectIndex(),
                                              power.size(),
                                              power.data());
    REQUIRE(status == Status::Success);
  }
  io->stopRecording();
  io->close();
  // [example_ogen_snippet]
  REQUIRE(site != nullptr);
  REQUIRE(series != nullptr);

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readSite = NWB::RegisteredType::create<NWB::OptogeneticStimulusSite>(
      "/general/optogenetics/site", readIO);
  REQUIRE(readSite->readDescription()->values().data[0] == "Fiber above VISp");
  REQUIRE(std::abs(readSite->readExcitationLambda()->values().data[0] - 473.0f)
          < 1e-6f);
  REQUIRE(readSite->readLocation()->values().data[0] == "VISp");
  REQUIRE(readSite->readDevice()->readDescription()->values().data[0]
          == "473 nm laser");

  auto readSeries = NWB::RegisteredType::create<NWB::OptogeneticSeries>(
      "/acquisition/OptogeneticSeries", readIO);
  REQUIRE(readSeries->readData<float>()->values().shape == SizeArray {3000});
  REQUIRE(readSeries->readDataUnit()->values().data[0] == "watts");
  REQUIRE(readSeries->readSite()->readLocation()->values().data[0] == "VISp");

  // Writing to an object that is not an OptogeneticSeries fails
  REQUIRE(NWB::Ogen::writeOptogeneticSeriesData(
              nullptr, 0, power.size(), power.data())
          == Status::Failure);
  readIO->close();
}
