
#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "HDF5IO.hpp"
#include "NWBFile.hpp"

namespace fs = std::filesystem;

std::string getTestFilePath(std::string filename)
{
  // create data directory if it doesn't exist
  fs::path dirPath = fs::current_path() / "data";
  fs::directory_entry dir(dirPath);
  if (!dir.exists()) {
    fs::create_directory(dir);
  }

  // get filename and remove old file
  fs::path filepath = dirPath / filename;
  if (fs::exists(filepath)) {
    fs::remove(filepath);
  }

  return filepath.u8string();
}

TEST_CASE("write_attributes", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_attributes.h5");
  HDF5IO hdf5io(filename);
  hdf5io.open();

  hdf5io.createGroup("/data");

  // single attribute
  SECTION("single_value")
  {
    const signed int data = 1;
    hdf5io.setAttribute(
        AQNWBIO::BaseDataType::I32, &data, "/data", "single_value");
  }

  // integer array
  SECTION("int_array")
  {
    const int data[] = {1, 2, 3, 4, 5};
    const int dataSize = sizeof(data) / sizeof(data[0]);

    hdf5io.setAttribute(
        AQNWBIO::BaseDataType::I32, &data, "/data", "array", dataSize);
  }

  // integer array
  SECTION("str_array")
  {
    const std::vector<std::string> data = {"col1", "col2", "col3"};

    hdf5io.setAttribute(data, "/data", "string_array");
  }

  // close file
  hdf5io.close();
}

TEST_CASE("generate_nwbfile", "[nwb]")
{
  std::string filename = getTestFilePath("test_nwb_file.h5");

  NWBFile nwbfile("123", std::make_unique<HDF5IO>(filename));
  nwbfile.initialize();
  nwbfile.finalize();
}
