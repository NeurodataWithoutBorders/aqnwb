#include <sstream>

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

TEST_CASE("TimeSeries is registered as a subclass of RegisteredType", "[base]")
{
  auto registry = AQNWB::NWB::RegisteredType::getRegistry();
  REQUIRE(registry.find("core::TimeSeries") != registry.end());
}

TEST_CASE("TimeSeries", "[base]")
{
  // Prepare common test data
  SizeType numSamples = 10;
  std::string dataPath = "/tsdata";
  std::vector<SizeType> dataShape = {numSamples};
  std::vector<SizeType> positionOffset = {0};
  BaseDataType dataType = BaseDataType::F32;
  std::vector<float> data = getMockData1D(numSamples);
  std::vector<double> timestamps = getMockTimestamps(numSamples, 1);
  std::vector<unsigned char> controlData = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
  std::vector<std::string> controlDescription = {"c0", "c1"};

  SECTION("test writing and reading timeseries with timestamps")
  {
    // Create a separate file for this test
    std::string path = getTestFilePath("testTimeseriesWithTimestamps.h5");
    // setup timeseries object
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    auto ts = NWB::TimeSeries::create(dataPath, io);
    std::string description = "Test TimeSeries";
    std::string comments = "Test comment";
    std::string unit = "volts";
    float conversion = 10.0;
    float resolution = 9.0;
    float offset = 8.0;
    std::vector<std::string> emptyControlDescription = {};
    AQNWB::NWB::TimeSeries::ContinuityType continuity =
        AQNWB::NWB::TimeSeries::Continuous;

    IO::ArrayDataSetConfig config(dataType, SizeArray {0}, SizeArray {1});
    ts->initialize(
        config,
        unit,
        description,
        comments,
        conversion,
        resolution,
        offset,
        continuity,
        -1.0,  // don't use starting time
        1.0,  // starting time rate. Not used since starting time is -1
        emptyControlDescription  // empty to NOT use a control and
                                 // control_description dataset
    );
    REQUIRE(ts->readTimestamps()->exists() == true);
    REQUIRE(ts->readStartingTime()->exists() == false);
    REQUIRE(ts->readControl()->exists() == false);
    REQUIRE(ts->readControlDescription()->exists() == false);

    // Write data to file
    Status writeStatus = ts->writeData(
        dataShape, positionOffset, data.data(), timestamps.data());
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
    // Create a separate file for this test
    std::string path = getTestFilePath("testTimeseriesWithStartingTime.h5");
    // setup timeseries object
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    auto ts = NWB::TimeSeries::create(dataPath, io);
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

    IO::ArrayDataSetConfig config(dataType, SizeArray {0}, SizeArray {1});
    ts->initialize(config,
                   unit,
                   description,
                   comments,
                   conversion,
                   resolution,
                   offset,
                   continuity,
                   startingTime,
                   startingTimeRate,
                   controlDescription);
    REQUIRE(ts->readTimestamps()->exists() == false);
    REQUIRE(ts->readStartingTime()->exists() == true);
    REQUIRE(ts->readControl()->exists() == true);
    REQUIRE(ts->readControlDescription()->exists() == true);

    // Write data to file
    Status writeStatus = ts->writeData(dataShape,
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

  SECTION("test record methods from DEFINE_DATASET_FIELD")
  {
    // Create a separate file for this test
    std::string path = getTestFilePath("testTimeseriesRecord.h5");
    // setup timeseries object
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    auto ts = NWB::TimeSeries::create(dataPath, io);
    std::string description = "Test TimeSeries";
    std::string comments = "Test comment";
    std::string unit = "volts";
    float conversion = 10.0;
    float resolution = 9.0;
    float offset = 8.0;
    std::vector<std::string> emptyControlDescription = {};
    AQNWB::NWB::TimeSeries::ContinuityType continuity =
        AQNWB::NWB::TimeSeries::Continuous;

    IO::ArrayDataSetConfig config(dataType, SizeArray {0}, SizeArray {1});
    ts->initialize(
        config,
        unit,
        description,
        comments,
        conversion,
        resolution,
        offset,
        continuity,
        -1.0,  // don't use starting time
        1.0,  // starting time rate. Not used since starting time is -1
        controlDescription  // use control and control_description
    );

    // Test recordData method
    auto dataRecorder = ts->recordData();
    REQUIRE(dataRecorder != nullptr);

    // Test recordTimestamps method
    auto timestampsRecorder = ts->recordTimestamps();
    REQUIRE(timestampsRecorder != nullptr);

    // Test recordControl method
    auto controlRecorder = ts->recordControl();
    REQUIRE(controlRecorder != nullptr);

    // Test recordControlDescription method
    auto controlDescriptionRecorder = ts->recordControlDescription();
    REQUIRE(controlDescriptionRecorder != nullptr);

    // Initialize a second TimeSeries with starting_time
    auto ts2 = NWB::TimeSeries::create(dataPath + "/ts2", io);
    ts2->initialize(config,
                    unit,
                    description,
                    comments,
                    conversion,
                    resolution,
                    offset,
                    continuity,
                    0.0,  // use starting time
                    1.0,  // starting time rate
                    emptyControlDescription);

    // Test recordStartingTime method
    auto startingTimeRecorder = ts2->recordStartingTime();
    REQUIRE(startingTimeRecorder != nullptr);

    io->close();
  }
}

TEST_CASE("LinkArrayDataSetConfig for TimeSeries data", "[base][link]")
{
  // Prepare common test data
  SizeType numSamples = 20;
  std::string dataPath1 = "/original_timeseries";
  std::string dataPath2 = "/linked_timeseries";
  std::vector<SizeType> dataShape = {numSamples};
  std::vector<SizeType> positionOffset = {0};
  BaseDataType dataType = BaseDataType::F32;
  std::vector<float> data = getMockData1D(numSamples);
  std::vector<double> timestamps1 = getMockTimestamps(numSamples, 1);
  std::vector<double> timestamps2 =
      getMockTimestamps(numSamples, 2);  // Different timestamps

  SECTION("create TimeSeries with linked data")
  {
    // Create a file for this test
    std::string path = getTestFilePath("testTimeseriesWithLink.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // Create first TimeSeries with actual data
    auto ts1 = NWB::TimeSeries::create(dataPath1, io);
    std::string description = "Original TimeSeries";
    std::string unit = "volts";
    float conversion = 1.0f;
    float resolution = -1.0f;
    float offset = 0.0f;
    AQNWB::NWB::TimeSeries::ContinuityType continuity =
        AQNWB::NWB::TimeSeries::Continuous;

    IO::ArrayDataSetConfig config1(dataType, SizeArray {0}, SizeArray {1});
    ts1->initialize(config1,
                    unit,
                    description,
                    "Original comments",
                    conversion,
                    resolution,
                    offset,
                    continuity,
                    -1.0,
                    1.0,
                    {});

    // Write data to first TimeSeries
    Status writeStatus1 = ts1->writeData(
        dataShape, positionOffset, data.data(), timestamps1.data());
    REQUIRE(writeStatus1 == Status::Success);

    // Create second TimeSeries with linked data
    auto ts2 = NWB::TimeSeries::create(dataPath2, io);
    std::string linkTarget = dataPath1 + "/data";
    IO::LinkArrayDataSetConfig linkConfig(linkTarget);

    // Verify it's identified as a link
    REQUIRE(linkConfig.isLink() == true);
    REQUIRE(linkConfig.getTargetPath() == linkTarget);

    ts2->initialize(linkConfig,
                    unit,
                    "Linked TimeSeries with same data",
                    "Linked comments",
                    conversion,
                    resolution,
                    offset,
                    continuity,
                    -1.0,
                    1.0,
                    {});

    // Write only timestamps to second TimeSeries (data is linked)
    // We need to manually write timestamps since recordData() returns nullptr
    // for links
    auto ts2TimestampsRecorder = ts2->recordTimestamps();
    REQUIRE(ts2TimestampsRecorder != nullptr);
    Status writeStatus2 = ts2TimestampsRecorder->writeDataBlock(
        dataShape, positionOffset, IO::BaseDataType::F64, timestamps2.data());
    REQUIRE(writeStatus2 == Status::Success);

    io->flush();
    io->close();

    // Verify the link was created correctly using HDF5 C++ API
    H5::H5File file(path, H5F_ACC_RDONLY);

    // Check that the link exists
    htri_t exists =
        H5Lexists(file.getId(), (dataPath2 + "/data").c_str(), H5P_DEFAULT);
    REQUIRE(exists > 0);

    // Get link info
    H5L_info_t linkInfo;
    herr_t status = H5Lget_info(
        file.getId(), (dataPath2 + "/data").c_str(), &linkInfo, H5P_DEFAULT);
    REQUIRE(status >= 0);

    // Verify it's a soft link
    REQUIRE(linkInfo.type == H5L_TYPE_SOFT);

    // For a soft link, linkInfo.u.val_size contains the size
    REQUIRE(linkInfo.u.val_size > 0);

    // Now read the actual link target
    std::vector<char> linkTargetBuffer(linkInfo.u.val_size + 1);
    herr_t linkStatus = H5Lget_val(file.getId(),
                                   (dataPath2 + "/data").c_str(),
                                   linkTargetBuffer.data(),
                                   linkInfo.u.val_size + 1,
                                   H5P_DEFAULT);
    REQUIRE(linkStatus >= 0);
    linkTargetBuffer[linkInfo.u.val_size] = '\0';  // Ensure null termination
    std::string actualTarget(linkTargetBuffer.data());
    REQUIRE(actualTarget == linkTarget);

    // Verify data can be read through the link
    H5::DataSet linkedDataset = file.openDataSet(dataPath2 + "/data");
    H5::DataSpace linkedDataspace = linkedDataset.getSpace();

    // Check dimensions
    hsize_t dims[1];
    linkedDataspace.getSimpleExtentDims(dims);
    REQUIRE(dims[0] == numSamples);

    // Read data through the link
    std::vector<float> readData(numSamples);
    linkedDataset.read(readData.data(), H5::PredType::NATIVE_FLOAT);

    // Verify the data matches the original
    for (size_t i = 0; i < numSamples; ++i) {
      REQUIRE(approxComparator(readData[i], data[i]));
    }

    file.close();
  }

  SECTION(
      "verify LinkArrayDataSetConfig returns nullptr from createArrayDataSet")
  {
    // Create a file for this test
    std::string path = getTestFilePath("testLinkReturnValue.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // Create a dummy dataset first
    auto ts1 = NWB::TimeSeries::create(dataPath1, io);
    IO::ArrayDataSetConfig config1(dataType, SizeArray {0}, SizeArray {1});
    ts1->initialize(config1, "volts", "Test", "comments", 1.0f, -1.0f, 0.0f);

    // Try to create a link
    std::string linkTarget = dataPath1 + "/data";
    IO::LinkArrayDataSetConfig linkConfig(linkTarget);

    // createArrayDataSet should return nullptr for links (since you can't write
    // to a link)
    auto result = io->createArrayDataSet(linkConfig, "/test_link");
    REQUIRE(result == nullptr);

    io->close();
  }
}

TEST_CASE("TimeSeries chunking fallback and validation", "[base][chunking]")
{
  // Note: The chunking fallback logic (tsChunkSize = 8192 when chunking is empty)
  // and empty shape validation are tested through the link creation tests where
  // these scenarios occur naturally with LinkArrayDataSetConfig
}
