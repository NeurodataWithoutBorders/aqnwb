// HDF5 I/O Examples used in the Documentation
// [example_HDF5_includes]
#include <filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;
namespace fs = std::filesystem;
// [example_HDF5_includes]

TEST_CASE("SWMRmodeExamples", "[hdf5io]")
{
  SECTION("withSWMRMode")
  {
    // [example_HDF5_with_SWMR_mode]
    // create and open the HDF5 file. SWMR mode is used by default
    std::string path = getTestFilePath("testWithSWMRMode.h5");
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // add a dataset
    std::vector<int> testData(10000);
    std::iota(testData.begin(), testData.end(), 1);  // Initialize testData
    std::string dataPath = "/data";
    SizeType numBlocks = 10;  // write 10 chunks of
    SizeType numSamples = testData.size();
    IO::ArrayDataSetConfig datasetConfig(
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
      std::vector<SizeType> dataShape = {numSamples};
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
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path,
                                           true  // Disable SWMR mode
        );
    hdf5io->open();

    // add a dataset
    std::vector<int> testData(10000);
    std::iota(testData.begin(), testData.end(), 1);  // Initialize testData
    std::string dataPath = "/data";
    SizeType numBlocks = 10;  // write 10 chunks of
    SizeType numSamples = testData.size();
    IO::ArrayDataSetConfig datasetConfig(
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
      std::vector<SizeType> dataShape = {numSamples};
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
