#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "nwb/misc/AnnotationSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("AnnotationSeries", "[misc]")
{
  // setup recording info
  SizeType numSamples = 10;
  std::string dataPath = "/annotations";
  std::vector<std::string> mockAnnotations = {
      "Subject moved",
      "Break started",
      "Break ended",
  };
  std::vector<double> mockTimestamps = getMockTimestamps(numSamples, 1);

  SECTION("test writing annotations")
  {
    // setup io object
    std::string path = getTestFilePath("AnnotationSeries.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // setup annotation series
    NWB::AnnotationSeries as = NWB::AnnotationSeries(dataPath, io);
    as.initialize("Test annotations",
                 "Test comments",
                 SizeArray {0},
                 SizeArray {1});

    // write annotations
    Status writeStatus = as.writeAnnotation(
        numSamples, mockAnnotations.data(), mockTimestamps.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();
    io->close();

    // Read data back from file
    std::unique_ptr<H5::H5File> file =
        std::make_unique<H5::H5File>(path, H5F_ACC_RDONLY);
    std::unique_ptr<H5::DataSet> dataset =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath + "/data"));

    // Read annotations
    std::vector<std::string> dataOut(numSamples);
    H5::StrType str_type(H5::PredType::C_S1, H5T_VARIABLE);
    H5::DataSpace fSpace = dataset->getSpace();
    dataset->read(dataOut.data(), str_type);

    REQUIRE_THAT(dataOut, Catch::Matchers::Equals(mockAnnotations));

    // Read timestamps
    std::unique_ptr<H5::DataSet> timestampsDataset =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath + "/timestamps"));
    std::vector<double> timestampsOut(numSamples);
    timestampsDataset->read(timestampsOut.data(), H5::PredType::NATIVE_DOUBLE);

    REQUIRE_THAT(timestampsOut, Catch::Matchers::Approx(mockTimestamps));
  }
}
