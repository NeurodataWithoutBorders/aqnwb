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
  SizeType numSamples = 3;
  std::string dataPath = "/annotations";
  std::vector<std::string> mockAnnotations = {
      "Subject moved",
      "Break started",
      "Break ended",
  };
  std::vector<double> mockTimestamps = getMockTimestamps(numSamples, 1);
  std::vector<double> mockTimestamps2 = mockTimestamps;
  for (double& value : mockTimestamps2) {
    value += 5;
  }

  SECTION("test writing annotations")
  {
    // setup io object
    std::string path = getTestFilePath("AnnotationSeries.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // setup annotation series
    NWB::AnnotationSeries as = NWB::AnnotationSeries(dataPath, io);
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {1});
    as.initialize("Test annotations", "Test comments", config);

    // write annotations multiple times to test adding to same dataset
    Status writeStatus =
        as.writeAnnotation(numSamples, mockAnnotations, mockTimestamps.data());
    REQUIRE(writeStatus == Status::Success);
    Status writeStatus2 =
        as.writeAnnotation(numSamples, mockAnnotations, mockTimestamps2.data());
    REQUIRE(writeStatus2 == Status::Success);
    io->flush();

    // Read annotations back from file
    std::vector<std::string> expectedAnnotations = mockAnnotations;
    expectedAnnotations.insert(expectedAnnotations.end(),
                               mockAnnotations.begin(),
                               mockAnnotations.end());
    std::vector<std::string> dataOut(expectedAnnotations.size());

    auto readDataWrapper = as.readData();
    auto readAnnotationsDataTyped = readDataWrapper->values();
    REQUIRE(readAnnotationsDataTyped.data == expectedAnnotations);

    // Read timestamps
    std::vector<double> expectedTimestamps = mockTimestamps;
    expectedTimestamps.insert(expectedTimestamps.end(),
                              mockTimestamps2.begin(),
                              mockTimestamps2.end());
    std::vector<double> timestampsOut(expectedTimestamps.size());

    auto readTimestampsWrapper = as.readTimestamps();
    auto readTimestampsDataTyped = readTimestampsWrapper->values();
    REQUIRE_THAT(readTimestampsDataTyped.data,
                 Catch::Matchers::Approx(expectedTimestamps));
  }
}
