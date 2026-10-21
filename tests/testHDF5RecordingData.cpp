#include <numeric>

#include <H5Cpp.h>
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
    IO::ArrayDataSetConfig config(
        BaseDataType::I32, SizeArray {10}, SizeArray {5});
    std::unique_ptr<BaseRecordingData> dataset =
        hdf5io->createArrayDataSet(config, "/testDataset");

    // Verify the dataset was created with correct dimensions
    REQUIRE(dataset->getNumDimensions() == 1);
    REQUIRE(dataset->getShape()[0] == 10);
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
      IO::ArrayDataSetConfig config(
          BaseDataType::I32, SizeArray {5}, SizeArray {5});
      auto dataset = hdf5io->createArrayDataSet(config, "/int32Dataset");
      Status status = dataset->writeDataBlock(
          SizeArray {5}, SizeArray {0}, BaseDataType::I32, data.data());
      REQUIRE(status == Status::Success);
    }

    // Test float
    {
      std::vector<float> data = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
      IO::ArrayDataSetConfig config(
          BaseDataType::F32, SizeArray {5}, SizeArray {5});
      auto dataset = hdf5io->createArrayDataSet(config, "/floatDataset");
      Status status = dataset->writeDataBlock(
          SizeArray {5}, SizeArray {0}, BaseDataType::F32, data.data());
      REQUIRE(status == Status::Success);
    }
  }

  SECTION("Write string data")
  {
    // Test fixed-length strings
    {
      std::vector<std::string> data = {"abc", "def", "ghi"};
      BaseDataType strType(BaseDataType::Type::T_STR, 3);
      IO::ArrayDataSetConfig config(strType, SizeArray {3}, SizeArray {3});
      auto dataset = hdf5io->createArrayDataSet(config, "/fixedStrDataset");
      Status status =
          dataset->writeDataBlock(SizeArray {3}, SizeArray {0}, strType, data);
      REQUIRE(status == Status::Success);
    }

    // Test variable-length strings
    {
      std::vector<std::string> data = {
          "longer", "strings", "of", "varying", "length"};
      BaseDataType strType(BaseDataType::Type::V_STR, 0);
      IO::ArrayDataSetConfig config(strType, SizeArray {5}, SizeArray {5});
      auto dataset = hdf5io->createArrayDataSet(config, "/varStrDataset");
      Status status =
          dataset->writeDataBlock(SizeArray {5}, SizeArray {0}, strType, data);
      REQUIRE(status == Status::Success);
    }
  }

  SECTION("Error cases")
  {
    // Test dimension mismatch
    {
      std::vector<int32_t> data = {1, 2, 3, 4, 5};
      IO::ArrayDataSetConfig config(
          BaseDataType::I32, SizeArray {5}, SizeArray {5});
      auto dataset = hdf5io->createArrayDataSet(config, "/errorDataset1");
      // Wrong number of dimensions in dataShape
      Status status =
          dataset->writeDataBlock(SizeArray {5, 1},  // 2D shape for 1D dataset
                                  SizeArray {0},
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
            SizeArray{5},
            SizeArray{0},
            BaseDataType::I32,  // Wrong type
            data.data()
        );
        REQUIRE(status == Status::Failure);
    }*/

    // Test writing string data with void* interface
    {
      std::vector<std::string> data = {"test"};
      IO::ArrayDataSetConfig config(
          BaseDataType::V_STR, SizeArray {1}, SizeArray {1});
      auto dataset = hdf5io->createArrayDataSet(config, "/errorDataset3");
      // Try to write string data using void* interface
      Status status = dataset->writeDataBlock(SizeArray {1},
                                              SizeArray {0},
                                              BaseDataType::V_STR,
                                              data.data()  // This should fail
      );
      REQUIRE(status == Status::Failure);
    }

    // Test position offset out of bounds
    {
      std::vector<int32_t> data = {1, 2, 3, 4, 5};
      IO::ArrayDataSetConfig config(
          BaseDataType::I32, SizeArray {5}, SizeArray {5});
      auto dataset = hdf5io->createArrayDataSet(config, "/errorDataset4");
      // Write at larger offset - should succeed by extending dataset
      Status status = dataset->writeDataBlock(
          SizeArray {5},
          SizeArray {10},  // Dataset will extend to accommodate
          BaseDataType::I32,
          data.data());
      REQUIRE(status == Status::Success);

      // Verify the dataset was extended
      REQUIRE(dataset->getShape()[0]
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
    IO::ArrayDataSetConfig config(
        BaseDataType::I32, SizeArray {2, 3}, SizeArray {2, 3});
    auto dataset = hdf5io->createArrayDataSet(config, "/2dDataset");

    // Write full 2D block
    Status status = dataset->writeDataBlock(
        SizeArray {2, 3}, SizeArray {0, 0}, BaseDataType::I32, data.data());
    REQUIRE(status == Status::Success);

    // Write partial block
    std::vector<int32_t> partial = {7, 8};
    status = dataset->writeDataBlock(
        SizeArray {1, 2}, SizeArray {1, 1}, BaseDataType::I32, partial.data());
    REQUIRE(status == Status::Success);
  }

  SECTION("3D operations")
  {
    std::vector<int32_t> data(24);  // 2x3x4 array
    std::iota(data.begin(), data.end(), 1);  // Fill with 1,2,3,...,24

    IO::ArrayDataSetConfig config(
        BaseDataType::I32, SizeArray {2, 3, 4}, SizeArray {2, 3, 4});
    auto dataset = hdf5io->createArrayDataSet(config, "/3dDataset");

    // Write full 3D block
    Status status = dataset->writeDataBlock(SizeArray {2, 3, 4},
                                            SizeArray {0, 0, 0},
                                            BaseDataType::I32,
                                            data.data());
    REQUIRE(status == Status::Success);

    // Write partial block
    std::vector<int32_t> partial = {100, 101, 102, 103};
    status = dataset->writeDataBlock(SizeArray {1, 1, 4},
                                     SizeArray {1, 1, 0},
                                     BaseDataType::I32,
                                     partial.data());
    REQUIRE(status == Status::Success);
  }

  hdf5io->close();
}

TEST_CASE("HDF5RecordingData overwrite scalar and fixed-size datasets",
          "[hdf5recordingdata]")
{
  std::string path = getTestFilePath("test_HDF5RecordingData_overwrite.h5");
  // Create a file with datasets that cannot be extended: a scalar fixed-length
  // string, a scalar variable-length string, a numeric scalar, and a
  // contiguous 1D dataset
  {
    IO::HDF5::HDF5IO hdf5io(path);
    hdf5io.open(IO::FileMode::Overwrite);
    REQUIRE(hdf5io.createStringDataSet("/fixed_str", "abcdef")
            == Status::Success);
    hdf5io.close();

    H5::H5File file(path, H5F_ACC_RDWR);
    H5::StrType vlenType(H5::PredType::C_S1, H5T_VARIABLE);
    vlenType.setCset(H5T_CSET_UTF8);  // as AqNWB writes strings
    H5::DataSet vlen =
        file.createDataSet("/vlen_str", vlenType, H5::DataSpace(H5S_SCALAR));
    const char* vlenValue = "variable";
    vlen.write(&vlenValue, vlenType);
    H5::DataSet scalarInt = file.createDataSet(
        "/scalar_int", H5::PredType::NATIVE_INT32, H5::DataSpace(H5S_SCALAR));
    const int32_t intValue = 1;
    scalarInt.write(&intValue, H5::PredType::NATIVE_INT32);
    const hsize_t dims[1] = {3};
    H5::DataSet fixed1D = file.createDataSet(
        "/fixed_1d", H5::PredType::NATIVE_INT32, H5::DataSpace(1, dims));
    const std::vector<int32_t> fixedValues = {1, 2, 3};
    fixed1D.write(fixedValues.data(), H5::PredType::NATIVE_INT32);
  }

  auto readString = [](IO::HDF5::HDF5IO& io, const std::string& dataPath)
  {
    auto data =
        IO::DataBlock<std::string>::fromGeneric(io.readDataset(dataPath));
    REQUIRE(data.data.size() == 1);
    // fixed-length strings are padded with null characters
    return std::string(data.data[0].c_str());
  };

  SECTION("overwrite datasets")
  {
    IO::HDF5::HDF5IO hdf5io(path);
    REQUIRE(hdf5io.open(IO::FileMode::ReadWrite) == Status::Success);

    // fixed-length string: same length and shorter values
    const BaseDataType strType = hdf5io.getStorageObjectDataType("/fixed_str");
    REQUIRE(strType.type == BaseDataType::Type::T_STR);
    REQUIRE(strType.typeSize == 6);
    auto fixedStr = hdf5io.getDataSet("/fixed_str");
    REQUIRE(fixedStr->getNumDimensions() == 0);
    REQUIRE(fixedStr->writeDataBlock(
                {}, {}, strType, std::vector<std::string> {"uvwxyz"})
            == Status::Success);
    REQUIRE(readString(hdf5io, "/fixed_str") == "uvwxyz");
    REQUIRE(fixedStr->writeDataBlock(
                {}, {}, strType, std::vector<std::string> {"uvw"})
            == Status::Success);
    REQUIRE(readString(hdf5io, "/fixed_str") == "uvw");

    // longer than the fixed length, or not exactly one value
    REQUIRE(fixedStr->writeDataBlock(
                {}, {}, strType, std::vector<std::string> {"abcdefg"})
            == Status::Failure);
    REQUIRE(fixedStr->writeDataBlock(
                {}, {}, strType, std::vector<std::string> {"a", "b"})
            == Status::Failure);
    REQUIRE(readString(hdf5io, "/fixed_str") == "uvw");

    // variable-length string
    auto vlenStr = hdf5io.getDataSet("/vlen_str");
    REQUIRE(vlenStr->writeDataBlock({},
                                    {},
                                    BaseDataType::V_STR,
                                    std::vector<std::string> {"a longer value"})
            == Status::Success);
    REQUIRE(readString(hdf5io, "/vlen_str") == "a longer value");

    // numeric scalar
    const int32_t newInt = 42;
    REQUIRE(hdf5io.getDataSet("/scalar_int")
                ->writeDataBlock({}, {}, BaseDataType::I32, &newInt)
            == Status::Success);
    auto intData =
        IO::DataBlock<int32_t>::fromGeneric(hdf5io.readDataset("/scalar_int"));
    REQUIRE(intData.data == std::vector<int32_t> {42});

    // fixed-size (contiguous) 1D dataset: overwriting works, extending fails
    auto fixed1D = hdf5io.getDataSet("/fixed_1d");
    const std::vector<int32_t> newValues = {7, 8};
    REQUIRE(
        fixed1D->writeDataBlock({2}, {1}, BaseDataType::I32, newValues.data())
        == Status::Success);
    auto fixedData =
        IO::DataBlock<int32_t>::fromGeneric(hdf5io.readDataset("/fixed_1d"));
    REQUIRE(fixedData.data == std::vector<int32_t> {1, 7, 8});
    REQUIRE(
        fixed1D->writeDataBlock({2}, {2}, BaseDataType::I32, newValues.data())
        == Status::Failure);
    hdf5io.close();
  }

  SECTION("overwrite in SWMR mode")
  {
    IO::HDF5::HDF5IO hdf5io(path);
    REQUIRE(hdf5io.open(IO::FileMode::ReadWrite) == Status::Success);
    REQUIRE(hdf5io.startRecording() == Status::Success);
    REQUIRE(hdf5io.canModifyObjects() == false);
    REQUIRE(
        hdf5io.getDataSet("/fixed_str")
            ->writeDataBlock(
                {}, {}, BaseDataType::STR(6), std::vector<std::string> {"swmr"})
        == Status::Success);
    REQUIRE(hdf5io.stopRecording() == Status::Success);
    hdf5io.close();

    IO::HDF5::HDF5IO readio(path);
    readio.open(IO::FileMode::ReadOnly);
    REQUIRE(readString(readio, "/fixed_str") == "swmr");
    readio.close();
  }

  SECTION("read-only file")
  {
    IO::HDF5::HDF5IO readio(path);
    readio.open(IO::FileMode::ReadOnly);
    REQUIRE(readio.getDataSet("/fixed_str")
                ->writeDataBlock({},
                                 {},
                                 BaseDataType::STR(6),
                                 std::vector<std::string> {"uvwxyz"})
            == Status::Failure);
    readio.close();
  }
}
