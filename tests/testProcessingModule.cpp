#include <catch2/catch_test_macros.hpp>

#include "Utils.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/base/ProcessingModule.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("ProcessingModule registered", "[processingmodule]")
{
  auto registry = AQNWB::NWB::RegisteredType::getRegistry();
  REQUIRE(registry.find("core::ProcessingModule") != registry.end());
}

TEST_CASE("createProcessingModule", "[processingmodule]")
{
  std::string filename = getTestFilePath("createProcessingModule.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // create and initialize a ProcessingModule
  auto processingModule = nwbfile->createProcessingModule("test_module");
  REQUIRE(processingModule != nullptr);
  Status initStatus = processingModule->initialize("A test processing module");
  REQUIRE(initStatus == Status::Success);

  // verify the module path is correct
  REQUIRE(processingModule->getPath() == "/processing/test_module");

  // read back the description
  auto descriptionData = processingModule->readDescription();
  REQUIRE(descriptionData->exists());
  std::string descriptionValue = descriptionData->values().data[0];
  REQUIRE(descriptionValue == "A test processing module");

  // read back the module via NWBFile
  auto readModule = nwbfile->readProcessingModule("test_module");
  REQUIRE(readModule != nullptr);

  nwbfile->finalize();
  io->close();
}

TEST_CASE("createMultipleProcessingModules", "[processingmodule]")
{
  std::string filename = getTestFilePath("createMultipleProcessingModules.nwb");

  // initialize nwbfile object and create base structure
  std::shared_ptr<IO::HDF5::HDF5IO> io =
      std::make_shared<IO::HDF5::HDF5IO>(filename);
  io->open();
  auto nwbfile = NWB::NWBFile::create(io);
  nwbfile->initialize(generateUuid());

  // create and initialize two ProcessingModules
  auto module1 = nwbfile->createProcessingModule("module1");
  REQUIRE(module1 != nullptr);
  REQUIRE(module1->initialize("First processing module") == Status::Success);

  auto module2 = nwbfile->createProcessingModule("module2");
  REQUIRE(module2 != nullptr);
  REQUIRE(module2->initialize("Second processing module") == Status::Success);

  // verify paths
  REQUIRE(module1->getPath() == "/processing/module1");
  REQUIRE(module2->getPath() == "/processing/module2");

  // add a TimeSeries to module1 via createNWBDataInterface
  auto ts = module1->createNWBDataInterface<NWB::TimeSeries>("test_timeseries");
  REQUIRE(ts != nullptr);

  nwbfile->finalize();
  io->close();
}
