// [example_all]
#include <catch2/catch_test_macros.hpp>

#include "hdf5/HDF5IO.hpp"
#include "testUtils.hpp"

TEST_CASE("SimpleExamples", "[hdf5io]")
{
  SECTION("docsExample")
  {
    std::string path = getTestFilePath("testWithSWMRMode.h5");
    // [example_hdf5io_code_snippet]
    std::unique_ptr<AQNWB::HDF5::HDF5IO> hdf5io =
        std::make_unique<AQNWB::HDF5::HDF5IO>(path);
    // [example_hdf5io_code_snippet]
    hdf5io->open();
    hdf5io->close();
  }
}
// [example_all]
