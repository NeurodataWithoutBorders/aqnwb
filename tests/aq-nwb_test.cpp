
#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "HDF5IO.hpp"
#include "NWBFile.hpp"

namespace fs = std::filesystem;


// TODO - change setup/teardown of data folder when running tests
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


TEST_CASE("write_attribute", "[io]")
{
  std::string filename = getTestFilePath("test_attribute.h5");

  // create file
  HDF5IO hdf5io;
  hdf5io.open(filename);

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

  HDF5IO hdf5io;
  hdf5io.open(filename);

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

  HDF5IO hdf5io;
  hdf5io.open(filename);

  // Setup data structures
  const std::vector<std::string> data = {"col1", "col2", "col3"};

  // Write data to file
  hdf5io.createGroup("/data");
  hdf5io.setAttribute(data, "/data", "string_array");

  hdf5io.close();
  // std::remove(filename.c_str());
}

TEST_CASE("nwbfile_generation", "[nwb]")
{
std::string filename = getTestFilePath("test_nwb_file.h5");

NWBFile nwbfile(filename, "123", std::make_unique<HDF5IO>());
nwbfile.open();
nwbfile.close();
}
