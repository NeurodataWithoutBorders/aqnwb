
#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "Types.hpp"
#include "Channel.hpp"
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
  
  SECTION("initialize group if it already exists")
  {
    hdf5io.createGroup("/data");
    hdf5io.close();
  }
}

TEST_CASE("writeDataset", "[hdf5io]")
{
  // TODO write tests for writing datasets
}

TEST_CASE("writeLink", "[hdf5io]")
{
  // TODO write tests for writing soft links
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

  // close file
  hdf5io.close();
}

