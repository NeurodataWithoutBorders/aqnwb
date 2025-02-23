#include <catch2/catch_test_macros.hpp>

#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"

using namespace AQNWB;

TEST_CASE("HDF5FilterConfig constructor", "[HDF5FilterConfig]")
{
  unsigned int cd_values[3] = {1, 2, 3};
  IO::HDF5::HDF5FilterConfig filterConfig(H5Z_FILTER_DEFLATE, 3, cd_values);

  REQUIRE(filterConfig.filter_id == H5Z_FILTER_DEFLATE);
  REQUIRE(filterConfig.cd_nelmts == 3);
  REQUIRE(filterConfig.cd_values.size() == 3);
  REQUIRE(filterConfig.cd_values[0] == 1);
  REQUIRE(filterConfig.cd_values[1] == 2);
  REQUIRE(filterConfig.cd_values[2] == 3);
}

TEST_CASE("HDF5ArrayDataSetConfig addFilter", "[HDF5ArrayDataSetConfig]")
{
  IO::BaseDataType typeI32(IO::BaseDataType::T_I32, 1);
  SizeArray shape = {10, 1000};
  SizeArray chunking = {10, 100};
  IO::HDF5::HDF5ArrayDataSetConfig config(typeI32, shape, chunking);

  // Add GZIP compression filter (H5Z_FILTER_DEFLATE) with level 4
  unsigned int gzip_level = 4;
  config.addFilter(H5Z_FILTER_DEFLATE, 1, &gzip_level);

  // Add shuffle filter (H5Z_FILTER_SHUFFLE)
  config.addFilter(IO::HDF5::HDF5FilterConfig(H5Z_FILTER_SHUFFLE, 0, nullptr));

  const auto& filters = config.getFilters();
  REQUIRE(filters.size() == 2);

  REQUIRE(filters[0].filter_id == H5Z_FILTER_DEFLATE);
  REQUIRE(filters[0].cd_nelmts == 1);
  REQUIRE(filters[0].cd_values.size() == 1);
  REQUIRE(filters[0].cd_values[0] == gzip_level);

  REQUIRE(filters[1].filter_id == H5Z_FILTER_SHUFFLE);
  REQUIRE(filters[1].cd_nelmts == 0);
  REQUIRE(filters[1].cd_values.size() == 0);
}