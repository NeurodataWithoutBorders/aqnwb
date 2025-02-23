#include <catch2/catch_test_macros.hpp>

#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"

using namespace AQNWB;

TEST_CASE("HDF5FilterConfig constructor", "[HDF5FilterConfig]")
{
  std::vector<unsigned int> cd_values = {1, 2, 3};
  IO::HDF5::HDF5FilterConfig filterConfig(H5Z_FILTER_DEFLATE, cd_values);

  REQUIRE(filterConfig.filter_id == H5Z_FILTER_DEFLATE);
  REQUIRE(filterConfig.cd_values.size() == 3);
  REQUIRE(filterConfig.cd_values[0] == 1);
  REQUIRE(filterConfig.cd_values[1] == 2);
  REQUIRE(filterConfig.cd_values[2] == 3);
}

TEST_CASE("HDF5FilterConfig factory methods", "[HDF5FilterConfig]")
{
  SECTION("createGzipFilter")
  {
    unsigned int level = 4;
    auto filterConfig = IO::HDF5::HDF5FilterConfig::createGzipFilter(level);

    REQUIRE(filterConfig.filter_id == H5Z_FILTER_DEFLATE);
    REQUIRE(filterConfig.cd_values.size() == 1);
    REQUIRE(filterConfig.cd_values[0] == level);
  }

  SECTION("createShuffleFilter")
  {
    auto filterConfig = IO::HDF5::HDF5FilterConfig::createShuffleFilter();

    REQUIRE(filterConfig.filter_id == H5Z_FILTER_SHUFFLE);
    REQUIRE(filterConfig.cd_values.size() == 0);
  }

  SECTION("createFletcher32Filter")
  {
    auto filterConfig = IO::HDF5::HDF5FilterConfig::createFletcher32Filter();

    REQUIRE(filterConfig.filter_id == H5Z_FILTER_FLETCHER32);
    REQUIRE(filterConfig.cd_values.size() == 0);
  }

  SECTION("createNbitFilter")
  {
    auto filterConfig = IO::HDF5::HDF5FilterConfig::createNbitFilter();

    REQUIRE(filterConfig.filter_id == H5Z_FILTER_NBIT);
    REQUIRE(filterConfig.cd_values.size() == 0);
  }
}

TEST_CASE("HDF5ArrayDataSetConfig addFilter", "[HDF5ArrayDataSetConfig]")
{
  IO::BaseDataType typeI32(IO::BaseDataType::T_I32, 1);
  SizeArray shape = {10, 1000};
  SizeArray chunking = {10, 100};
  IO::HDF5::HDF5ArrayDataSetConfig config(typeI32, shape, chunking);

  // Add GZIP compression filter (H5Z_FILTER_DEFLATE) with level 4
  unsigned int gzip_level = 4;
  std::vector<unsigned int> gzip_cd_values = {gzip_level};
  config.addFilter(H5Z_FILTER_DEFLATE, gzip_cd_values);

  // Add shuffle filter (H5Z_FILTER_SHUFFLE)
  std::vector<unsigned int> shuffle_cd_values = {};
  config.addFilter(H5Z_FILTER_SHUFFLE, shuffle_cd_values);

  const auto& filters = config.getFilters();
  REQUIRE(filters.size() == 2);

  REQUIRE(filters[0].filter_id == H5Z_FILTER_DEFLATE);
  REQUIRE(filters[0].cd_values.size() == 1);
  REQUIRE(filters[0].cd_values[0] == gzip_level);

  REQUIRE(filters[1].filter_id == H5Z_FILTER_SHUFFLE);
  REQUIRE(filters[1].cd_values.size() == 0);
}