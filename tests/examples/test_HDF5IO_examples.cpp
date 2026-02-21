// HDF5 I/O Examples used in the Documentation
// [example_HDF5_includes]
#include <filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/file/ElectrodesTable.hpp"
#include "testUtils.hpp"

namespace fs = std::filesystem;
// [example_HDF5_includes]

TEST_CASE("SWMRmodeExamples", "[hdf5io]")
{
  SECTION("withSWMRMode")
  {
    // [example_HDF5_with_SWMR_mode]
    // create and open the HDF5 file. SWMR mode is used by default
    std::string path = getTestFilePath("testWithSWMRMode.h5");
    std::unique_ptr<AQNWB::IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<AQNWB::IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // add a dataset
    std::vector<int> testData(10000);
    std::iota(testData.begin(), testData.end(), 1);  // Initialize testData
    std::string dataPath = "/data";
    SizeType numBlocks = 10;  // write 10 chunks of
    SizeType numSamples = testData.size();
    AQNWB::IO::ArrayDataSetConfig datasetConfig(
        BaseDataType::I32,  // type
        SizeArray {0},  // size. Initial size of the dataset
        SizeArray {1000}  // chunking. Size of a data chunk
    );
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        datasetConfig,
        dataPath);  // path. Path to the dataset in the HDF5 file

    // Start recording. Starting the recording places the HDF5 file in SWMR mode
    Status status = hdf5io->startRecording();
    REQUIRE(status == Status::Success);

    // Once in SWMR mode we can add data to the file but we can no longer create
    // new data objects (Groups, Datasets, Attributes etc.).
    REQUIRE(hdf5io->canModifyObjects() == false);

    // write the our testData to the file.
    for (SizeType b = 0; b <= numBlocks; b++) {
      // write a single 1D block of data and flush to file
      SizeArray dataShape = {numSamples};
      dataset->writeDataBlock(dataShape, BaseDataType::I32, &testData[0]);
      // Optionally we can flush all data to disk
      status = hdf5io->flush();
      REQUIRE(status == Status::Success);
    }

    // stop recording. In SWMR mode the file is now closed and recording cannot
    // be restarted
    status = hdf5io->stopRecording();
    REQUIRE(hdf5io->isOpen() == false);
    REQUIRE(hdf5io->startRecording() == Status::Failure);
    // [example_HDF5_with_SWMR_mode]
  }

  SECTION("disableSWMRMode")
  {
    // [example_HDF5_without_SWMR_mode]
    // create and open the HDF5 file. With SWMR mode explicitly disabled
    std::string path = getTestFilePath("testWithoutSWMRMode.h5");
    std::unique_ptr<AQNWB::IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<AQNWB::IO::HDF5::HDF5IO>(path,
                                                  true  // Disable SWMR mode
        );
    hdf5io->open();

    // add a dataset
    std::vector<int> testData(10000);
    std::iota(testData.begin(), testData.end(), 1);  // Initialize testData
    std::string dataPath = "/data";
    SizeType numBlocks = 10;  // write 10 chunks of
    SizeType numSamples = testData.size();
    AQNWB::IO::ArrayDataSetConfig datasetConfig(
        BaseDataType::I32,  // type
        SizeArray {0},  // size. Initial size of the dataset
        SizeArray {1000}  // chunking. Size of a data chunk
    );
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        datasetConfig,
        dataPath);  // path. Path to the dataset in the HDF5 file

    // Start recording. Starting the recording places the HDF5 file in SWMR mode
    Status status = hdf5io->startRecording();
    REQUIRE(status == Status::Success);

    // With SWMR mode disabled we are still allowed to create new data objects
    // (Groups, Datasets, Attributes etc.) during the recording. However, with
    // SWMR mode disabled, we lose the data consistency and concurrent read
    // features that SWMR mode provides.
    REQUIRE(hdf5io->canModifyObjects() == true);

    // write the our testData to the file.
    for (SizeType b = 0; b <= numBlocks; b++) {
      // write a single 1D block of data and flush to file
      SizeArray dataShape = {numSamples};
      dataset->writeDataBlock(dataShape, BaseDataType::I32, &testData[0]);
      // Optionally we can flush all data to disk
      status = hdf5io->flush();
      REQUIRE(status == Status::Success);
    }

    // stop recording.
    status = hdf5io->stopRecording();

    // Since SWMR mode is disabled, stopping the recording won't close the file
    // so that we can restart the recording if we want to
    REQUIRE(hdf5io->isOpen() == true);

    // Restart the recording
    REQUIRE(hdf5io->startRecording() == Status::Success);

    // Stop the recording and close the file
    hdf5io->stopRecording();
    hdf5io->close();
    REQUIRE(hdf5io->isOpen() == false);
    // [example_HDF5_without_SWMR_mode]
  }
}

TEST_CASE("HDF5FiltersExamples", "[hdf5io]")
{
  SECTION("usingFilters")
  {
    // [example_HDF5_with_filters]
    // Create the HDF5IO object and open the file as usual
    std::string path = getTestFilePath("testWithFilters.h5");
    std::unique_ptr<AQNWB::IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<AQNWB::IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // Define the data type, shape, and chunking
    AQNWB::IO::BaseDataType type(AQNWB::IO::BaseDataType::Type::T_I32, 1);
    SizeArray shape = {100, 100};
    SizeArray chunking = {10, 10};

    // Create HDF5ArrayDataSetConfig and add filters
    AQNWB::IO::HDF5::HDF5ArrayDataSetConfig config(type, shape, chunking);
    unsigned int gzip_level = 4;
    config.addFilter(
        AQNWB::IO::HDF5::HDF5FilterConfig::createGzipFilter(gzip_level));
    config.addFilter(AQNWB::IO::HDF5::HDF5FilterConfig::createShuffleFilter());

    // Create the dataset
    auto baseDataset = hdf5io->createArrayDataSet(config, "/filtered_dataset");

    // [Optional/Testing] Verify the dataset properties
    auto dataset =
        dynamic_cast<AQNWB::IO::HDF5::HDF5RecordingData*>(baseDataset.get());
    const H5::DataSet* h5Dataset = dataset->getDataSet();
    H5::DSetCreatPropList dcpl = h5Dataset->getCreatePlist();
    REQUIRE(dcpl.getNfilters() == 2);
    // [example_HDF5_with_filters]
  }
}
