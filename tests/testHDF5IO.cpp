
#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Types.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

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

  SECTION("try initializing group that already exists")
  {
    // TODO
  }

  SECTION("try initializing group without parent group")
  {
    // TODO
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
    BaseRecordingData* dataset = hdf5io->createDataSet(
        BaseDataType::I32, SizeArray {0}, SizeArray {1}, dataPath);

    // Write data block
    std::vector<SizeType> dataShape = {numSamples};
    std::vector<SizeType> positionOffset = {0};
    static_cast<HDF5::HDF5RecordingData*>(dataset)->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, &testData[0]);

    BaseRecordingData* dataRead = hdf5io->getDataSet(dataPath);
    int* buffer = new int[numSamples];
    static_cast<HDF5::HDF5RecordingData*>(dataRead)->readDataBlock(
        BaseDataType::I32, buffer);
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

    BaseRecordingData* dataset =
        hdf5io->createDataSet(BaseDataType::I32,
                              SizeArray {numRows, numCols},
                              SizeArray {0, 0},
                              dataPath);
    Status status =
        static_cast<HDF5::HDF5RecordingData*>(dataset)->writeDataBlock(
            dataShape, positionOffset, BaseDataType::I32, testData.data());

    // Read back the 1D data block from 3D dataset
    BaseRecordingData* dataRead1D = hdf5io->getDataSet(dataPath);
    int* buffer1D = new int[numCols];
    static_cast<HDF5::HDF5RecordingData*>(dataRead1D)
        ->readDataBlock(BaseDataType::I32, buffer1D);
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
    BaseRecordingData* dataset =
        hdf5io->createDataSet(BaseDataType::I32,
                              SizeArray {numRows, numCols},  // Initial size
                              SizeArray {0, 0},  // chunking
                              dataPath);

    // Write 2D data block
    Status status =
        static_cast<HDF5::HDF5RecordingData*>(dataset)->writeDataBlock(
            dataShape, positionOffset, BaseDataType::I32, testData.data());

    // Read back the 2D data block
    BaseRecordingData* dataRead = hdf5io->getDataSet(dataPath);
    int* buffer = new int[numRows * numCols];
    static_cast<HDF5::HDF5RecordingData*>(dataRead)->readDataBlock(
        BaseDataType::I32, buffer);
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

    BaseRecordingData* dataset =
        hdf5io->createDataSet(BaseDataType::I32,
                              SizeArray {depth, height, width},
                              SizeArray {0, 0, 0},
                              dataPath);
    Status status =
        static_cast<HDF5::HDF5RecordingData*>(dataset)->writeDataBlock(
            dataShape, positionOffset, BaseDataType::I32, testData.data());

    // Read back the 1D data block from 3D dataset
    BaseRecordingData* dataRead1D = hdf5io->getDataSet(dataPath);
    int* buffer1D =
        new int[width];  // Assuming 'width' is the size of the 1D data block
    static_cast<HDF5::HDF5RecordingData*>(dataRead1D)
        ->readDataBlock(BaseDataType::I32, buffer1D);
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

    BaseRecordingData* dataset =
        hdf5io->createDataSet(BaseDataType::I32,
                              SizeArray {depth, height, width},
                              SizeArray {0, 0, 0},
                              dataPath);
    Status status =
        static_cast<HDF5::HDF5RecordingData*>(dataset)->writeDataBlock(
            dataShape, positionOffset, BaseDataType::I32, testData.data());

    // Read back the 2D data block from 3D dataset
    BaseRecordingData* dataRead2D = hdf5io->getDataSet(dataPath);
    int* buffer2D =
        new int[height * width];  // Assuming 'numRows' and 'numCols' define the
                                  // 2D data block size
    static_cast<HDF5::HDF5RecordingData*>(dataRead2D)
        ->readDataBlock(BaseDataType::I32, buffer2D);
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