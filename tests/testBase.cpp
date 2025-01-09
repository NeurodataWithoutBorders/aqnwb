#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"
#include "nwb/RegisteredType.hpp"
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
  std::vector<float> data = getMockData1D(numSamples);
  BaseDataType timestampsType = BaseDataType::F64;
  std::vector<double> timestamps = getMockTimestamps(numSamples, 1);
  std::string path = getTestFilePath("testTimeseries.h5");

  SECTION("test writing timeseries data block")
  {
    // setup timeseries object
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
    std::unique_ptr<IO::HDF5::HDF5RecordingData> tsH5Dataset(
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(tsDset.release()));
    readH5DataBlock(tsH5Dataset->getDataSet(), timestampsType, tsBuffer);
    std::vector<double> tsRead(tsBuffer, tsBuffer + numSamples);
    delete[] tsBuffer;
    REQUIRE(tsRead == timestamps);

    // Read data back from file
    float* dataBuffer = new float[numSamples];
    std::unique_ptr<BaseRecordingData> dataDset =
        io->getDataSet(dataPath + "/data");
    std::unique_ptr<IO::HDF5::HDF5RecordingData> dataH5Dataset(
        dynamic_cast<IO::HDF5::HDF5RecordingData*>(dataDset.release()));
    readH5DataBlock(dataH5Dataset->getDataSet(), dataType, dataBuffer);
    std::vector<float> dataRead(dataBuffer, dataBuffer + numSamples);
    delete[] dataBuffer;
    REQUIRE_THAT(dataRead, Catch::Matchers::Approx(data).margin(1));

    // Read the "namespace" attribute
    DataBlockGeneric namespaceData = io->readAttribute(dataPath + "/namespace");
    auto namespaceBlock = DataBlock<std::string>::fromGeneric(namespaceData);
    std::string typeNamespace = namespaceBlock.data[0];
    REQUIRE(typeNamespace == "core");

    // Read the "neurodata_type" attribute
    DataBlockGeneric typeData = io->readAttribute(dataPath + "/neurodata_type");
    auto typeBlock = DataBlock<std::string>::fromGeneric(typeData);
    std::string typeName = typeBlock.data[0];
    REQUIRE(typeName == "TimeSeries");

    // Combine the namespace and type name to get the full class name
    std::string fullClassName = typeNamespace + "::" + typeName;
    // Create an instance of the corresponding RegisteredType subclass
    auto readContainer =
        AQNWB::NWB::RegisteredType::create(fullClassName, dataPath, io);
    std::string containerType = readContainer->getTypeName();
    REQUIRE(containerType == "TimeSeries");

    // Open the TimeSeries container directly from file using the utility method
    // This method does the same steps as above, i.e., read the attributes and
    // then create the type from the given name
    auto readTS = AQNWB::NWB::RegisteredType::create(dataPath, io);
    std::string readTSType = readContainer->getTypeName();
    REQUIRE(readTSType == "TimeSeries");
  }

  SECTION("test reading timeseries data")
  {
    // setup timeseries object
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    NWB::TimeSeries ts = NWB::TimeSeries(dataPath, io);
    std::string descripton = "Test TimeSeries";
    std::string comments = "Test comment";
    std::string unit = "volts";
    float conversion = 10.0;
    float resolution = 9.0;
    float offset = 8.0;
    ts.initialize(dataType,
                  unit,
                  descripton,
                  comments,
                  SizeArray {0},
                  SizeArray {1},
                  conversion,
                  resolution,
                  offset);

    // Write data to file
    Status writeStatus =
        ts.writeData(dataShape, positionOffset, data.data(), timestamps.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();
    io->close();

    // Read data back from file
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open(FileMode::ReadOnly);

    // Read all fields using the standard read methods
    auto readRegisteredType = NWB::RegisteredType::create(dataPath, readio);
    auto readTimeSeries =
        std::dynamic_pointer_cast<NWB::TimeSeries>(readRegisteredType);

    // Read the data
    auto readDataWrapper = readTimeSeries->readData<float>();
    REQUIRE(readDataWrapper->exists());
    REQUIRE(readDataWrapper->getStorageObjectType()
            == StorageObjectType::Dataset);
    REQUIRE(readDataWrapper->getPath() == "/tsdata/data");
    auto readDataValues = readDataWrapper->values();
    REQUIRE_THAT(readDataValues.data, Catch::Matchers::Approx(data).margin(1));

    // Read the timestamps
    auto readTimestampsWrapper = readTimeSeries->readTimestamps();
    auto readTimestampsValues = readTimestampsWrapper->values();
    REQUIRE(readDataWrapper->getStorageObjectType()
            == StorageObjectType::Dataset);
    REQUIRE(readTimestampsValues.data == timestamps);

    // Read the description
    auto readDescriptionWrapper = readTimeSeries->readDescription();
    auto readDescriptionValues = readDescriptionWrapper->values().data[0];
    REQUIRE(readDescriptionValues == descripton);

    // Read the comments
    auto readCommentsWrapper = readTimeSeries->readComments();
    auto readCommentsValues = readCommentsWrapper->values().data[0];
    REQUIRE(readCommentsValues == comments);

    // Read the data conversion
    auto readDataConversionWrapper = readTimeSeries->readDataConversion();
    auto readDataConversionValues = readDataConversionWrapper->values().data;
    REQUIRE(readDataConversionValues.size() == 1);
    REQUIRE(readDataConversionValues[0] == Catch::Approx(conversion));

    // Read the data resolution
    auto readDataResolutionWrapper = readTimeSeries->readDataResolution();
    auto readDataResolutionValues = readDataResolutionWrapper->values().data;
    REQUIRE(readDataResolutionValues.size() == 1);
    REQUIRE(readDataResolutionValues[0] == Catch::Approx(resolution));

    // Read the data offset
    auto readDataOffsetWrapper = readTimeSeries->readDataOffset();
    auto readDataOffsetValues = readDataOffsetWrapper->values().data;
    REQUIRE(readDataOffsetValues.size() == 1);
    REQUIRE(readDataOffsetValues[0] == Catch::Approx(offset));

    // TODO Read missing readDataContinuity, readControlDescription,
    // readControl, readTimestampsUnit, readTimestampsInterval,
    // readStartingTimeUnit, readStartingTimeRate, readStartingTime. Some of
    // these may not be written yet.
  }
}
