#include <catch2/catch_approx.hpp>

#include "io/hdf5/HDF5RecordingData.hpp"
#include "testUtils.hpp"

TEST_CASE("HDF5RecordingData basic operations", "[hdf5recordingdata]")
{
  SECTION("Constructor and initialization")
  {
    // Create a file and dataset first
    std::string path = getTestFilePath("test_HDF5RecordingData.h5");
    std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
        std::make_unique<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    // Create a dataset
    std::unique_ptr<BaseRecordingData> dataset = hdf5io->createArrayDataSet(
        BaseDataType::I32, SizeArray {10}, SizeArray {5}, "/testDataset");

    // Verify the dataset was created with correct dimensions
    REQUIRE(dataset->getNumDimensions() == 1);
    REQUIRE(dataset->getSize()[0] == 10);
    REQUIRE(dataset->getPosition()[0] == 0);

    hdf5io->close();
  }
}

TEST_CASE("HDF5RecordingData write operations", "[hdf5recordingdata]")
{
  std::string path = getTestFilePath("test_HDF5RecordingData_write.h5");
  std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
      std::make_unique<IO::HDF5::HDF5IO>(path);
  hdf5io->open();

  SECTION("Write numeric data types")
  {
    // Test int32
    {
      std::vector<int32_t> data = {1, 2, 3, 4, 5};
      auto dataset = hdf5io->createArrayDataSet(
          BaseDataType::I32, SizeArray {5}, SizeArray {5}, "/int32Dataset");
      Status status = dataset->writeDataBlock(std::vector<SizeType> {5},
                                              std::vector<SizeType> {0},
                                              BaseDataType::I32,
                                              data.data());
      REQUIRE(status == Status::Success);
    }

    // Test float
    {
      std::vector<float> data = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
      auto dataset = hdf5io->createArrayDataSet(
          BaseDataType::F32, SizeArray {5}, SizeArray {5}, "/floatDataset");
      Status status = dataset->writeDataBlock(std::vector<SizeType> {5},
                                              std::vector<SizeType> {0},
                                              BaseDataType::F32,
                                              data.data());
      REQUIRE(status == Status::Success);
    }
  }

  SECTION("Write string data")
  {
    // Test fixed-length strings
    {
      std::vector<std::string> data = {"abc", "def", "ghi"};
      BaseDataType strType(BaseDataType::Type::T_STR, 3);
      auto dataset = hdf5io->createArrayDataSet(
          strType, SizeArray {3}, SizeArray {3}, "/fixedStrDataset");
      Status status = dataset->writeDataBlock(
          std::vector<SizeType> {3}, std::vector<SizeType> {0}, strType, data);
      REQUIRE(status == Status::Success);
    }

    // Test variable-length strings
    {
      std::vector<std::string> data = {
          "longer", "strings", "of", "varying", "length"};
      BaseDataType strType(BaseDataType::Type::V_STR, 0);
      auto dataset = hdf5io->createArrayDataSet(
          strType, SizeArray {5}, SizeArray {5}, "/varStrDataset");
      Status status = dataset->writeDataBlock(
          std::vector<SizeType> {5}, std::vector<SizeType> {0}, strType, data);
      REQUIRE(status == Status::Success);
    }
  }

  SECTION("Error cases")
  {
    // Test dimension mismatch
    {
      std::vector<int32_t> data = {1, 2, 3, 4, 5};
      auto dataset = hdf5io->createArrayDataSet(
          BaseDataType::I32, SizeArray {5}, SizeArray {5}, "/errorDataset1");
      // Wrong number of dimensions in dataShape
      Status status = dataset->writeDataBlock(
          std::vector<SizeType> {5, 1},  // 2D shape for 1D dataset
          std::vector<SizeType> {0},
          BaseDataType::I32,
          data.data());
      REQUIRE(status == Status::Failure);
    }

    // Test wrong data type
    // TODO: This test is not possible because the data type is not checked
    //       when writing data. This is because the data type is not stored
    //       in the dataset object. It is only used when reading data.
    //       This should be fixed in the future by storing the data type
    //       in the BaseRecordingData object, which should allow us to
    //       remove the need to specify the data type when calling
    //       writeDataBlock and enforce types more strictly on write.
    /*
    {
        std::vector<int32_t> data = {1, 2, 3, 4, 5};
        auto dataset = hdf5io->createArrayDataSet(
            BaseDataType::F32,  // Float dataset
            SizeArray {5},
            SizeArray {5},
            "/errorDataset2"
        );
        // Try to write int32 data to float dataset
        Status status = dataset->writeDataBlock(
            std::vector<SizeType>{5},
            std::vector<SizeType>{0},
            BaseDataType::I32,  // Wrong type
            data.data()
        );
        REQUIRE(status == Status::Failure);
    }*/

    // Test writing string data with void* interface
    {
      std::vector<std::string> data = {"test"};
      auto dataset = hdf5io->createArrayDataSet(
          BaseDataType::V_STR, SizeArray {1}, SizeArray {1}, "/errorDataset3");
      // Try to write string data using void* interface
      Status status = dataset->writeDataBlock(std::vector<SizeType> {1},
                                              std::vector<SizeType> {0},
                                              BaseDataType::V_STR,
                                              data.data()  // This should fail
      );
      REQUIRE(status == Status::Failure);
    }

    // Test position offset out of bounds
    {
      std::vector<int32_t> data = {1, 2, 3, 4, 5};
      auto dataset = hdf5io->createArrayDataSet(
          BaseDataType::I32, SizeArray {5}, SizeArray {5}, "/errorDataset4");
      // Write at larger offset - should succeed by extending dataset
      Status status = dataset->writeDataBlock(
          std::vector<SizeType> {5},
          std::vector<SizeType> {10},  // Dataset will extend to accommodate
          BaseDataType::I32,
          data.data());
      REQUIRE(status == Status::Success);

      // Verify the dataset was extended
      REQUIRE(dataset->getSize()[0]
              == 15);  // Original offset (10) + data size (5)
    }
  }

  hdf5io->close();
}

TEST_CASE("HDF5RecordingData multi-dimensional operations",
          "[hdf5recordingdata]")
{
  std::string path = getTestFilePath("test_HDF5RecordingData_multidim.h5");
  std::unique_ptr<IO::HDF5::HDF5IO> hdf5io =
      std::make_unique<IO::HDF5::HDF5IO>(path);
  hdf5io->open();

  SECTION("2D operations")
  {
    std::vector<int32_t> data = {1, 2, 3, 4, 5, 6};
    auto dataset = hdf5io->createArrayDataSet(
        BaseDataType::I32, SizeArray {2, 3}, SizeArray {2, 3}, "/2dDataset");

    // Write full 2D block
    Status status = dataset->writeDataBlock(std::vector<SizeType> {2, 3},
                                            std::vector<SizeType> {0, 0},
                                            BaseDataType::I32,
                                            data.data());
    REQUIRE(status == Status::Success);

    // Write partial block
    std::vector<int32_t> partial = {7, 8};
    status = dataset->writeDataBlock(std::vector<SizeType> {1, 2},
                                     std::vector<SizeType> {1, 1},
                                     BaseDataType::I32,
                                     partial.data());
    REQUIRE(status == Status::Success);
  }

  SECTION("3D operations")
  {
    std::vector<int32_t> data(24);  // 2x3x4 array
    std::iota(data.begin(), data.end(), 1);  // Fill with 1,2,3,...,24

    auto dataset = hdf5io->createArrayDataSet(BaseDataType::I32,
                                              SizeArray {2, 3, 4},
                                              SizeArray {2, 3, 4},
                                              "/3dDataset");

    // Write full 3D block
    Status status = dataset->writeDataBlock(std::vector<SizeType> {2, 3, 4},
                                            std::vector<SizeType> {0, 0, 0},
                                            BaseDataType::I32,
                                            data.data());
    REQUIRE(status == Status::Success);

    // Write partial block
    std::vector<int32_t> partial = {100, 101, 102, 103};
    status = dataset->writeDataBlock(std::vector<SizeType> {1, 1, 4},
                                     std::vector<SizeType> {1, 1, 0},
                                     BaseDataType::I32,
                                     partial.data());
    REQUIRE(status == Status::Success);
  }

  hdf5io->close();
}
