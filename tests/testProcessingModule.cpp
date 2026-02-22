#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/base/ProcessingModule.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
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

TEST_CASE("ProcessingModule createNWBDataInterface and readNWBDataInterface",
          "[processingmodule]")
{
  std::string filename =
      getTestFilePath("createProcessingModuleTimeSeries.nwb");
  SizeType numSamples = 5;
  std::vector<float> data = getMockData1D(numSamples);
  std::vector<double> timestamps = getMockTimestamps(numSamples, 1);
  SizeArray dataShape = {numSamples};
  SizeArray positionOffset = {0};

  // --- Write ---
  {
    std::shared_ptr<IO::HDF5::HDF5IO> io =
        std::make_shared<IO::HDF5::HDF5IO>(filename);
    io->open();
    auto nwbfile = NWB::NWBFile::create(io);
    nwbfile->initialize(generateUuid());

    // create processing module
    auto processingModule = nwbfile->createProcessingModule("ecephys");
    REQUIRE(processingModule != nullptr);
    REQUIRE(processingModule->initialize("Processed ecephys data")
            == Status::Success);

    // create a TimeSeries inside the module via createNWBDataInterface
    auto ts = processingModule->createNWBDataInterface<NWB::TimeSeries>(
        "filtered_signal");
    REQUIRE(ts != nullptr);
    REQUIRE(ts->getPath() == "/processing/ecephys/filtered_signal");

    // initialize the TimeSeries
    IO::ArrayDataSetConfig config(
        BaseDataType::F32, SizeArray {0}, SizeArray {numSamples});
    Status tsStatus = ts->initialize(config, "volts", "Filtered LFP signal");
    REQUIRE(tsStatus == Status::Success);

    // write data
    Status writeStatus = ts->writeData(
        dataShape, positionOffset, data.data(), timestamps.data());
    REQUIRE(writeStatus == Status::Success);

    // read back the TimeSeries within the same session via readNWBDataInterface
    auto readTs = processingModule->readNWBDataInterface<NWB::TimeSeries>(
        "filtered_signal");
    REQUIRE(readTs != nullptr);
    REQUIRE(readTs->getPath() == "/processing/ecephys/filtered_signal");

    io->flush();
    nwbfile->finalize();
    io->close();
  }

  // --- Read back from file ---
  {
    std::shared_ptr<IO::HDF5::HDF5IO> readio =
        std::make_shared<IO::HDF5::HDF5IO>(filename);
    readio->open(IO::FileMode::ReadOnly);

    // read the processing module
    auto nwbfile = NWB::NWBFile::create(readio);
    auto processingModule = nwbfile->readProcessingModule("ecephys");
    REQUIRE(processingModule != nullptr);

    // verify description
    auto descData = processingModule->readDescription();
    REQUIRE(descData->exists());
    REQUIRE(descData->values().data[0] == "Processed ecephys data");

    // read the TimeSeries from the module
    auto readTs = processingModule->readNWBDataInterface<NWB::TimeSeries>(
        "filtered_signal");
    REQUIRE(readTs != nullptr);

    // verify data
    auto readDataWrapper = readTs->readData<float>();
    REQUIRE(readDataWrapper->exists());
    auto readDataValues = readDataWrapper->values();
    REQUIRE_THAT(readDataValues.data, Catch::Matchers::Approx(data).margin(1));

    // verify timestamps
    auto readTimestampsWrapper = readTs->readTimestamps();
    REQUIRE(readTimestampsWrapper->exists());
    auto readTimestampsValues = readTimestampsWrapper->values();
    REQUIRE(readTimestampsValues.data == timestamps);

    readio->close();
  }
}

TEST_CASE("ProcessingModule createDynamicTable and readDynamicTable",
          "[processingmodule]")
{
  std::string filename =
      getTestFilePath("createProcessingModuleDynamicTable.nwb");
  // cppcheck-suppress variableScope
  std::string tablePath = "/processing/analysis_module/summary_table";

  // --- Write ---
  {
    std::shared_ptr<IO::HDF5::HDF5IO> io =
        std::make_shared<IO::HDF5::HDF5IO>(filename);
    io->open();
    auto nwbfile = NWB::NWBFile::create(io);
    nwbfile->initialize(generateUuid());

    // create processing module
    auto processingModule = nwbfile->createProcessingModule("analysis_module");
    REQUIRE(processingModule != nullptr);
    REQUIRE(processingModule->initialize("Analysis results")
            == Status::Success);

    // create a DynamicTable inside the module via createDynamicTable
    auto table = processingModule->createDynamicTable("summary_table");
    REQUIRE(table != nullptr);
    REQUIRE(table->getPath() == tablePath);

    // initialize the table
    Status tableStatus = table->initialize("Summary statistics table");
    REQUIRE(tableStatus == Status::Success);

    // add a string column
    std::vector<std::string> colValues = {"alpha", "beta", "gamma"};
    SizeArray dataShape = {colValues.size()};
    IO::ArrayDataSetConfig config(BaseDataType::V_STR, dataShape, dataShape);
    auto vectorData = NWB::VectorData::create(tablePath + "/label", io);
    vectorData->initialize(config, "Label column");
    Status colStatus = table->addColumn(vectorData, colValues);
    REQUIRE(colStatus == Status::Success);

    // set row IDs
    std::vector<int> ids = {0, 1, 2};
    SizeArray idShape = {ids.size()};
    IO::ArrayDataSetConfig idConfig(BaseDataType::I32, idShape, idShape);
    auto elementIDs = NWB::ElementIdentifiers::create(tablePath + "/id", io);
    elementIDs->initialize(idConfig);
    REQUIRE(table->setRowIDs(elementIDs, ids) == Status::Success);

    // read back the table within the same session via readDynamicTable
    auto readTable = processingModule->readDynamicTable("summary_table");
    REQUIRE(readTable != nullptr);
    REQUIRE(readTable->getPath() == tablePath);

    REQUIRE(table->finalize() == Status::Success);

    nwbfile->finalize();
    io->close();
  }

  // --- Read back from file ---
  {
    std::shared_ptr<IO::HDF5::HDF5IO> readio =
        std::make_shared<IO::HDF5::HDF5IO>(filename);
    readio->open(IO::FileMode::ReadOnly);

    auto nwbfile = NWB::NWBFile::create(readio);
    auto processingModule = nwbfile->readProcessingModule("analysis_module");
    REQUIRE(processingModule != nullptr);

    // read the DynamicTable from the module
    auto readTable = processingModule->readDynamicTable("summary_table");
    REQUIRE(readTable != nullptr);

    // verify description
    auto readDesc = readTable->readDescription()->values().data;
    REQUIRE(readDesc[0] == "Summary statistics table");

    // verify column names include "label"
    auto readColNames = readTable->readColNames()->values().data;
    REQUIRE(std::find(readColNames.begin(), readColNames.end(), "label")
            != readColNames.end());

    readio->close();
  }
}

TEST_CASE("ProcessingModule initialize fails when IO is deleted",
          "[processingmodule]")
{
  std::shared_ptr<NWB::ProcessingModule> processingModule;

  // Create the ProcessingModule with a temporary IO that goes out of scope
  {
    std::shared_ptr<IO::HDF5::HDF5IO> io = std::make_shared<IO::HDF5::HDF5IO>(
        getTestFilePath("processingModuleDeletedIO.h5"));
    processingModule = NWB::RegisteredType::create<NWB::ProcessingModule>(
        "/processing/test", io);
    REQUIRE(processingModule != nullptr);
    // io goes out of scope here, expiring the weak_ptr inside processingModule
  }

  // initialize should return Failure since the IO object has been deleted
  Status result = processingModule->initialize("should fail");
  REQUIRE(result == Status::Failure);
}
