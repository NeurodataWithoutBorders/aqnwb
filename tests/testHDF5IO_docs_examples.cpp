// HDF5 I/O Examples used in the Documentation
#include <filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;
namespace fs = std::filesystem;

TEST_CASE("SWMRmodeExamples", "[hdf5io]")
{
  SECTION("withSWMRMode")
  {
    // [example_HDF5_with_SWMR_mode]
    // create and open the HDF5 file. SWMR mode is used by default
    std::string path = getTestFilePath("testWithSWMRMode.h5");
    std::unique_ptr<HDF5::HDF5IO> hdf5io = std::make_unique<HDF5::HDF5IO>(path);
    hdf5io->open();

    // add a dataset
    std::vector<int> testData(10000);  // Initialize the testData to 0, 1, 2, ... 10000 with std::iota
    std::iota(testData.begin(),
              testData.end(),
              1);
    std::string dataPath = "/data";
    SizeType numBlocks = 10;  // write 10 chunks of
    SizeType numSamples = testData.size();
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        BaseDataType::I32,  // type
        SizeArray {0},  // size. Initial size of the dataset
        SizeArray {1000},  // chunking. Size of a data chunk
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
    // TODO
  }
}
