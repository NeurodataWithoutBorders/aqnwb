
#include <filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

// Get the current working directory
std::filesystem::path currentPath = std::filesystem::current_path();
#ifdef _WIN32
std::string executablePath =
    (currentPath / BUILD_CONFIG / "reader_executable.exe").string();
#else
std::string executablePath = "./reader_executable";
#endif

using namespace AQNWB;
namespace fs = std::filesystem;

TEST_CASE("writeGroup", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("testGroup.h5");
  IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  SECTION("initialize group")
  {
    hdf5io.createGroup("/data");
    hdf5io.close();
  }
}

TEST_CASE("searchGroup", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("searchGroup.h5");
  IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  hdf5io.createGroup("/data");
  hdf5io.createGroup("/data/test");
  hdf5io.createArrayDataSet(
      BaseDataType::I32, SizeArray {0}, SizeArray {1}, "/data/mydata");
  hdf5io.flush();
  auto group_content = hdf5io.getGroupObjects("/data");
  REQUIRE(group_content.size() == 2);
  auto group_content2 = hdf5io.getGroupObjects("/");
  REQUIRE(group_content2.size() == 1);
  REQUIRE(group_content2[0] == "data");
  hdf5io.close();
}

TEST_CASE("writeDataset", "[hdf5io]")
{
  std::vector<int> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  SECTION("write 1D data block to 1D dataset")
  {
    // open file
    std::string path = getTestFilePath("1DData1DDataset.h5");
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
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
    std::unique_ptr<IO::HDF5::HDF5RecordingData> datasetRead1D(
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(dataRead.release()));
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
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
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
    std::unique_ptr<IO::HDF5::HDF5RecordingData> dataset1DRead(
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(dataRead1D.release()));
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
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
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
    std::unique_ptr<IO::HDF5::HDF5RecordingData> data2DRead(
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(dsetRead2D.release()));
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
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
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
    std::unique_ptr<IO::HDF5::HDF5RecordingData> dSet1D(
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(dataRead1D.release()));
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
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
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
    std::unique_ptr<IO::HDF5::HDF5RecordingData> dSetRead2D(
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(dataset.release()));
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
  IO::HDF5::HDF5IO hdf5io(filename);
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
    REQUIRE(hdf5io.attributeExists("/data/array"));
  }

  // string array
  SECTION("str_array")
  {
    const std::vector<std::string> data = {"col1", "col2", "col3"};

    hdf5io.createAttribute(data, "/data", "string_array");
    REQUIRE(hdf5io.attributeExists("/data/string_array"));
  }

  // soft link
  SECTION("link")
  {
    std::vector<std::string> data;
    hdf5io.createLink("/data/link", "linked_data");
    REQUIRE(hdf5io.objectExists("/data/link"));
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
    std::string path = getTestFilePath("testSWMRmodeEnable.h5");
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // add a dataset
    std::vector<int> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string dataPath = "/data";
    SizeType numBlocks = 3;
    SizeType numSamples = testData.size();
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        BaseDataType::I32, SizeArray {0}, SizeArray {1}, dataPath);

    // try to read the file before starting SWMR mode
    std::string command = executablePath + " " + path + " " + dataPath;
    std::cout << "Executing command: " << command << std::endl;

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
#ifdef _WIN32
          // required on Windows to allow writer process to access file
          _putenv_s("HDF5_USE_FILE_LOCKING", "FALSE");
#endif
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
      H5Dflush(static_cast<IO::HDF5::HDF5RecordingData*>(dataset.get())
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
    std::string path = getTestFilePath("testSWMRmodeDisable.h5");
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path, true);
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
      H5Dflush(static_cast<IO::HDF5::HDF5RecordingData*>(dataset.get())
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
      H5Dflush(
          static_cast<IO::HDF5::HDF5RecordingData*>(datasetPostRestart.get())
              ->getDataSet()
              ->getId());
    }

    hdf5io->close();
  }
}

TEST_CASE("readDataset", "[hdf5io]")
{
  std::vector<int32_t> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  SECTION("read 1D data block of a 1D dataset")
  {
    // open file
    std::string path = getTestFilePath("Read1DData1DDataset.h5");
    std::shared_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_shared<IO::HDF5::HDF5IO>(path);
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

    // Confirm using HDF5IO readDataset that the data is correct
    auto readData = hdf5io->readDataset(dataPath);
    REQUIRE(readData.shape[0] == 10);
    auto readDataTyped = DataBlock<int32_t>::fromGeneric(readData);
    REQUIRE(readDataTyped.shape[0] == 10);
    REQUIRE(readDataTyped.data == testData);

    // Confirm using lazy read as well
    auto readDataWrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>>(
        hdf5io, dataPath);
    auto readDataGeneric = readDataWrapper->valuesGeneric();
    REQUIRE(readDataGeneric.shape[0] == 10);
    auto readDataTypedV2 = readDataWrapper->values();
    REQUIRE(readDataTypedV2.shape[0] == 10);
    REQUIRE(readDataTypedV2.data == testData);

    hdf5io->close();
  }
}

TEST_CASE("getH5ObjectType", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_getH5ObjectType.h5");
  IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  SECTION("group")
  {
    hdf5io.createGroup("/group");
    REQUIRE(hdf5io.getH5ObjectType("/group") == H5O_TYPE_GROUP);
  }

  SECTION("dataset")
  {
    std::vector<int> testData = {1, 2, 3, 4, 5};
    std::string dataPath = "/dataset";
    hdf5io.createArrayDataSet(
        BaseDataType::I32, SizeArray {0}, SizeArray {1}, dataPath);
    REQUIRE(hdf5io.getH5ObjectType(dataPath) == H5O_TYPE_DATASET);
  }

  SECTION("non_existent_object")
  {
    REQUIRE(hdf5io.getH5ObjectType("/non_existent") == H5O_TYPE_UNKNOWN);
  }

  // close file
  hdf5io.close();
}

TEST_CASE("HDF5IO::getNativeType", "[hdf5io]")
{
  SECTION("Integer Types")
  {
    // Test for T_I8
    IO::BaseDataType typeI8(IO::BaseDataType::T_I8, 1);
    H5::DataType nativeTypeI8 = IO::HDF5::HDF5IO::getNativeType(typeI8);
    REQUIRE(nativeTypeI8.getId() == H5::PredType::NATIVE_INT8.getId());

    // Test for T_I16
    IO::BaseDataType typeI16(IO::BaseDataType::T_I16, 1);
    H5::DataType nativeTypeI16 = IO::HDF5::HDF5IO::getNativeType(typeI16);
    REQUIRE(nativeTypeI16.getId() == H5::PredType::NATIVE_INT16.getId());

    // Test for T_I32
    IO::BaseDataType typeI32(IO::BaseDataType::T_I32, 1);
    H5::DataType nativeTypeI32 = IO::HDF5::HDF5IO::getNativeType(typeI32);
    REQUIRE(nativeTypeI32.getId() == H5::PredType::NATIVE_INT32.getId());

    // Test for T_I64
    IO::BaseDataType typeI64(IO::BaseDataType::T_I64, 1);
    H5::DataType nativeTypeI64 = IO::HDF5::HDF5IO::getNativeType(typeI64);
    REQUIRE(nativeTypeI64.getId() == H5::PredType::NATIVE_INT64.getId());

    // Test for T_U8
    IO::BaseDataType typeU8(IO::BaseDataType::T_U8, 1);
    H5::DataType nativeTypeU8 = IO::HDF5::HDF5IO::getNativeType(typeU8);
    REQUIRE(nativeTypeU8.getId() == H5::PredType::NATIVE_UINT8.getId());

    // Test for T_U16
    IO::BaseDataType typeU16(IO::BaseDataType::T_U16, 1);
    H5::DataType nativeTypeU16 = IO::HDF5::HDF5IO::getNativeType(typeU16);
    REQUIRE(nativeTypeU16.getId() == H5::PredType::NATIVE_UINT16.getId());

    // Test for T_U32
    IO::BaseDataType typeU32(IO::BaseDataType::T_U32, 1);
    H5::DataType nativeTypeU32 = IO::HDF5::HDF5IO::getNativeType(typeU32);
    REQUIRE(nativeTypeU32.getId() == H5::PredType::NATIVE_UINT32.getId());

    // Test for T_U64
    IO::BaseDataType typeU64(IO::BaseDataType::T_U64, 1);
    H5::DataType nativeTypeU64 = IO::HDF5::HDF5IO::getNativeType(typeU64);
    REQUIRE(nativeTypeU64.getId() == H5::PredType::NATIVE_UINT64.getId());
  }

  SECTION("Floating Point Types")
  {
    // Test for T_F32
    IO::BaseDataType typeF32(IO::BaseDataType::T_F32, 1);
    H5::DataType nativeTypeF32 = IO::HDF5::HDF5IO::getNativeType(typeF32);
    REQUIRE(nativeTypeF32.getId() == H5::PredType::NATIVE_FLOAT.getId());

    // Test for T_F64
    IO::BaseDataType typeF64(IO::BaseDataType::T_F64, 1);
    H5::DataType nativeTypeF64 = IO::HDF5::HDF5IO::getNativeType(typeF64);
    REQUIRE(nativeTypeF64.getId() == H5::PredType::NATIVE_DOUBLE.getId());
  }

  SECTION("String Types")
  {
    // Test for T_STR
    IO::BaseDataType typeSTR(IO::BaseDataType::T_STR, 256);
    H5::DataType nativeTypeSTR = IO::HDF5::HDF5IO::getNativeType(typeSTR);
    REQUIRE(nativeTypeSTR.getSize()
            == H5::StrType(H5::PredType::C_S1, 256).getSize());

    // Test for V_STR
    IO::BaseDataType typeVSTR(IO::BaseDataType::V_STR, 1);
    H5::DataType nativeTypeVSTR = IO::HDF5::HDF5IO::getNativeType(typeVSTR);
    REQUIRE(nativeTypeVSTR.getSize()
            == H5::StrType(H5::PredType::C_S1, H5T_VARIABLE).getSize());
  }

  SECTION("Array Types")
  {
    // Test for array types
    IO::BaseDataType typeArray(IO::BaseDataType::T_I32, 5);
    H5::DataType nativeTypeArray = IO::HDF5::HDF5IO::getNativeType(typeArray);
    hsize_t size = 5;
    H5::ArrayType expectedArrayType(H5::PredType::NATIVE_INT32, 1, &size);
    REQUIRE(nativeTypeArray.getSize() == expectedArrayType.getSize());
  }
}

TEST_CASE("HDF5IO::getH5Type", "[hdf5io]")
{
  SECTION("Integer Types")
  {
    // Test for T_I8
    IO::BaseDataType typeI8(IO::BaseDataType::T_I8, 1);
    H5::DataType h5TypeI8 = IO::HDF5::HDF5IO::getH5Type(typeI8);
    REQUIRE(h5TypeI8.getSize() == H5::PredType::STD_I8LE.getSize());

    // Test for T_I16
    IO::BaseDataType typeI16(IO::BaseDataType::T_I16, 1);
    H5::DataType h5TypeI16 = IO::HDF5::HDF5IO::getH5Type(typeI16);
    REQUIRE(h5TypeI16.getSize() == H5::PredType::STD_I16LE.getSize());

    // Test for T_I32
    IO::BaseDataType typeI32(IO::BaseDataType::T_I32, 1);
    H5::DataType h5TypeI32 = IO::HDF5::HDF5IO::getH5Type(typeI32);
    REQUIRE(h5TypeI32.getSize() == H5::PredType::STD_I32LE.getSize());

    // Test for T_I64
    IO::BaseDataType typeI64(IO::BaseDataType::T_I64, 1);
    H5::DataType h5TypeI64 = IO::HDF5::HDF5IO::getH5Type(typeI64);
    REQUIRE(h5TypeI64.getSize() == H5::PredType::STD_I64LE.getSize());

    // Test for T_U8
    IO::BaseDataType typeU8(IO::BaseDataType::T_U8, 1);
    H5::DataType h5TypeU8 = IO::HDF5::HDF5IO::getH5Type(typeU8);
    REQUIRE(h5TypeU8.getSize() == H5::PredType::STD_U8LE.getSize());

    // Test for T_U16
    IO::BaseDataType typeU16(IO::BaseDataType::T_U16, 1);
    H5::DataType h5TypeU16 = IO::HDF5::HDF5IO::getH5Type(typeU16);
    REQUIRE(h5TypeU16.getSize() == H5::PredType::STD_U16LE.getSize());

    // Test for T_U32
    IO::BaseDataType typeU32(IO::BaseDataType::T_U32, 1);
    H5::DataType h5TypeU32 = IO::HDF5::HDF5IO::getH5Type(typeU32);
    REQUIRE(h5TypeU32.getSize() == H5::PredType::STD_U32LE.getSize());

    // Test for T_U64
    IO::BaseDataType typeU64(IO::BaseDataType::T_U64, 1);
    H5::DataType h5TypeU64 = IO::HDF5::HDF5IO::getH5Type(typeU64);
    REQUIRE(h5TypeU64.getSize() == H5::PredType::STD_U64LE.getSize());
  }

  SECTION("Floating Point Types")
  {
    // Test for T_F32
    IO::BaseDataType typeF32(IO::BaseDataType::T_F32, 1);
    H5::DataType h5TypeF32 = IO::HDF5::HDF5IO::getH5Type(typeF32);
    REQUIRE(h5TypeF32.getSize() == H5::PredType::IEEE_F32LE.getSize());

    // Test for T_F64
    IO::BaseDataType typeF64(IO::BaseDataType::T_F64, 1);
    H5::DataType h5TypeF64 = IO::HDF5::HDF5IO::getH5Type(typeF64);
    REQUIRE(h5TypeF64.getSize() == H5::PredType::IEEE_F64LE.getSize());
  }

  SECTION("String Types")
  {
    // Test for T_STR
    IO::BaseDataType typeSTR(IO::BaseDataType::T_STR, 256);
    H5::DataType h5TypeSTR = IO::HDF5::HDF5IO::getH5Type(typeSTR);
    REQUIRE(h5TypeSTR.getSize()
            == H5::StrType(H5::PredType::C_S1, 256).getSize());

    // Test for V_STR
    IO::BaseDataType typeVSTR(IO::BaseDataType::V_STR, 1);
    H5::DataType h5TypeVSTR = IO::HDF5::HDF5IO::getH5Type(typeVSTR);
    REQUIRE(h5TypeVSTR.getSize()
            == H5::StrType(H5::PredType::C_S1, H5T_VARIABLE).getSize());
  }

  SECTION("Array Types")
  {
    // Test for array types
    IO::BaseDataType typeArray(IO::BaseDataType::T_I32, 5);
    H5::DataType h5TypeArray = IO::HDF5::HDF5IO::getH5Type(typeArray);
    hsize_t size = 5;
    H5::ArrayType expectedArrayType(H5::PredType::STD_I32LE, 1, &size);
    REQUIRE(h5TypeArray.getSize() == expectedArrayType.getSize());
  }
}