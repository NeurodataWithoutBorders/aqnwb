#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "BaseIO.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("TimeSeries", "[base]")
{
  // Prepare test data
  SizeType numSamples = 10;
  std::string dataPath = "/tsdata";
  std::vector<SizeType> dataShape = {numSamples};
  std::vector<SizeType> positionOffset = {0};
  BaseDataType dataType = BaseDataType::F32;
  std::vector<float> data = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9};  //  getMockData1D(numSamples);
  BaseDataType timestampsType = BaseDataType::F64;
  std::vector<double> timestamps = getMockTimestamps(numSamples, 1);

  SECTION("test writing timeseries data block")
  {
    // setup timeseries object
    std::string path = getTestFilePath("testTimeseries.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    NWB::TimeSeries ts = NWB::TimeSeries(dataPath, io);
    ts.initialize(dataType, "unit");

    // Write data to file
    Status writeStatus =
        ts.writeData(dataShape, positionOffset, data.data(), timestamps.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Read timestamps back from file
    double* tsBuffer = new double[numSamples];
    std::unique_ptr<BaseRecordingData> tsDset =
        io->getDataSet(dataPath + "/timestamps");
    std::unique_ptr<HDF5::HDF5RecordingData> tsH5Dataset(
        dynamic_cast<HDF5::HDF5RecordingData*>(tsDset.release()));
    readH5DataBlock(tsH5Dataset->getDataSet(), timestampsType, tsBuffer);
    std::vector<double> tsRead(tsBuffer, tsBuffer + numSamples);
    delete[] tsBuffer;
    REQUIRE(tsRead == timestamps);

    // Read data back from file
    float* dataBuffer = new float[numSamples];
    std::unique_ptr<BaseRecordingData> dataDset =
        io->getDataSet(dataPath + "/data");
    std::unique_ptr<HDF5::HDF5RecordingData> dataH5Dataset(
        dynamic_cast<HDF5::HDF5RecordingData*>(dataDset.release()));
    readH5DataBlock(dataH5Dataset->getDataSet(), dataType, dataBuffer);
    std::vector<float> dataRead(dataBuffer, dataBuffer + numSamples);
    delete[] dataBuffer;
    REQUIRE_THAT(dataRead, Catch::Matchers::Approx(data).margin(1));
  }
}
