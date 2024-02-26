
#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "HDF5IO.hpp"

namespace fs = std::filesystem;

std::string getTestFilePath(std::string filename)
{
  fs::path dirPath = fs::current_path() / "data";
  fs::directory_entry dir(dirPath);
  if (!dir.exists()) {
    fs::create_directory(dir);
  }
  fs::path filepath = dirPath / filename;

  return filepath.u8string();
}

TEST_CASE("get_filename", "[io]")
{
  // create test data folder
  std::string filename = getTestFilePath("test.h5");

  // Create instance of hdf5io
  HDF5IO hdf5io(filename);
  CHECK(filename == hdf5io.getFileName());
}

TEST_CASE("write_attribute", "[io]")
{
  std::string filename = getTestFilePath("test_attribute.h5");

  // create file
  HDF5IO hdf5io(filename);
  hdf5io.open();

  // Write data to file
  const signed int data = 1;
  hdf5io.createGroup("/data");
  hdf5io.setAttribute(
      AQNWBIO::BaseDataType::I32, &data, "/data", "single_value");

  // close file
  hdf5io.close();
  // std::remove(filename.c_str());
}

TEST_CASE("write_int_array", "[io]")
{
  std::string filename = getTestFilePath("test_int_array.h5");

  HDF5IO hdf5io(filename);
  hdf5io.open();

  // Setup data structures
  const int data[] = {1, 2, 3, 4, 5};
  const int dataSize = sizeof(data) / sizeof(data[0]);

  // Write data to file
  hdf5io.createGroup("/data");
  hdf5io.setAttribute(
      AQNWBIO::BaseDataType::I32, &data, "/data", "array", dataSize);

  hdf5io.close();
  // std::remove(filename.c_str());
}

TEST_CASE("write_str_array", "[io]")
{
  std::string filename = getTestFilePath("test_str_array.h5");

  HDF5IO hdf5io(filename);
  hdf5io.open();

  // Setup data structures
  const std::vector<std::string> data = {"col1", "col2", "col3"};

  // Write data to file
  hdf5io.createGroup("/data");
  hdf5io.setAttributeStr(
      AQNWBIO::BaseDataType::T_STR_ARR, data, "/data", "string_array");

  hdf5io.close();
  // std::remove(filename.c_str());
}
