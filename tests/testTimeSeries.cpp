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

TEST_CASE("TimeSeries is registerd as a subclass of RegisteredType", "[base]")
{
  auto registry = AQNWB::NWB::RegisteredType::getRegistry();
  REQUIRE(registry.find("core::TimeSeries") != registry.end());
}

TEST_CASE("TimeSeries", "[base]")
{
  // Prepare test data
  SizeType numSamples = 10;
  std::string dataPath = "/tsdata";
  std::vector<SizeType> dataShape = {numSamples};
  std::vector<SizeType> positionOffset = {0};
  BaseDataType dataType = BaseDataType::F32;
  std::vector<float> data = getMockData1D(numSamples);
  std::vector<double> timestamps = getMockTimestamps(numSamples, 1);
  std::vector<unsigned char> controlData = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
  std::vector<std::string> controlDescription = {"c0", "c1"};
  std::string path = getTestFilePath("testTimeseries.h5");

  SECTION("test writing and reading timeseries with timestamps")
  {
    // setup timeseries object
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    NWB::TimeSeries ts = NWB::TimeSeries(dataPath, io);
    std::string description = "Test TimeSeries";
    std::string comments = "Test comment";
    std::string unit = "volts";
    float conversion = 10.0;
    float resolution = 9.0;
    float offset = 8.0;
    std::vector<std::string> emptyControlDescription = {};
    AQNWB::NWB::TimeSeries::ContinuityType continuity =
        AQNWB::NWB::TimeSeries::Continuous;
    ts.initialize(
        dataType,
        unit,
        description,
        comments,
        SizeArray {0},
        SizeArray {1},
        conversion,
        resolution,
        offset,
        continuity,
        -1.0,  // don't use starting time
        1.0,  // starting time rate. Not used since starting time is -1
        emptyControlDescription  // empty to NOT use a control and
                                 // control_description dataset
    );
    REQUIRE(ts.timestamps != nullptr);
    REQUIRE(ts.starting_time == nullptr);
    REQUIRE(ts.control == nullptr);
    REQUIRE(ts.control_description == nullptr);

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
    REQUIRE(readDescriptionValues == description);

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

    // Read the data continuity
    auto readDataContinuityWrapper = readTimeSeries->readDataContinuity();
    auto readDataContinuityValues = readDataContinuityWrapper->values().data[0];
    REQUIRE(readDataContinuityValues
            == AQNWB::NWB::TimeSeries::ContinuityTypeNames[continuity]);

    // Read the timestamps unit
    auto readTimestampsUnitWrapper = readTimeSeries->readTimestampsUnit();
    auto readTimestampsUnitValues = readTimestampsUnitWrapper->values().data[0];
    REQUIRE(readTimestampsUnitValues == "seconds");

    // Read the timestamps interval
    auto readTimestampsIntervalWrapper =
        readTimeSeries->readTimestampsInterval();
    auto readTimestampsIntervalValues =
        readTimestampsIntervalWrapper->values().data[0];
    REQUIRE(readTimestampsIntervalValues == 1);

    // Test reading the missing starting_time, starting_time_rate, and
    // starting_time_unit
    auto readStartingTimeWrapper = readTimeSeries->readStartingTime();
    REQUIRE(!readStartingTimeWrapper->exists());
    auto readStartingTimeRateWrapper = readTimeSeries->readStartingTimeRate();
    REQUIRE(!readStartingTimeRateWrapper->exists());
    auto readStartingTimeUnitWrapper = readTimeSeries->readStartingTimeUnit();
    REQUIRE(!readStartingTimeUnitWrapper->exists());

    // Read the neurodata_type
    auto readNeurodataTypeWrapper = readTimeSeries->readNeurodataType();
    auto readNeurodataTypeValues = readNeurodataTypeWrapper->values().data[0];
    REQUIRE(readNeurodataTypeValues == "TimeSeries");

    // Read the namespace
    auto readNamespaceWrapper = readTimeSeries->readNamespace();
    auto readNamespaceValues = readNamespaceWrapper->values().data[0];
    REQUIRE(readNamespaceValues == "core");

    readio->close();
  }

  SECTION("test writing and reading timeseries with starting time")
  {
    // setup timeseries object
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    NWB::TimeSeries ts = NWB::TimeSeries(dataPath, io);
    std::string description = "Test TimeSeries";
    std::string comments = "Test comment";
    std::string unit = "volts";
    float conversion = 10.0;
    float resolution = 9.0;
    float offset = 8.0;
    AQNWB::NWB::TimeSeries::ContinuityType continuity =
        AQNWB::NWB::TimeSeries::Continuous;
    double startingTime = 0.0;
    float startingTimeRate = 1.0;
    ts.initialize(dataType,
                  unit,
                  description,
                  comments,
                  SizeArray {0},
                  SizeArray {1},
                  conversion,
                  resolution,
                  offset,
                  continuity,
                  startingTime,
                  startingTimeRate,
                  controlDescription);
    REQUIRE(ts.timestamps == nullptr);
    REQUIRE(ts.starting_time != nullptr);
    REQUIRE(ts.control != nullptr);
    REQUIRE(ts.control_description != nullptr);

    // Write data to file
    Status writeStatus = ts.writeData(dataShape,
                                      positionOffset,
                                      data.data(),
                                      nullptr,  // no timestamps
                                      controlData.data());

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

    // Read the starting time
    auto readStartingTimeWrapper = readTimeSeries->readStartingTime();
    auto readStartingTimeValues = readStartingTimeWrapper->values().data[0];
    REQUIRE(readStartingTimeValues == Catch::Approx(startingTime));

    // Read the starting time rate
    auto readStartingTimeRateWrapper = readTimeSeries->readStartingTimeRate();
    auto readStartingTimeRateValues =
        readStartingTimeRateWrapper->values().data[0];
    REQUIRE(readStartingTimeRateValues == Catch::Approx(startingTimeRate));

    // Read the starting time unit
    auto readStartingTimeUnitWrapper = readTimeSeries->readStartingTimeUnit();
    auto readStartingTimeUnitValues =
        readStartingTimeUnitWrapper->values().data[0];
    REQUIRE(readStartingTimeUnitValues == "seconds");

    // Read missing timestamps, timestamps unit, and timestamps interval
    auto readTimestampsWrapper = readTimeSeries->readTimestamps();
    REQUIRE(!readTimestampsWrapper->exists());
    auto readTimestampsUnitWrapper = readTimeSeries->readTimestampsUnit();
    REQUIRE(!readTimestampsUnitWrapper->exists());
    auto readTimestampsIntervalWrapper =
        readTimeSeries->readTimestampsInterval();
    REQUIRE(!readTimestampsIntervalWrapper->exists());

    // Read the control data
    auto readControlWrapper = readTimeSeries->readControl();
    auto readControlValues = readControlWrapper->values();
    REQUIRE_THAT(readControlValues.data, Catch::Matchers::Equals(controlData));

    // Read the control description
    auto readControlDescriptionWrapper =
        readTimeSeries->readControlDescription();
    auto readControlDescriptionValues = readControlDescriptionWrapper->values();
    REQUIRE(readControlDescriptionValues.data == controlDescription);

    readio->close();
  }
}
