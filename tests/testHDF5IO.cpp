
#include <filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Types.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

#ifdef _WIN32
#  define EXECUTABLE_NAME ".\\tests\\Release\\reader_executable.exe"
#else
#  define EXECUTABLE_NAME "./reader_executable"
#endif

using namespace AQNWB;
namespace fs = std::filesystem;

TEST_CASE("writeGroup", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_group.h5");
  HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  SECTION("initialize group")
  {
    hdf5io.createGroup("/data");
    hdf5io.close();
  }
}

TEST_CASE("writeDataset", "[hdf5io]")
{
  std::vector<int> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  SECTION("write 1D data block to 1D dataset")
  {
    // open file
    std::string path = getTestFilePath("1DData1DDataset.h5");
    std::unique_ptr<HDF5::HDF5IO> hdf5io = std::make_unique<HDF5::HDF5IO>(path);
    hdf5io->open();

    // Set up test data
    std::string dataPath = "/1DData1DDataset";
    SizeType numSamples = 10;

    // Create HDF5RecordingData object and dataset
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        BaseDataType::I32, SizeArray {0}, SizeArray {1}, dataPath);

    // Write data block
    std::vector<SizeType> dataShape = {numSamples};
    std::vector<SizeType> positionOffset = {0};
    dataset->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, &testData[0]);

    std::unique_ptr<BaseRecordingData> dataRead = hdf5io->getDataSet(dataPath);
    std::unique_ptr<HDF5::HDF5RecordingData> datasetRead1D(
        dynamic_cast<HDF5::HDF5RecordingData*>(dataRead.release()));
    int* buffer = new int[numSamples];
    readH5DataBlock(datasetRead1D->getDataSet(), BaseDataType::I32, buffer);
    std::vector<int> dataOut(buffer, buffer + numSamples);
    delete[] buffer;

    REQUIRE(dataOut == testData);
    hdf5io->close();
  }

  SECTION("write 1D data block to 2D dataset")
  {
    // open file
    std::string path = getTestFilePath("1DData2DDataset.h5");
    std::unique_ptr<HDF5::HDF5IO> hdf5io = std::make_unique<HDF5::HDF5IO>(path);
    hdf5io->open();

    // Set up test data for 3D
    SizeType numRows = 1, numCols = 10;
    std::string dataPath = "/1DData2DDataset";
    std::vector<SizeType> dataShape = {numRows, numCols};
    std::vector<SizeType> positionOffset = {0, 0};

    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(BaseDataType::I32,
                                   SizeArray {numRows, numCols},
                                   SizeArray {0, 0},
                                   dataPath);
    Status status = dataset->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, testData.data());
    REQUIRE(status == Status::Success);

    // Read back the 1D data block from 3D dataset
    std::unique_ptr<BaseRecordingData> dataRead1D =
        hdf5io->getDataSet(dataPath);
    std::unique_ptr<HDF5::HDF5RecordingData> dataset1DRead(
        dynamic_cast<HDF5::HDF5RecordingData*>(dataRead1D.release()));
    int* buffer1D = new int[numCols];
    readH5DataBlock(dataset1DRead->getDataSet(), BaseDataType::I32, buffer1D);
    std::vector<int> dataOut1D(buffer1D, buffer1D + numCols);
    delete[] buffer1D;

    // Check if the written and read data match for 1D data block in 3D dataset
    REQUIRE(dataOut1D == testData);
    hdf5io->close();
  }

  SECTION("write 2D data block to 2D dataset")
  {
    // open file
    std::string path = getTestFilePath("2DData2DDataset.h5");
    std::unique_ptr<HDF5::HDF5IO> hdf5io = std::make_unique<HDF5::HDF5IO>(path);
    hdf5io->open();

    // Set up test data for 2D
    SizeType numRows = 2, numCols = 5;
    std::string dataPath = "/2DData2DDataset";
    std::vector<SizeType> dataShape = {numRows, numCols};
    std::vector<SizeType> positionOffset = {0, 0};

    // Create HDF5RecordingData object and dataset for 2D data
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        BaseDataType::I32,
        SizeArray {numRows, numCols},  // Initial size
        SizeArray {0, 0},  // chunking
        dataPath);

    // Write 2D data block
    Status status = dataset->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, testData.data());
    REQUIRE(status == Status::Success);

    // Read back the 2D data block
    std::unique_ptr<BaseRecordingData> dsetRead2D =
        hdf5io->getDataSet(dataPath);
    std::unique_ptr<HDF5::HDF5RecordingData> data2DRead(
        dynamic_cast<HDF5::HDF5RecordingData*>(dsetRead2D.release()));
    int* buffer = new int[numRows * numCols];
    readH5DataBlock(data2DRead->getDataSet(), BaseDataType::I32, buffer);
    std::vector<int> dataOut(buffer, buffer + numRows * numCols);
    delete[] buffer;

    // Check if the written and read data match
    REQUIRE(dataOut == testData);
    hdf5io->close();
  }

  SECTION("write 1D data block to 3D dataset")
  {
    // open file
    std::string path = getTestFilePath("1DData3DDataset.h5");
    std::unique_ptr<HDF5::HDF5IO> hdf5io = std::make_unique<HDF5::HDF5IO>(path);
    hdf5io->open();

    // Set up test data for 3D
    SizeType depth = 1, height = 1, width = 10;
    std::string dataPath = "1DData3DDataset";
    std::vector<SizeType> dataShape = {depth, height, width};
    std::vector<SizeType> positionOffset = {0, 0, 0};

    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(BaseDataType::I32,
                                   SizeArray {depth, height, width},
                                   SizeArray {0, 0, 0},
                                   dataPath);
    Status status = dataset->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, testData.data());
    REQUIRE(status == Status::Success);

    // Read back the 1D data block from 3D dataset
    std::unique_ptr<BaseRecordingData> dataRead1D =
        hdf5io->getDataSet(dataPath);
    std::unique_ptr<HDF5::HDF5RecordingData> dSet1D(
        dynamic_cast<HDF5::HDF5RecordingData*>(dataRead1D.release()));
    int* buffer1D =
        new int[width];  // Assuming 'width' is the size of the 1D data block
    readH5DataBlock(dSet1D->getDataSet(), BaseDataType::I32, buffer1D);
    std::vector<int> dataOut1D(buffer1D, buffer1D + width);
    delete[] buffer1D;

    // Check if the written and read data match for 1D data block in 3D dataset
    REQUIRE(dataOut1D == testData);
    hdf5io->close();
  }

  SECTION("write 2D data block to 3D dataset")
  {
    // open file
    std::string path = getTestFilePath("2DData3DDataset.h5");
    std::unique_ptr<HDF5::HDF5IO> hdf5io = std::make_unique<HDF5::HDF5IO>(path);
    hdf5io->open();

    SizeType depth = 1, height = 2, width = 5;
    std::string dataPath = "2DData3DDataset";
    std::vector<SizeType> dataShape = {depth, height, width};
    std::vector<SizeType> positionOffset = {0, 0, 0};

    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(BaseDataType::I32,
                                   SizeArray {depth, height, width},
                                   SizeArray {0, 0, 0},
                                   dataPath);
    Status status = dataset->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, testData.data());
    REQUIRE(status == Status::Success);

    // Read back the 2D data block from 3D dataset
    std::unique_ptr<BaseRecordingData> dataRead2D =
        hdf5io->getDataSet(dataPath);
    std::unique_ptr<HDF5::HDF5RecordingData> dSetRead2D(
        dynamic_cast<HDF5::HDF5RecordingData*>(dataset.release()));
    int* buffer2D =
        new int[height * width];  // Assuming 'numRows' and 'numCols' define the
                                  // 2D data block size
    readH5DataBlock(dSetRead2D->getDataSet(), BaseDataType::I32, buffer2D);
    std::vector<int> dataOut2D(buffer2D, buffer2D + height * width);
    delete[] buffer2D;

    // Check if the written and read data match for 2D data block in 3D dataset
    REQUIRE(dataOut2D == testData);
    hdf5io->close();
  }
}

TEST_CASE("writeAttributes", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_attributes.h5");
  HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  hdf5io.createGroup("/data");

  // single attribute
  SECTION("single_value")
  {
    const signed int data = 1;
    hdf5io.createAttribute(BaseDataType::I32, &data, "/data", "single_value");
  }

  // integer array
  SECTION("int_array")
  {
    const int data[] = {1, 2, 3, 4, 5};
    const int dataSize = sizeof(data) / sizeof(data[0]);

    hdf5io.createAttribute(
        BaseDataType::I32, &data, "/data", "array", dataSize);
  }

  // string array
  SECTION("str_array")
  {
    const std::vector<std::string> data = {"col1", "col2", "col3"};

    hdf5io.createAttribute(data, "/data", "string_array");
  }

  // soft link
  SECTION("link")
  {
    std::vector<std::string> data;
    hdf5io.createLink("/data/link", "linked_data");
  }

  // reference
  SECTION("reference")
  {
    // TODO
  }

  // close file
  hdf5io.close();
}

TEST_CASE("SWMRmode", "[hdf5io]")
{
  SECTION("useSWMRMODE")
  {
    // create and open file
    std::string path = getTestFilePath("testSWMRmode.h5");
    std::unique_ptr<HDF5::HDF5IO> hdf5io = std::make_unique<HDF5::HDF5IO>(path);
    hdf5io->open();

    // add a dataset
    std::vector<int> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string dataPath = "/data";
    SizeType numBlocks = 3;
    SizeType numSamples = testData.size();
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        BaseDataType::I32, SizeArray {0}, SizeArray {1}, dataPath);

    // try to read the file before starting SWMR mode
    std::string command =
        std::string(EXECUTABLE_NAME) + " " + path + " " + dataPath;
    int retPreSWMREnabled = std::system(command.c_str());
    REQUIRE(retPreSWMREnabled
            != 0);  // process should fail if SWMR mode is not enabled

    // start recording, check that objects cannot be modified
    Status status = hdf5io->startRecording();
    REQUIRE(status == Status::Success);
    REQUIRE(hdf5io->canModifyObjects() == false);

    // Try to read the file after starting SWMR mode
    std::promise<int> promise;
    std::future<int> future = promise.get_future();
    std::thread readerThread(
        [](const std::string& cmd, std::promise<int> promise)
        {
          int ret = std::system(cmd.c_str());
          promise.set_value(ret);
        },
        command,
        std::move(promise));

    // write to file
    for (SizeType b = 0; b <= numBlocks; b++) {
      // write data block and flush to file
      std::vector<SizeType> dataShape = {numSamples};
      dataset->writeDataBlock(dataShape, BaseDataType::I32, &testData[0]);
      H5Dflush(static_cast<HDF5::HDF5RecordingData*>(dataset.get())
                   ->getDataSet()
                   ->getId());

      // update test data values
      for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] += (testData.size());
      }

      // pause to simulate streaming
      std::this_thread::sleep_for(
          std::chrono::seconds(1));  // Simulate real-time data streaming
    }

    // wait for reader to finish and close file
    readerThread.join();
    int retSWMREnabled = future.get();
    REQUIRE(retSWMREnabled == 0);  // process should succeed if data was written
                                   // and read successfully

    // test flush data to disk
    REQUIRE(hdf5io->flush() == Status::Success);

    // stop recording, check that file is closed and recording cannot be
    // restarted
    status = hdf5io->stopRecording();
    REQUIRE(hdf5io->isOpen() == false);
    REQUIRE(hdf5io->startRecording() == Status::Failure);
  }

  SECTION("disableSWMRMode")
  {
    // create and open file with SWMR mode disabled
    std::string path = getTestFilePath("testSWMRmode.h5");
    std::unique_ptr<HDF5::HDF5IO> hdf5io =
        std::make_unique<HDF5::HDF5IO>(path, true);
    hdf5io->open();

    // add a dataset
    std::vector<int> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string dataPath = "/data";
    SizeType numBlocks = 3;
    SizeType numSamples = testData.size();
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        BaseDataType::I32, SizeArray {0}, SizeArray {1}, dataPath);

    // start recording, check that can still modify objects
    Status status = hdf5io->startRecording();
    REQUIRE(status == Status::Success);
    REQUIRE(hdf5io->canModifyObjects() == true);

    // write to file
    for (SizeType b = 0; b <= numBlocks; b++) {
      // write data block and flush to file
      std::vector<SizeType> dataShape = {numSamples};
      dataset->writeDataBlock(dataShape, BaseDataType::I32, &testData[0]);
      H5Dflush(static_cast<HDF5::HDF5RecordingData*>(dataset.get())
                   ->getDataSet()
                   ->getId());

      // update test data values
      for (size_t i = 0; i < testData.size(); ++i) {
        testData[i] += (testData.size());
      }
    }

    // stop recording, check that file is still open and recording can be
    // restarted
    status = hdf5io->stopRecording();
    REQUIRE(hdf5io->isOpen() == true);

    // restart recording and write to a dataset
    Status statusRestart = hdf5io->startRecording();
    REQUIRE(statusRestart == Status::Success);

    std::string dataPathPostRestart = "/dataPostRestart/data";
    hdf5io->createGroup("/dataPostRestart");
    std::unique_ptr<BaseRecordingData> datasetPostRestart =
        hdf5io->createArrayDataSet(BaseDataType::I32,
                                   SizeArray {0},
                                   SizeArray {1},
                                   dataPathPostRestart);

    for (SizeType b = 0; b <= numBlocks; b++) {
      // write data block and flush to file
      std::vector<SizeType> dataShape = {numSamples};
      datasetPostRestart->writeDataBlock(
          dataShape, BaseDataType::I32, &testData[0]);
      H5Dflush(static_cast<HDF5::HDF5RecordingData*>(datasetPostRestart.get())
                   ->getDataSet()
                   ->getId());
    }

    hdf5io->close();
  }
}
