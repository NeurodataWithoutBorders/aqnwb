
#include <filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <H5Rpublic.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"
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

TEST_CASE("open - hdf5 file modes", "[hdf5io]")
{
  const std::string fileName = getTestFilePath("test_open_modes.h5");

  SECTION("Open file in Overwrite mode")
  {
    IO::HDF5::HDF5IO hdf5io(fileName);
    REQUIRE(hdf5io.open(IO::FileMode::Overwrite) == Status::Success);
    REQUIRE(hdf5io.isOpen());

    // Verify file is created and opened in Overwrite mode
    H5::H5File file(fileName, H5F_ACC_RDONLY);
    REQUIRE(file.getId() >= 0);

    // Clean up
    file.close();
    hdf5io.close();
    std::filesystem::remove(fileName);
  }

  SECTION("Open file in ReadWrite mode")
  {
    // Create a file first
    {
      IO::HDF5::HDF5IO hdf5io(fileName);
      REQUIRE(hdf5io.open(IO::FileMode::Overwrite) == Status::Success);
      hdf5io.close();
    }

    // Open the existing file in ReadWrite mode
    IO::HDF5::HDF5IO hdf5io(fileName);
    REQUIRE(hdf5io.open(IO::FileMode::ReadWrite) == Status::Success);
    REQUIRE(hdf5io.isOpen());

    // Verify file is opened in ReadWrite mode
    H5::H5File file(fileName, H5F_ACC_RDWR);
    REQUIRE(file.getId() >= 0);

    // Clean up
    file.close();
    hdf5io.close();
    std::filesystem::remove(fileName);
  }

  SECTION("Open file in ReadOnly mode")
  {
    // Create a file first
    {
      IO::HDF5::HDF5IO hdf5io(fileName);
      REQUIRE(hdf5io.open(IO::FileMode::Overwrite) == Status::Success);
      hdf5io.close();
    }

    // Open the existing file in ReadOnly mode
    IO::HDF5::HDF5IO hdf5io(fileName);
    REQUIRE(hdf5io.open(IO::FileMode::ReadOnly) == Status::Success);
    REQUIRE(hdf5io.isOpen());

    // Verify file is opened in ReadOnly mode
    H5::H5File file(fileName, H5F_ACC_RDONLY | H5F_ACC_SWMR_READ);
    REQUIRE(file.getId() >= 0);

    // Clean up
    file.close();
    hdf5io.close();
    std::filesystem::remove(fileName);
  }

  SECTION("Open non-existent file in ReadWrite mode")
  {
    IO::HDF5::HDF5IO hdf5io(fileName);
    REQUIRE(hdf5io.open(IO::FileMode::ReadWrite) == Status::Failure);
    REQUIRE_FALSE(hdf5io.isOpen());
  }

  SECTION("Open non-existent file in ReadOnly mode")
  {
    IO::HDF5::HDF5IO hdf5io(fileName);
    REQUIRE(hdf5io.open(IO::FileMode::ReadOnly) == Status::Failure);
    REQUIRE_FALSE(hdf5io.isOpen());
  }
}

TEST_CASE("createGroup", "[hdf5io]")
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

TEST_CASE("getStorageObjects", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_getStorageObjects.h5");
  AQNWB::IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  SECTION("empty group")
  {
    hdf5io.createGroup("/emptyGroup");
    auto groupContent = hdf5io.getStorageObjects("/emptyGroup");
    REQUIRE(groupContent.size() == 0);
  }

  SECTION("attribute")
  {
    int attrData1 = 42;
    hdf5io.createAttribute(BaseDataType::I32, &attrData1, "/", "attr1");
    auto attributeContent = hdf5io.getStorageObjects("/attr1");
    REQUIRE(attributeContent.size() == 0);
  }

  SECTION("dataset w/o attribute")
  {
    // Dataset without attributes
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    hdf5io.createArrayDataSet(config, "/data");
    auto datasetContent = hdf5io.getStorageObjects("/data");
    REQUIRE(datasetContent.size() == 0);

    // Dataset with attribute
    int attrData1 = 42;
    hdf5io.createAttribute(BaseDataType::I32, &attrData1, "/data", "attr1");
    auto dataContent2 = hdf5io.getStorageObjects("/data");
    REQUIRE(dataContent2.size() == 1);
    REQUIRE(
        dataContent2[0]
        == std::make_pair(std::string("attr1"), StorageObjectType::Attribute));
  }

  SECTION("invalid path")
  {
    auto invalidPathContent = hdf5io.getStorageObjects("/invalid/path");
    REQUIRE(invalidPathContent.size() == 0);
  }

  SECTION("group with datasets, subgroups, and attributes")
  {
    hdf5io.createGroup("/data");
    hdf5io.createGroup("/data/subgroup1");
    hdf5io.createGroup("/data/subgroup2");
    IO::ArrayDataSetConfig config1 {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    hdf5io.createArrayDataSet(config1, "/data/dataset1");
    IO::ArrayDataSetConfig config2 {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    hdf5io.createArrayDataSet(config2, "/data/dataset2");

    // Add attributes to the group
    int attrData1 = 42;
    hdf5io.createAttribute(BaseDataType::I32, &attrData1, "/data", "attr1");
    int attrData2 = 43;
    hdf5io.createAttribute(BaseDataType::I32, &attrData2, "/data", "attr2");

    auto groupContent = hdf5io.getStorageObjects("/data");
    REQUIRE(groupContent.size() == 6);
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("subgroup1"),
                                     StorageObjectType::Group))
            != groupContent.end());
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("subgroup2"),
                                     StorageObjectType::Group))
            != groupContent.end());
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("dataset1"),
                                     StorageObjectType::Dataset))
            != groupContent.end());
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("dataset2"),
                                     StorageObjectType::Dataset))
            != groupContent.end());
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("attr1"),
                                     StorageObjectType::Attribute))
            != groupContent.end());
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("attr2"),
                                     StorageObjectType::Attribute))
            != groupContent.end());
  }

  SECTION("root group")
  {
    hdf5io.createGroup("/rootGroup1");
    hdf5io.createGroup("/rootGroup2");

    auto groupContent = hdf5io.getStorageObjects("/");
    REQUIRE(groupContent.size() == 2);
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("rootGroup1"),
                                     StorageObjectType::Group))
            != groupContent.end());
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("rootGroup2"),
                                     StorageObjectType::Group))
            != groupContent.end());
  }

  SECTION("filter by object type")
  {
    hdf5io.createGroup("/filterGroup");
    hdf5io.createGroup("/filterGroup/subgroup1");
    IO::ArrayDataSetConfig config3 {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    hdf5io.createArrayDataSet(config3, "/filterGroup/dataset1");

    // Add attributes to the group
    int attrData = 44;
    hdf5io.createAttribute(
        BaseDataType::I32, &attrData, "/filterGroup", "attr1");

    auto groupContent =
        hdf5io.getStorageObjects("/filterGroup", StorageObjectType::Group);
    REQUIRE(groupContent.size() == 1);
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("subgroup1"),
                                     StorageObjectType::Group))
            != groupContent.end());

    groupContent =
        hdf5io.getStorageObjects("/filterGroup", StorageObjectType::Dataset);
    REQUIRE(groupContent.size() == 1);
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("dataset1"),
                                     StorageObjectType::Dataset))
            != groupContent.end());

    groupContent =
        hdf5io.getStorageObjects("/filterGroup", StorageObjectType::Attribute);
    REQUIRE(groupContent.size() == 1);
    REQUIRE(std::find(groupContent.begin(),
                      groupContent.end(),
                      std::make_pair(std::string("attr1"),
                                     StorageObjectType::Attribute))
            != groupContent.end());
  }

  // close file
  hdf5io.close();
}
// END TEST_CASE("getStorageObjects", "[hdf5io]")

TEST_CASE("HDF5IO; write datasets", "[hdf5io]")
{
  std::vector<int> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  SECTION("write 1D data block to 1D dataset")
  {
    // open file
    std::string path = getTestFilePath("test_write1DData1DDataset.h5");
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // Set up test data
    std::string dataPath = "/1DData1DDataset";
    SizeType numSamples = 10;

    // Create HDF5RecordingData object and dataset
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);

    // Write data block
    std::vector<SizeType> dataShape = {numSamples};
    std::vector<SizeType> positionOffset = {0};
    Status writeStatus = dataset->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, &testData[0]);
    REQUIRE(writeStatus == Status::Success);

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

    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {numRows, numCols}, SizeArray {0, 0}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);
    Status writeStatus = dataset->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, testData.data());
    REQUIRE(writeStatus == Status::Success);

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
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {numRows, numCols}, SizeArray {0, 0}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);

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

    IO::ArrayDataSetConfig config {BaseDataType::I32,
                                   SizeArray {depth, height, width},
                                   SizeArray {0, 0, 0}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);
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

    IO::ArrayDataSetConfig config {BaseDataType::I32,
                                   SizeArray {depth, height, width},
                                   SizeArray {0, 0, 0}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);
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

TEST_CASE("HDF5IO createArrayDataSet with filters", "[HDF5IO]")
{
  const std::string fileName = getTestFilePath("test.h5");

  // Define the data type, shape, and chunking
  IO::BaseDataType type(IO::BaseDataType::Type::T_I32, 1);
  SizeArray shape = {10, 10};  // Reduced size to avoid excessive data
  SizeArray chunking = {5, 5};

  // Create the HDF5IO object and open the file
  std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
      std::make_unique<IO::HDF5::HDF5IO>(fileName);
  hdf5io->open();

  SECTION("Create dataset with GZIP filter")
  {
    // Create HDF5ArrayDataSetConfig and add GZIP filter
    IO::HDF5::HDF5ArrayDataSetConfig config(type, shape, chunking);
    std::vector<unsigned int> gzip_level = {4};
    config.addFilter(H5Z_FILTER_DEFLATE, gzip_level);

    // Create the dataset
    auto baseDataset = hdf5io->createArrayDataSet(config, "/gzip_dataset");

    REQUIRE(baseDataset != nullptr);

    // Cast to HDF5RecordingData to access getDataSet()
    auto dataset =
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(baseDataset.get());
    REQUIRE(dataset != nullptr);

    // Verify the dataset properties
    const H5::DataSet* h5Dataset = dataset->getDataSet();
    H5::DSetCreatPropList dcpl = h5Dataset->getCreatePlist();
    REQUIRE(dcpl.getNfilters() == 1);

    unsigned int flags;
    size_t cd_nelmts = 1;
    unsigned int cd_values[1];
    char name[256];
    size_t namelen = 256;
    unsigned int filter_config;
    H5Z_filter_t filter_type = dcpl.getFilter(
        0, flags, cd_nelmts, cd_values, namelen, name, filter_config);
    REQUIRE(filter_type == H5Z_FILTER_DEFLATE);
    REQUIRE(cd_nelmts == 1);
    REQUIRE(cd_values[0] == gzip_level[0]);
  }

  SECTION("Create dataset with shuffle filter")
  {
    // Create HDF5ArrayDataSetConfig and add shuffle filter
    IO::HDF5::HDF5ArrayDataSetConfig config(type, shape, chunking);
    config.addFilter(H5Z_FILTER_SHUFFLE, {});

    // Create the dataset
    auto baseDataset = hdf5io->createArrayDataSet(config, "/shuffle_dataset");

    REQUIRE(baseDataset != nullptr);

    // Cast to HDF5RecordingData to access getDataSet()
    auto dataset =
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(baseDataset.get());
    REQUIRE(dataset != nullptr);

    // Verify the dataset properties
    const H5::DataSet* h5Dataset = dataset->getDataSet();
    H5::DSetCreatPropList dcpl = h5Dataset->getCreatePlist();
    REQUIRE(dcpl.getNfilters() == 1);

    unsigned int flags;
    size_t cd_nelmts = 1;
    unsigned int cd_values[1];
    char name[256];
    size_t namelen = 256;
    unsigned int filter_config;
    H5Z_filter_t filter_type = dcpl.getFilter(
        0, flags, cd_nelmts, cd_values, namelen, name, filter_config);
    REQUIRE(filter_type == H5Z_FILTER_SHUFFLE);
    REQUIRE(cd_nelmts == 1);
  }

  SECTION("Create dataset with multiple filters")
  {
    // Create HDF5ArrayDataSetConfig and add multiple filters
    IO::HDF5::HDF5ArrayDataSetConfig config(type, shape, chunking);
    std::vector<unsigned int> gzip_level = {4};
    config.addFilter(H5Z_FILTER_DEFLATE, gzip_level);
    config.addFilter(H5Z_FILTER_SHUFFLE, {});

    // Create the dataset
    auto baseDataset =
        hdf5io->createArrayDataSet(config, "/multiple_filters_dataset");

    REQUIRE(baseDataset != nullptr);

    // Cast to HDF5RecordingData to access getDataSet()
    auto dataset =
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(baseDataset.get());
    REQUIRE(dataset != nullptr);

    // Verify the dataset properties
    const H5::DataSet* h5Dataset = dataset->getDataSet();
    H5::DSetCreatPropList dcpl = h5Dataset->getCreatePlist();
    REQUIRE(dcpl.getNfilters() == 2);

    unsigned int flags;
    size_t cd_nelmts = 1;
    unsigned int cd_values[1];
    char name[256];
    size_t namelen = 256;
    unsigned int filter_config;
    H5Z_filter_t filter_type = dcpl.getFilter(
        0, flags, cd_nelmts, cd_values, namelen, name, filter_config);
    REQUIRE(filter_type == H5Z_FILTER_DEFLATE);
    REQUIRE(cd_nelmts == 1);
    REQUIRE(cd_values[0] == gzip_level[0]);

    cd_nelmts = 1;
    filter_type = dcpl.getFilter(
        1, flags, cd_nelmts, cd_values, namelen, name, filter_config);
    REQUIRE(filter_type == H5Z_FILTER_SHUFFLE);
    REQUIRE(cd_nelmts == 1);
  }

  SECTION("Create dataset without filters")
  {
    // Create HDF5ArrayDataSetConfig without filters
    IO::HDF5::HDF5ArrayDataSetConfig config(type, shape, chunking);

    // Create the dataset
    auto baseDataset =
        hdf5io->createArrayDataSet(config, "/no_filters_dataset");

    REQUIRE(baseDataset != nullptr);

    // Cast to HDF5RecordingData to access getDataSet()
    auto dataset =
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(baseDataset.get());
    REQUIRE(dataset != nullptr);

    // Verify the dataset properties
    const H5::DataSet* h5Dataset = dataset->getDataSet();
    H5::DSetCreatPropList dcpl = h5Dataset->getCreatePlist();
    REQUIRE(dcpl.getNfilters() == 0);
  }
}

TEST_CASE("HDF5IO; create attributes", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_attributes.h5");
  IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  const std::string groupPath = "/data";
  hdf5io.createGroup(groupPath);

  // scalar signed int attribute
  SECTION("single_value")
  {
    const signed int data = 1;
    const std::string attrName = "single_value";
    const std::string attrPath = mergePaths(groupPath, attrName);
    hdf5io.createAttribute(BaseDataType::I32, &data, groupPath, attrName);
    REQUIRE(hdf5io.attributeExists(attrPath));

    // Read the attribute and verify the attribute data
    auto readAttrGeneric = hdf5io.readAttribute(attrPath);
    auto readAttrData = IO::DataBlock<int>::fromGeneric(readAttrGeneric);
    REQUIRE(readAttrData.shape.size() == 0);  // Scalar attribute
    REQUIRE(readAttrData.data.size() == 1);
    REQUIRE(readAttrData.data[0] == data);
  }

  // scalar string attribute
  SECTION("single_str_value")
  {
    const std::string data = "scalar string";
    const std::string attrName = "scalar_string_value";
    const std::string attrPath = mergePaths(groupPath, attrName);
    hdf5io.createAttribute(data, groupPath, attrName);
    REQUIRE(hdf5io.attributeExists(attrPath));

    // Read the attribute and verify the attribute data
    auto readAttrGeneric = hdf5io.readAttribute(attrPath);
    auto readAttrData =
        IO::DataBlock<std::string>::fromGeneric(readAttrGeneric);
    REQUIRE(readAttrData.shape.size() == 0);  // Scalar attribute
    REQUIRE(readAttrData.data.size() == 1);
    REQUIRE(readAttrData.data[0] == data);
  }

  // integer array
  SECTION("int_array")
  {
    const std::vector<int> data = {1, 2, 3, 4, 5};
    const std::string attrName = "int_array";
    const std::string attrPath = mergePaths(groupPath, attrName);

    hdf5io.createAttribute(
        BaseDataType::I32, data.data(), groupPath, attrName, data.size());
    REQUIRE(hdf5io.attributeExists(attrPath));

    // Read the attribute and verify the attribute data
    auto readAttrGeneric = hdf5io.readAttribute(attrPath);
    auto readAttrData = IO::DataBlock<int>::fromGeneric(readAttrGeneric);
    REQUIRE(readAttrData.shape.size() == 1);  // Scalar attribute
    REQUIRE(readAttrData.data.size() == 5);
    REQUIRE(readAttrData.data == data);
  }

  // string array with a single value
  SECTION("str_array")
  {
    const std::vector<std::string> data = {"col1"};
    const std::string attrName = "string_array_with_one_value";
    const std::string attrPath = mergePaths(groupPath, attrName);

    hdf5io.createAttribute(data, groupPath, attrName);
    REQUIRE(hdf5io.attributeExists(attrPath));

    // Read the attribute and verify the attribute data
    auto readAttrGeneric = hdf5io.readAttribute(attrPath);
    auto readAttrData =
        IO::DataBlock<std::string>::fromGeneric(readAttrGeneric);
    REQUIRE(readAttrData.shape.size() == 1);
    REQUIRE(readAttrData.data.size() == 1);
    REQUIRE(readAttrData.data == data);
  }

  // string array
  SECTION("str_array")
  {
    const std::vector<std::string> data = {"col1", "col2", "col3"};
    const std::string attrName = "string_array";
    const std::string attrPath = mergePaths(groupPath, attrName);

    hdf5io.createAttribute(data, groupPath, attrName);
    REQUIRE(hdf5io.attributeExists(attrPath));

    // Read the attribute and verify the attribute data
    auto readAttrGeneric = hdf5io.readAttribute(attrPath);
    auto readAttrData =
        IO::DataBlock<std::string>::fromGeneric(readAttrGeneric);
    REQUIRE(readAttrData.shape.size() == 1);
    REQUIRE(readAttrData.data.size() == 3);
    REQUIRE(readAttrData.data == data);
  }

  // string array with overwrite
  SECTION("str_array_overwrite")
  {
    const std::vector<std::string> initial_data = {"initial1", "initial2"};
    const std::vector<std::string> new_data = {"new1", "new2", "new3"};
    const std::string attrName = "overwrite_string_array";
    const std::string attrPath = mergePaths(groupPath, attrName);

    // Create initial attribute
    REQUIRE(hdf5io.createAttribute(initial_data, groupPath, attrName)
            == Status::Success);
    REQUIRE(hdf5io.attributeExists(attrPath));

    // Read the attribute and verify the attribute data
    auto readAttrGeneric = hdf5io.readAttribute(attrPath);
    auto readAttrData =
        IO::DataBlock<std::string>::fromGeneric(readAttrGeneric);
    REQUIRE(readAttrData.shape.size() == 1);
    REQUIRE(readAttrData.data.size() == 2);
    REQUIRE(readAttrData.data == initial_data);

    // Attempt to create the same attribute without overwrite (should fail)
    REQUIRE(hdf5io.createAttribute(new_data, groupPath, attrName, false)
            == Status::Failure);

    // Overwrite the attribute
    REQUIRE(hdf5io.createAttribute(new_data, groupPath, attrName, true)
            == Status::Success);
    REQUIRE(hdf5io.attributeExists(attrPath));

    // Read the attribute and verify the attribute data
    auto readAttrGenericNew = hdf5io.readAttribute(attrPath);
    auto readAttrDataNew =
        IO::DataBlock<std::string>::fromGeneric(readAttrGenericNew);
    REQUIRE(readAttrDataNew.shape.size() == 1);
    REQUIRE(readAttrDataNew.data.size() == 3);
    REQUIRE(readAttrDataNew.data == new_data);
  }

  // soft link
  SECTION("link")
  {
    std::vector<std::string> data;
    hdf5io.createLink("/data/link", "linked_data");
    REQUIRE(hdf5io.objectExists("/data/link"));
  }

  // reference attribute tests
  SECTION("reference")
  {
    // Create target objects that we'll reference
    hdf5io.createGroup("/referenceTargetGroup");
    auto referenceConfig =
        IO::ArrayDataSetConfig(BaseDataType::I32, SizeArray {3}, SizeArray {3});
    hdf5io.createArrayDataSet(referenceConfig, "/referenceTargetDataset");

    // Test reference to a group
    SECTION("reference to group")
    {
      hdf5io.createReferenceAttribute(
          "/referenceTargetGroup", "/data", "groupRefAttr");
      REQUIRE(hdf5io.attributeExists("/data/groupRefAttr"));

      std::string resolvedPath =
          hdf5io.readReferenceAttribute("/data/groupRefAttr");
      REQUIRE(resolvedPath == "/referenceTargetGroup");
    }

    // Test reference to a dataset
    SECTION("reference to dataset")
    {
      hdf5io.createReferenceAttribute(
          "/referenceTargetDataset", "/data", "datasetRefAttr");
      REQUIRE(hdf5io.attributeExists("/data/datasetRefAttr"));

      std::string resolvedPath =
          hdf5io.readReferenceAttribute("/data/datasetRefAttr");
      REQUIRE(resolvedPath == "/referenceTargetDataset");
    }

    // Test reading non-existent reference attribute
    SECTION("non-existent reference attribute")
    {
      REQUIRE_THROWS_AS(
          hdf5io.readReferenceAttribute("/data/nonExistentRefAttr"),
          std::invalid_argument);
    }

    // Test reading an attribute that is not a reference attribute
    SECTION("non-reference attribute")
    {
      const int data = 1;
      hdf5io.createAttribute(BaseDataType::I32, &data, "/data", "intAttr");
      REQUIRE_THROWS_AS(hdf5io.readReferenceAttribute("/data/intAttr"),
                        std::invalid_argument);
    }
  }

  // close file
  hdf5io.close();
}

TEST_CASE("HDF5IO; SWMR mode", "[hdf5io]")
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
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);

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
    IO::ArrayDataSetConfig datasetConfig {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(datasetConfig, dataPath);

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
    IO::ArrayDataSetConfig configPostRestart {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    std::unique_ptr<BaseRecordingData> datasetPostRestart =
        hdf5io->createArrayDataSet(configPostRestart, dataPathPostRestart);

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
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    hdf5io.createArrayDataSet(config, dataPath);
    REQUIRE(hdf5io.getH5ObjectType(dataPath) == H5O_TYPE_DATASET);
  }

  SECTION("non_existent_object")
  {
    REQUIRE(hdf5io.getH5ObjectType("/non_existent") == H5O_TYPE_UNKNOWN);
  }

  // close file
  hdf5io.close();
}

TEST_CASE("getNativeType", "[hdf5io]")
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

TEST_CASE("getH5Type", "[hdf5io]")
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

TEST_CASE("objectExists", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_objectExists.h5");
  IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  SECTION("existing group")
  {
    hdf5io.createGroup("/existingGroup");
    REQUIRE(hdf5io.objectExists("/existingGroup") == true);
  }

  SECTION("non-existing group")
  {
    REQUIRE(hdf5io.objectExists("/nonExistingGroup") == false);
  }

  SECTION("existing dataset")
  {
    std::vector<int> testData = {1, 2, 3, 4, 5};
    std::string dataPath = "/existingDataset";
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    hdf5io.createArrayDataSet(config, dataPath);
    REQUIRE(hdf5io.objectExists(dataPath) == true);
  }

  SECTION("non-existing dataset")
  {
    REQUIRE(hdf5io.objectExists("/nonExistingDataset") == false);
  }

  SECTION("existing attribute")
  {
    hdf5io.createGroup("/groupWithAttribute");
    const int data = 1;
    hdf5io.createAttribute(
        BaseDataType::I32, &data, "/groupWithAttribute", "existingAttribute");
    REQUIRE(hdf5io.attributeExists("/groupWithAttribute/existingAttribute")
            == true);
  }

  SECTION("non-existing attribute")
  {
    REQUIRE(hdf5io.attributeExists("/groupWithAttribute/nonExistingAttribute")
            == false);
  }

  // close file
  hdf5io.close();
}

TEST_CASE("HDF5IOI::attributeExists", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_attributeExists.h5");
  IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  hdf5io.createGroup("/data");

  SECTION("existing attribute")
  {
    const int data = 1;
    hdf5io.createAttribute(
        BaseDataType::I32, &data, "/data", "existingAttribute");
    REQUIRE(hdf5io.attributeExists("/data/existingAttribute") == true);
  }

  SECTION("non-existing attribute")
  {
    REQUIRE(hdf5io.attributeExists("/data/nonExistingAttribute") == false);
  }

  SECTION("existing string attribute")
  {
    const std::string data = "test_string";
    hdf5io.createAttribute(data, "/data", "existingStringAttribute");
    REQUIRE(hdf5io.attributeExists("/data/existingStringAttribute") == true);
  }

  SECTION("existing string array attribute")
  {
    const std::vector<std::string> data = {"str1", "str2", "str3"};
    hdf5io.createAttribute(data, "/data", "existingStringArrayAttribute");
    REQUIRE(hdf5io.attributeExists("/data/existingStringArrayAttribute")
            == true);
  }

  SECTION("existing reference attribute")
  {
    hdf5io.createGroup("/referenceTarget");
    hdf5io.createReferenceAttribute(
        "/referenceTarget", "/data", "existingReferenceAttribute");
    REQUIRE(hdf5io.attributeExists("/data/existingReferenceAttribute") == true);
  }

  // close file
  hdf5io.close();
}

TEST_CASE("getStorageObjectType", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_getStorageObjectType.h5");
  IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  SECTION("group")
  {
    hdf5io.createGroup("/testGroup");
    REQUIRE(hdf5io.getStorageObjectType("/testGroup")
            == StorageObjectType::Group);
  }

  SECTION("dataset")
  {
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    hdf5io.createArrayDataSet(config, "/testDataset");
    REQUIRE(hdf5io.getStorageObjectType("/testDataset")
            == StorageObjectType::Dataset);
  }

  SECTION("attribute")
  {
    hdf5io.createGroup("/groupWithAttribute");
    const int data = 1;
    hdf5io.createAttribute(
        BaseDataType::I32, &data, "/groupWithAttribute", "testAttribute");
    REQUIRE(hdf5io.getStorageObjectType("/groupWithAttribute/testAttribute")
            == StorageObjectType::Attribute);
  }

  SECTION("non-existing object")
  {
    REQUIRE(hdf5io.getStorageObjectType("/nonExistingObject")
            == StorageObjectType::Undefined);
  }

  SECTION("link to group")
  {
    hdf5io.createGroup("/originalGroup");
    hdf5io.createLink("/linkToGroup", "/originalGroup");
    REQUIRE(hdf5io.getStorageObjectType("/linkToGroup")
            == StorageObjectType::Group);
  }

  SECTION("link to dataset")
  {
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    hdf5io.createArrayDataSet(config, "/originalDataset");
    hdf5io.createLink("/linkToDataset", "/originalDataset");
    REQUIRE(hdf5io.getStorageObjectType("/linkToDataset")
            == StorageObjectType::Dataset);
  }

  // close file
  hdf5io.close();
}

TEST_CASE("readAttribute", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_readAttribute.h5");
  IO::HDF5::HDF5IO hdf5io(filename);
  hdf5io.open();

  hdf5io.createGroup("/data");

  SECTION("read integer attribute")
  {
    const int32_t writeData = 42;
    hdf5io.createAttribute(
        BaseDataType::I32, &writeData, "/data", "intAttribute");
    auto readDataGeneric = hdf5io.readAttribute("/data/intAttribute");
    auto readData = IO::DataBlock<int32_t>::fromGeneric(readDataGeneric);

    REQUIRE(readData.shape.empty());  // Scalar attribute
    REQUIRE(readData.data.size() == 1);
    REQUIRE(readData.data[0] == writeData);
  }

  SECTION("read float attribute")
  {
    const float writeData = 3.14f;
    hdf5io.createAttribute(
        BaseDataType::F32, &writeData, "/data", "floatAttribute");
    auto readDataGeneric = hdf5io.readAttribute("/data/floatAttribute");
    auto readData = IO::DataBlock<float>::fromGeneric(readDataGeneric);

    REQUIRE(readData.shape.empty());  // Scalar attribute
    REQUIRE(readData.data.size() == 1);
    REQUIRE(readData.data[0] == Catch::Approx(writeData).epsilon(0.001));
  }

  SECTION("read string attribute")
  {
    const std::string writeData = "test_string";
    hdf5io.createAttribute(writeData, "/data", "stringAttribute");
    auto readDataGeneric = hdf5io.readAttribute("/data/stringAttribute");
    auto readData = IO::DataBlock<std::string>::fromGeneric(readDataGeneric);

    REQUIRE(readData.shape.empty());  // Scalar attribute
    REQUIRE(readData.data.size() == 1);
    REQUIRE(readData.data[0] == writeData);
  }

  SECTION("read integer array attribute")
  {
    const int32_t writeData[] = {1, 2, 3, 4, 5};
    const int dataSize = sizeof(writeData) / sizeof(writeData[0]);
    hdf5io.createAttribute(
        BaseDataType::I32, writeData, "/data", "intArrayAttribute", dataSize);
    auto readDataGeneric = hdf5io.readAttribute("/data/intArrayAttribute");
    auto readData = IO::DataBlock<int32_t>::fromGeneric(readDataGeneric);

    REQUIRE(readData.shape.size() == 1);  // 1D array attribute
    REQUIRE(readData.shape[0] == dataSize);
    REQUIRE(readData.data.size() == dataSize);
    for (SizeType i = 0; i < dataSize; ++i) {
      REQUIRE(readData.data[i] == writeData[i]);
    }
  }

  SECTION("read double attribute")
  {
    const double writeData = 2.718;
    hdf5io.createAttribute(
        BaseDataType::F64, &writeData, "/data", "doubleAttribute");
    auto readDataGeneric = hdf5io.readAttribute("/data/doubleAttribute");
    auto readData = IO::DataBlock<double>::fromGeneric(readDataGeneric);

    REQUIRE(readData.shape.empty());  // Scalar attribute
    REQUIRE(readData.data.size() == 1);
    REQUIRE(readData.data[0] == Catch::Approx(writeData).epsilon(0.001));
  }

  SECTION("read string array attribute")
  {
    const std::vector<std::string> writeData = {"str1", "str2", "str3"};
    hdf5io.createAttribute(writeData, "/data", "stringArrayAttribute");
    auto readDataGeneric = hdf5io.readAttribute("/data/stringArrayAttribute");
    auto readData = IO::DataBlock<std::string>::fromGeneric(readDataGeneric);

    REQUIRE(readData.shape.size() == 1);  // 1D array attribute
    REQUIRE(readData.shape[0] == writeData.size());
    REQUIRE(readData.data.size() == writeData.size());
    for (size_t i = 0; i < writeData.size(); ++i) {
      REQUIRE(readData.data[i] == writeData[i]);
    }
  }

  SECTION("read reference attribute")
  {
    // Create a target group to reference
    hdf5io.createGroup("/referenceTarget");

    // Create a reference attribute pointing to the target group
    hdf5io.createReferenceAttribute(
        "/referenceTarget", "/data", "referenceAttribute");

    // Read the reference attribute
    auto readDataGeneric = hdf5io.readAttribute("/data/referenceAttribute");
    auto readData = IO::DataBlock<hobj_ref_t>::fromGeneric(readDataGeneric);

    REQUIRE(readData.shape.empty());  // Scalar attribute
    REQUIRE(readData.data.size() == 1);

    // Verify the reference points to the correct target group
    H5::H5File file(hdf5io.getFileName(), H5F_ACC_RDONLY);
    hid_t file_id = file.getId();
    hid_t ref_group_id =
        H5Rdereference2(file_id, H5P_DEFAULT, H5R_OBJECT, &readData.data[0]);

    // Check if the dereferenced ID is valid
    REQUIRE(ref_group_id >= 0);

    H5::Group refGroup(ref_group_id);
    H5::Group targetGroup(file.openGroup("/referenceTarget"));

    // Compare the paths to ensure the reference points to the correct group
    std::string refPath = refGroup.getObjName();
    std::string targetPath = targetGroup.getObjName();
    REQUIRE(refPath == targetPath);

    // Close the dereferenced group ID
    H5Gclose(ref_group_id);
  }

  SECTION("read attribute from dataset")
  {
    // Define the dimensions and chunking for the dataset
    SizeArray dims = {10};
    SizeArray chunking = {10};

    // Create the dataset using createArrayDataSet
    IO::ArrayDataSetConfig config {BaseDataType::I32, dims, chunking};
    auto dataset = hdf5io.createArrayDataSet(config, "/data/dataset");

    // Define and create the attribute
    const int32_t writeData = 123;
    hdf5io.createAttribute(
        BaseDataType::I32, &writeData, "/data/dataset", "datasetAttribute");

    // Read the attribute
    auto readDataGeneric =
        hdf5io.readAttribute("/data/dataset/datasetAttribute");
    auto readData = IO::DataBlock<int32_t>::fromGeneric(readDataGeneric);

    // Verify the attribute data
    REQUIRE(readData.shape.empty());  // Scalar attribute
    REQUIRE(readData.data.size() == 1);
    REQUIRE(readData.data[0] == writeData);
  }

  SECTION("read non-existent attribute")
  {
    REQUIRE_THROWS_AS(hdf5io.readAttribute("/data/nonExistentAttribute"),
                      std::invalid_argument);
  }

  SECTION("read attribute from invalid path")
  {
    REQUIRE_THROWS_AS(hdf5io.readAttribute("/invalidPath/attribute"),
                      std::invalid_argument);
  }

  // close file
  hdf5io.close();
}

TEST_CASE("HDF5IO; read dataset", "[hdf5io]")
{
  std::vector<int32_t> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  SECTION("read 1D data block of a 1D dataset")
  {
    // open file
    std::string path = getTestFilePath("test_Read1DData1DDataset.h5");
    std::shared_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // Set up test data
    std::string dataPath = "/1DData1DDataset";
    SizeType numSamples = 10;

    // Create HDF5RecordingData object and dataset
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {0}, SizeArray {1}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);

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

  SECTION("read 2D dataset")
  {
    // open file
    std::string path = getTestFilePath("test_Read2DData2DDataset.h5");
    std::shared_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // Set up test data
    std::string dataPath = "/2DData2DDataset";
    SizeType numRows = 2, numCols = 5;
    std::vector<int32_t> testData2D = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Create HDF5RecordingData object and dataset
    IO::ArrayDataSetConfig config {
        BaseDataType::I32, SizeArray {numRows, numCols}, SizeArray {0, 0}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);

    // Write data block
    std::vector<SizeType> dataShape = {numRows, numCols};
    std::vector<SizeType> positionOffset = {0, 0};
    dataset->writeDataBlock(
        dataShape, positionOffset, BaseDataType::I32, testData2D.data());

    // Confirm using HDF5IO readDataset that the data is correct
    auto readData = hdf5io->readDataset(dataPath);
    REQUIRE(readData.shape[0] == numRows);
    REQUIRE(readData.shape[1] == numCols);
    auto readDataTyped = DataBlock<int32_t>::fromGeneric(readData);
    REQUIRE(readDataTyped.shape[0] == numRows);
    REQUIRE(readDataTyped.shape[1] == numCols);
    REQUIRE(readDataTyped.data == testData2D);

    hdf5io->close();
  }

  SECTION("read dataset with different data types")
  {
    // open file
    std::string path = getTestFilePath("test_ReadDifferentDataTypes.h5");
    std::shared_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();
    Status writeStatus;

    // Set up test data for float
    std::string floatDataPath = "/FloatDataset";
    std::vector<float> testDataFloat = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};

    // Create HDF5RecordingData object and dataset for float
    IO::ArrayDataSetConfig floatConfig {
        BaseDataType::F32, SizeArray {5}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> floatDataset =
        hdf5io->createArrayDataSet(floatConfig, floatDataPath);

    // Write float data block
    std::vector<SizeType> floatDataShape = {5};
    std::vector<SizeType> floatPositionOffset = {0};
    writeStatus = floatDataset->writeDataBlock(floatDataShape,
                                               floatPositionOffset,
                                               BaseDataType::F32,
                                               testDataFloat.data());
    REQUIRE(writeStatus == Status::Success);

    // Confirm using HDF5IO readDataset that the float data is correct
    auto readFloatData = hdf5io->readDataset(floatDataPath);
    REQUIRE(readFloatData.shape[0] == 5);
    auto readFloatDataTyped = DataBlock<float>::fromGeneric(readFloatData);
    REQUIRE(readFloatDataTyped.shape[0] == 5);
    REQUIRE(readFloatDataTyped.data == testDataFloat);

    // Set up test data for double
    std::string doubleDataPath = "/DoubleDataset";
    std::vector<double> testDataDouble = {1.1, 2.2, 3.3, 4.4, 5.5};

    // Create HDF5RecordingData object and dataset for double
    IO::ArrayDataSetConfig doubleConfig {
        BaseDataType::F64, SizeArray {5}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> doubleDataset =
        hdf5io->createArrayDataSet(doubleConfig, doubleDataPath);

    // Write double data block
    std::vector<SizeType> doubleDataShape = {5};
    std::vector<SizeType> doublePositionOffset = {0};
    writeStatus = doubleDataset->writeDataBlock(doubleDataShape,
                                                doublePositionOffset,
                                                BaseDataType::F64,
                                                testDataDouble.data());
    REQUIRE(writeStatus == Status::Success);

    // Confirm using HDF5IO readDataset that the double data is correct
    auto readDoubleData = hdf5io->readDataset(doubleDataPath);
    REQUIRE(readDoubleData.shape[0] == 5);
    auto readDoubleDataTyped = DataBlock<double>::fromGeneric(readDoubleData);
    REQUIRE(readDoubleDataTyped.shape[0] == 5);
    REQUIRE(readDoubleDataTyped.data == testDataDouble);

    // Set up test data for unsigned 8-bit integer
    std::string u8DataPath = "/U8Dataset";
    std::vector<uint8_t> testDataU8 = {1, 2, 3, 4, 5};

    // Create HDF5RecordingData object and dataset for unsigned 8-bit integer
    IO::ArrayDataSetConfig u8Config {
        BaseDataType::T_U8, SizeArray {5}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> u8Dataset =
        hdf5io->createArrayDataSet(u8Config, u8DataPath);

    // Write unsigned 8-bit integer data block
    std::vector<SizeType> u8DataShape = {5};
    std::vector<SizeType> u8PositionOffset = {0};
    writeStatus = u8Dataset->writeDataBlock(
        u8DataShape, u8PositionOffset, BaseDataType::T_U8, testDataU8.data());
    REQUIRE(writeStatus == Status::Success);

    // Confirm using HDF5IO readDataset that the unsigned 8-bit integer data is
    // correct
    auto readU8Data = hdf5io->readDataset(u8DataPath);
    REQUIRE(readU8Data.shape[0] == 5);
    auto readU8DataTyped = DataBlock<uint8_t>::fromGeneric(readU8Data);
    REQUIRE(readU8DataTyped.shape[0] == 5);
    REQUIRE(readU8DataTyped.data == testDataU8);

    // Set up test data for unsigned 16-bit integer
    std::string u16DataPath = "/U16Dataset";
    std::vector<uint16_t> testDataU16 = {1, 2, 3, 4, 5};

    // Create HDF5RecordingData object and dataset for unsigned 16-bit integer
    IO::ArrayDataSetConfig u16Config {
        BaseDataType::T_U16, SizeArray {5}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> u16Dataset =
        hdf5io->createArrayDataSet(u16Config, u16DataPath);

    // Write unsigned 16-bit integer data block
    std::vector<SizeType> u16DataShape = {5};
    std::vector<SizeType> u16PositionOffset = {0};
    writeStatus = u16Dataset->writeDataBlock(u16DataShape,
                                             u16PositionOffset,
                                             BaseDataType::T_U16,
                                             testDataU16.data());
    REQUIRE(writeStatus == Status::Success);

    // Confirm using HDF5IO readDataset that the unsigned 16-bit integer data is
    // correct
    auto readU16Data = hdf5io->readDataset(u16DataPath);
    REQUIRE(readU16Data.shape[0] == 5);
    auto readU16DataTyped = DataBlock<uint16_t>::fromGeneric(readU16Data);
    REQUIRE(readU16DataTyped.shape[0] == 5);
    REQUIRE(readU16DataTyped.data == testDataU16);

    // Set up test data for signed 8-bit integer
    std::string i8DataPath = "/I8Dataset";
    std::vector<int8_t> testDataI8 = {1, 2, 3, 4, 5};

    // Create HDF5RecordingData object and dataset for signed 8-bit integer
    IO::ArrayDataSetConfig i8Config {
        BaseDataType::T_I8, SizeArray {5}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> i8Dataset =
        hdf5io->createArrayDataSet(i8Config, i8DataPath);

    // Write signed 8-bit integer data block
    std::vector<SizeType> i8DataShape = {5};
    std::vector<SizeType> i8PositionOffset = {0};
    writeStatus = i8Dataset->writeDataBlock(
        i8DataShape, i8PositionOffset, BaseDataType::T_I8, testDataI8.data());
    REQUIRE(writeStatus == Status::Success);

    // Confirm using HDF5IO readDataset that the signed 8-bit integer data is
    // correct
    auto readI8Data = hdf5io->readDataset(i8DataPath);
    REQUIRE(readI8Data.shape[0] == 5);
    auto readI8DataTyped = DataBlock<int8_t>::fromGeneric(readI8Data);
    REQUIRE(readI8DataTyped.shape[0] == 5);
    REQUIRE(readI8DataTyped.data == testDataI8);

    // Set up test data for signed 16-bit integer
    std::string i16DataPath = "/I16Dataset";
    std::vector<int16_t> testDataI16 = {1, 2, 3, 4, 5};

    // Create HDF5RecordingData object and dataset for signed 16-bit integer
    IO::ArrayDataSetConfig i16Config {
        BaseDataType::T_I16, SizeArray {5}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> i16Dataset =
        hdf5io->createArrayDataSet(i16Config, i16DataPath);

    // Write signed 16-bit integer data block
    std::vector<SizeType> i16DataShape = {5};
    std::vector<SizeType> i16PositionOffset = {0};
    writeStatus = i16Dataset->writeDataBlock(i16DataShape,
                                             i16PositionOffset,
                                             BaseDataType::T_I16,
                                             testDataI16.data());
    REQUIRE(writeStatus == Status::Success);

    // Confirm using HDF5IO readDataset that the signed 16-bit integer data is
    // correct
    auto readI16Data = hdf5io->readDataset(i16DataPath);
    REQUIRE(readI16Data.shape[0] == 5);
    auto readI16DataTyped = DataBlock<int16_t>::fromGeneric(readI16Data);
    REQUIRE(readI16DataTyped.shape[0] == 5);
    REQUIRE(readI16DataTyped.data == testDataI16);

    // Set up test data for fixed-length string
    std::string strDataPath = "/StrDataset";
    std::vector<std::string> testDataStr = {"abc", "def", "ghi"};

    // Create HDF5RecordingData object and dataset for fixed-length string
    BaseDataType strType(BaseDataType::Type::T_STR,
                         3);  // Specify string length
    IO::ArrayDataSetConfig strConfig {strType, SizeArray {3}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> strDataset =
        hdf5io->createArrayDataSet(strConfig, strDataPath);

    // Write fixed-length string data block
    std::vector<SizeType> strDataShape = {3};
    std::vector<SizeType> strPositionOffset = {0};
    writeStatus = strDataset->writeDataBlock(
        strDataShape, strPositionOffset, strType, testDataStr);
    REQUIRE(writeStatus == Status::Success);

    // Confirm reading the fixed-length string data is correct
    auto readStrData = hdf5io->readDataset(strDataPath);
    REQUIRE(readStrData.shape[0] == 3);
    auto readStrDataTyped = DataBlock<std::string>::fromGeneric(readStrData);
    REQUIRE(readStrDataTyped.shape[0] == 3);
    REQUIRE(readStrDataTyped.data == testDataStr);

    // Test writing and reading of variable length strings as datasets
    std::string vstrDataPath = "/VStrDataset";
    std::vector<std::string> testDataVStr = {"jkl", "mnop", "qrstu"};

    // Initialize BaseDataType for variable-length strings
    BaseDataType vstrType(BaseDataType::Type::V_STR,
                          0);  // 0 indicates variable length

    // Create HDF5RecordingData object and dataset for variable-length string
    IO::ArrayDataSetConfig vstrConfig {vstrType, SizeArray {3}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> vstrDataset =
        hdf5io->createArrayDataSet(vstrConfig, vstrDataPath);

    // Write variable-length string data block
    std::vector<SizeType> vstrDataShape = {3};
    std::vector<SizeType> vstrPositionOffset = {0};
    writeStatus =
        vstrDataset->writeDataBlock(vstrDataShape,
                                    vstrPositionOffset,
                                    vstrType,  // Pass the vstrType object
                                    testDataVStr);
    REQUIRE(writeStatus == Status::Success);

    // Confirm reading the variable-length string data is correct
    auto readVStrData = hdf5io->readDataset(vstrDataPath);
    REQUIRE(readVStrData.shape[0] == 3);
    auto readVStrDataTyped = DataBlock<std::string>::fromGeneric(readVStrData);
    REQUIRE(readVStrDataTyped.shape[0] == 3);
    REQUIRE(readVStrDataTyped.data == testDataVStr);

    hdf5io->close();
  }

  SECTION("read empty string dataset")
  {
    // open file
    std::string path = getTestFilePath("test_ReadEmptyStringDataset.h5");
    std::shared_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // Set up test data for an unsupported type (e.g., string)
    std::string dataPath = "/EmptyStringdDataset";

    // Create HDF5RecordingData object and dataset for string
    BaseDataType vstrType(BaseDataType::Type::V_STR, 0);
    IO::ArrayDataSetConfig config {vstrType, SizeArray {3}, SizeArray {0}};
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, dataPath);

    // Attempt to read the dataset and verify that it returns empty data
    auto readVStrData = hdf5io->readDataset(dataPath);
    REQUIRE(readVStrData.shape[0] == 3);
    auto readVStrDataTyped = DataBlock<std::string>::fromGeneric(readVStrData);
    REQUIRE(readVStrDataTyped.data.size() == 3);
    REQUIRE(readVStrDataTyped.data[0].empty());
    REQUIRE(readVStrDataTyped.data[1].empty());
    REQUIRE(readVStrDataTyped.data[2].empty());

    hdf5io->close();
  }

  SECTION("read unsupported data type")
  {
    // open file
    std::string path = getTestFilePath("test_ReadUnsupportedDataType.h5");
    std::shared_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // Create a compound datatype
    H5::CompType compoundType(sizeof(double) * 2);
    compoundType.insertMember("real", 0, H5::PredType::NATIVE_DOUBLE);
    compoundType.insertMember(
        "imag", sizeof(double), H5::PredType::NATIVE_DOUBLE);

    // Create dataset with compound type directly using HDF5 C++ API
    H5::H5File file(path, H5F_ACC_RDWR);
    hsize_t dims[1] = {5};
    H5::DataSpace dataspace(1, dims);
    H5::DataSet dataset =
        file.createDataSet("ComplexData", compoundType, dataspace);

    // Attempt to read the dataset - should throw an exception
    REQUIRE_THROWS_AS(hdf5io->readDataset("/ComplexData"), std::runtime_error);

    hdf5io->close();
  }
}

TEST_CASE("HDF5IO; read dataset subset", "[hdf5io]")
{
  SECTION("read dataset with hyperslab selection")
  {
    // Set up test data
    std::string filePath = getTestFilePath("ReadHyperslabSelection.h5");
    std::string dataPath = "/2DDataHyperslab";
    std::vector<int32_t> testData2D;
    std::shared_ptr<IO::HDF5::HDF5IO> hdf5io =
        getHDF5IOWithInt32TestData2D(filePath, dataPath, testData2D);

    // Test case 1: Read a 2x2 hyperslab from the dataset
    {
      std::vector<SizeType> start = {1, 1};
      std::vector<SizeType> count = {2, 2};

      auto readData = hdf5io->readDataset(dataPath, start, count);
      auto readDataTyped = DataBlock<int32_t>::fromGeneric(readData);
      std::vector<int32_t> expectedData = {5, 6, 8, 9};

      // Check the shape of the read data
      REQUIRE(readData.shape.size() == 2);
      REQUIRE(readData.shape[0] == 2);
      REQUIRE(readData.shape[1] == 2);

      // Check the data itself
      REQUIRE(readDataTyped.data == expectedData);
    }

    // Test case 2: Read the entire dataset
    {
      std::vector<SizeType> start = {};
      std::vector<SizeType> count = {};

      auto readData = hdf5io->readDataset(dataPath, start, count);
      auto readDataTyped = DataBlock<int32_t>::fromGeneric(readData);
      std::vector<int32_t> expectedData = testData2D;

      // Check the shape of the read data
      REQUIRE(readData.shape.size() == 2);
      REQUIRE(readData.shape[0] == 3);
      REQUIRE(readData.shape[1] == 3);

      // Check the data itself
      REQUIRE(readDataTyped.data == expectedData);
    }

    // Test case 3: Read a single element from the dataset
    {
      std::vector<SizeType> start = {2, 2};
      std::vector<SizeType> count = {1, 1};

      auto readData = hdf5io->readDataset(dataPath, start, count);
      auto readDataTyped = DataBlock<int32_t>::fromGeneric(readData);
      std::vector<int32_t> expectedData = {9};

      // Check the shape of the read data
      REQUIRE(readData.shape.size() == 2);
      REQUIRE(readData.shape[0] == 1);
      REQUIRE(readData.shape[1] == 1);

      // Check the data itself
      REQUIRE(readDataTyped.data == expectedData);
    }

    // Test case 4: Read with stride
    {
      std::vector<SizeType> start = {0, 0};
      std::vector<SizeType> count = {2, 2};
      std::vector<SizeType> stride = {2, 2};

      auto readData = hdf5io->readDataset(dataPath, start, count, stride);
      auto readDataTyped = DataBlock<int32_t>::fromGeneric(readData);
      std::vector<int32_t> expectedData = {1, 3, 7, 9};

      // Check the shape of the read data
      REQUIRE(readData.shape.size() == 2);
      REQUIRE(readData.shape[0] == 2);
      REQUIRE(readData.shape[1] == 2);

      // Check the data itself
      REQUIRE(readDataTyped.data == expectedData);
    }

    // Test case 5: Read with block
    {
      std::vector<SizeType> start = {0, 0};
      std::vector<SizeType> count = {1, 1};
      std::vector<SizeType> block = {2, 2};

      auto readData = hdf5io->readDataset(dataPath, start, count, {}, block);
      auto readDataTyped = DataBlock<int32_t>::fromGeneric(readData);
      std::vector<int32_t> expectedData = {1, 2, 4, 5};

      // Check the shape of the read data
      REQUIRE(readData.shape.size() == 2);
      REQUIRE(readData.shape[0] == 2);
      REQUIRE(readData.shape[1] == 2);

      // Check the data itself
      REQUIRE(readDataTyped.data == expectedData);
    }

    hdf5io->close();
  }
}
