
#include <catch2/catch_test_macros.hpp>
#include "HDF5IO.hpp"
#include "BaseIO.hpp"

std::string test_filename = "test.h5";

TEST_CASE("get_filename", "[io]")
{
  // Create instance of hdf5io
  HDF5IO hdf5io(test_filename);
  CHECK(test_filename == hdf5io.getFileName());
}

TEST_CASE("write_attribute", "[io]")
{
  // create file
  HDF5IO hdf5io(test_filename);
  hdf5io.open();

  // Write data to file
  const signed int data = 1;
	hdf5io.createGroup("/data");
  hdf5io.setAttribute(AQNWBIO::BaseDataType::I32, &data, "/test", "single_value");

  // close file
  hdf5io.close();
}
