#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"
#include "io/nwbio_utils.hpp"
#include "nwb/RegisteredTypeSpec.hpp"
#include "nwb/base/ProcessingModule.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodesTable.hpp"
#include "nwb/file/Subject.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "nwb/misc/AnnotationSeries.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("RegisteredTypeSpec base class", "[spec]")
{
  SECTION("static type information of RegisteredTypes")
  {
    // The static methods return compile-time constants
    // cppcheck-suppress knownConditionTrueFalse
    REQUIRE(NWB::TimeSeries::getStaticTypeName() == "TimeSeries");
    // cppcheck-suppress knownConditionTrueFalse
    REQUIRE(NWB::TimeSeries::getStaticNamespace() == "core");
  }

  SECTION("DataSpecs are RegisteredTypeSpecs")
  {
    auto dataSpec = NWB::VectorData::createDataSpec(
        "col",
        IO::ArrayDataSetConfig(IO::BaseDataType::I32, {0}, {10}),
        "a column");
    std::shared_ptr<NWB::RegisteredTypeSpec> baseSpec = dataSpec;
    REQUIRE(baseSpec->validate() == Status::Success);
  }
}

TEST_CASE("RegisteredTypeSpec fields and setters", "[spec]")
{
  IO::ArrayDataSetConfig config(IO::BaseDataType::F32, {0, 2}, {10, 2});

  SECTION("default values")
  {
    NWB::TimeSeries::Spec spec;
    REQUIRE(spec.data == nullptr);
    REQUIRE(spec.unit.empty());
    REQUIRE(spec.description == "no description");
    REQUIRE(spec.comments == "no comments");
    REQUIRE(spec.continuity == NWB::TimeSeries::ContinuityType::Undefined);
    REQUIRE_FALSE(spec.startingTime.has_value());
    REQUIRE(spec.controlDescription.empty());

    REQUIRE(NWB::ElectricalSeries::Spec().unit == "volts");
    REQUIRE(NWB::ElectricalSeries::Spec().useChannelConversion);
    REQUIRE(NWB::AnnotationSeries::Spec().unit == "n/a");
    REQUIRE_FALSE(NWB::Device::Spec().manufacturer.has_value());
  }

  SECTION("setters store values and data configurations are copied")
  {
    auto spec = NWB::TimeSeries::Spec()
                    .withData(config)
                    .withUnit("volts")
                    .withStartingTime(1.5)
                    .withControlDescription({"a", "b"});
    REQUIRE(spec.data != nullptr);
    REQUIRE(spec.data.get() != &config);
    REQUIRE(spec.unit == "volts");
    REQUIRE(spec.startingTime.value() > 1.0);
    REQUIRE(spec.controlDescription.size() == 2);
  }

  SECTION("withData keeps the dynamic type of the configuration")
  {
    IO::HDF5::HDF5ArrayDataSetConfig hdf5Config(
        IO::BaseDataType::F32, {0, 2}, {10, 2});
    const IO::BaseArrayDataSetConfig& baseRef = hdf5Config;
    auto spec = NWB::TimeSeries::Spec().withData(baseRef);
    REQUIRE(std::dynamic_pointer_cast<const IO::HDF5::HDF5ArrayDataSetConfig>(
                spec.data)
            != nullptr);
    auto linkSpec = NWB::TimeSeries::Spec().withData(
        IO::LinkArrayDataSetConfig("/some/data"));
    REQUIRE(linkSpec.data->isLink());

    // Column specs keep the dynamic type of the configuration as well
    auto columnSpec =
        NWB::VectorData::createDataSpec("col", hdf5Config, "a column");
    REQUIRE(dynamic_cast<const IO::HDF5::HDF5ArrayDataSetConfig*>(
                &columnSpec->getDataConfig())
            != nullptr);
  }

  SECTION("chaining inherited setters returns the derived spec")
  {
    Types::ChannelVector channels = getMockChannelArrays(2, 1)[0];
    auto spec = NWB::SpikeEventSeries::Spec()
                    .withData(config)
                    .withDescription("spikes")
                    .withChannelVector(channels)
                    .withUseChannelConversion(false);
    static_assert(std::is_same_v<decltype(spec), NWB::SpikeEventSeries::Spec>,
                  "chained setters must return the derived spec");
    static_assert(std::is_same_v<
                      decltype(NWB::ElectricalSeries::Spec().withComments("c")),
                      NWB::ElectricalSeries::Spec&>,
                  "inherited setters must return the derived spec");
    REQUIRE(spec.description == "spikes");
    REQUIRE(spec.channelVector.size() == 2);
    REQUIRE_FALSE(spec.useChannelConversion);
  }
}

TEST_CASE("RegisteredTypeSpec validation", "[spec]")
{
  IO::ArrayDataSetConfig config(IO::BaseDataType::F32, {0}, {10});

  SECTION("TimeSeries")
  {
    REQUIRE(NWB::TimeSeries::Spec().withUnit("volts").validate()
            == Status::Failure);  // missing data
    REQUIRE(NWB::TimeSeries::Spec().withData(config).validate()
            == Status::Failure);  // missing unit
    REQUIRE(NWB::TimeSeries::Spec()
                .withData(config)
                .withUnit("volts")
                .withStartingTime(-1.0)
                .validate()
            == Status::Failure);  // negative starting time
    REQUIRE(NWB::TimeSeries::Spec()
                .withData(config)
                .withUnit("volts")
                .withStartingTime(0.0)
                .withStartingTimeRate(0.0f)
                .validate()
            == Status::Failure);  // invalid rate
    REQUIRE(
        NWB::TimeSeries::Spec().withData(config).withUnit("volts").validate()
        == Status::Success);
  }

  SECTION("ElectricalSeries")
  {
    Types::ChannelVector channels = getMockChannelArrays(2, 1)[0];
    REQUIRE(NWB::ElectricalSeries::Spec().withData(config).validate()
            == Status::Failure);  // missing channels
    REQUIRE(NWB::ElectricalSeries::Spec()
                .withData(config)
                .withChannelVector(channels)
                .withUnit("millivolts")
                .validate()
            == Status::Failure);  // unit is fixed to volts
    REQUIRE(NWB::ElectricalSeries::Spec()
                .withData(config)
                .withChannelVector(channels)
                .validate()
            == Status::Success);
  }

  SECTION("AnnotationSeries")
  {
    REQUIRE(NWB::AnnotationSeries::Spec().withData(config).validate()
            == Status::Success);
    // The fixed values can only be changed via the TimeSeries::Spec setters,
    // which validate() rejects
    REQUIRE(NWB::AnnotationSeries::Spec()
                .withData(config)
                .withUnit("volts")
                .validate()
            == Status::Failure);
    REQUIRE(NWB::AnnotationSeries::Spec()
                .withData(config)
                .withConversion(2.0f)
                .validate()
            == Status::Failure);
  }

  SECTION("ElectrodeGroup")
  {
    REQUIRE(NWB::ElectrodeGroup::Spec().withLocation("CA1").validate()
            == Status::Failure);  // missing device
  }

  SECTION("validation through a base spec reference uses the derived checks")
  {
    NWB::ElectricalSeries::Spec esSpec;
    esSpec.withData(config);
    const NWB::TimeSeries::Spec& tsRef = esSpec;
    REQUIRE(tsRef.validate() == Status::Failure);  // missing channels
  }
}

TEST_CASE("Initialize RegisteredTypes from specs", "[spec]")
{
  SECTION("invalid specs fail without creating the object")
  {
    std::string path = getTestFilePath("testSpecInvalid.h5");
    std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
    io->open();
    auto ts = NWB::TimeSeries::create("/ts", io);
    REQUIRE(ts->initialize(NWB::TimeSeries::Spec().withUnit("volts"))
            == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/ts"));

    auto device = NWB::Device::create("/device", io);
    auto group = NWB::ElectrodeGroup::create("/group", io);
    REQUIRE(group->initialize(NWB::ElectrodeGroup::Spec().withLocation("CA1"))
            == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/group"));
    io->close();
  }

  SECTION("create and initialize from a spec")
  {
    std::string path = getTestFilePath("testSpecCreateInitialize.h5");
    std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
    io->open();

    auto module = NWB::ProcessingModule::create("/processing_module", io);
    REQUIRE(module != nullptr);
    REQUIRE(module->initialize(
                NWB::ProcessingModule::Spec().withDescription("processed data"))
            == Status::Success);
    REQUIRE(module->getPath() == "/processing_module");

    auto device = NWB::Device::create("/device", io);
    REQUIRE(device != nullptr);
    REQUIRE(device->initialize(NWB::Device::Spec().withDescription("a device"))
            == Status::Success);

    auto invalidGroup = NWB::ElectrodeGroup::create("/invalid_group", io);
    REQUIRE(invalidGroup->initialize(NWB::ElectrodeGroup::Spec())
            == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/invalid_group"));
    io->close();

    std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
    readIO->open(IO::FileMode::ReadOnly);
    auto readModule = NWB::RegisteredType::create<NWB::ProcessingModule>(
        "/processing_module", readIO);
    REQUIRE(readModule->readDescription()->values().data[0]
            == "processed data");
    auto readDevice =
        NWB::RegisteredType::create<NWB::Device>("/device", readIO);
    REQUIRE(readDevice->readDescription()->values().data[0] == "a device");
    // manufacturer is optional and not written if not set
    REQUIRE_FALSE(readDevice->readManufacturer()->exists());
    readIO->close();
  }

  SECTION("TimeSeries with a regular sampling rate")
  {
    std::string path = getTestFilePath("testSpecTimeSeriesRate.h5");
    std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
    io->open();
    auto ts = NWB::TimeSeries::create("/ts", io);
    IO::ArrayDataSetConfig config(IO::BaseDataType::F32, {0}, {10});
    REQUIRE(ts->initialize(NWB::TimeSeries::Spec()
                               .withData(config)
                               .withUnit("m/s")
                               .withStartingTime(2.0)
                               .withStartingTimeRate(100.0f))
            == Status::Success);
    io->close();

    std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
    readIO->open(IO::FileMode::ReadOnly);
    auto readTs = NWB::RegisteredType::create<NWB::TimeSeries>("/ts", readIO);
    REQUIRE(readTs->readStartingTime()->exists());
    REQUIRE_FALSE(readTs->readTimestamps()->exists());
    REQUIRE(readTs->readStartingTimeRate()->values().data[0] > 99.0f);
    readIO->close();
  }
}

TEST_CASE("Initialize a DynamicTable from a spec", "[spec]")
{
  std::string path = getTestFilePath("testSpecDynamicTable.h5");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();

  auto columns = NWB::DynamicTable::createDefaultDataSpecs();
  columns.push_back(NWB::VectorData::createDataSpec(
      "values",
      IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}),
      "the values"));
  auto spec = NWB::DynamicTable::Spec()
                  .withDescription("a test table")
                  .withColumns(columns);

  auto table = NWB::DynamicTable::create("/table", io);
  REQUIRE(table->initialize(spec) == Status::Success);
  REQUIRE(table->finalize() == Status::Success);
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readTable =
      NWB::RegisteredType::create<NWB::DynamicTable>("/table", readIO);
  REQUIRE(readTable->readDescription()->values().data[0] == "a test table");
  REQUIRE(readIO->objectExists("/table/id"));
  REQUIRE(readIO->objectExists("/table/values"));
  readIO->close();
}

// Disable deprecation warnings to test the deprecated positional initialize
// overloads, which are kept for backward compatibility
#if defined(__clang__) || defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4996)
#endif

TEST_CASE("Deprecated positional initialize methods", "[spec]")
{
  std::string path = getTestFilePath("testSpecDeprecatedInitialize.h5");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(2, 1);
  std::string groupName = mockArrays[0][0].getGroupName();
  io->createGroup("/general");
  io->createGroup("/general/devices");
  io->createGroup("/general/extracellular_ephys");

  auto device = NWB::Device::create("/general/devices/" + groupName, io);
  REQUIRE(device->initialize("description", "unknown") == Status::Success);
  auto elecGroup = NWB::ElectrodeGroup::create(
      "/general/extracellular_ephys/" + groupName, io);
  REQUIRE(elecGroup->initialize("description", "unknown", device)
          == Status::Success);
  auto elecTable = NWB::ElectrodesTable::create(io);
  REQUIRE(elecTable->initialize("description") == Status::Success);
  elecTable->addElectrodes(mockArrays[0]);
  REQUIRE(elecTable->finalize() == Status::Success);

  IO::ArrayDataSetConfig config(IO::BaseDataType::F32, {0, 2}, {10, 2});
  auto es = NWB::ElectricalSeries::create("/es", io);
  REQUIRE(es->initialize(config, mockArrays[0], "es") == Status::Success);
  IO::ArrayDataSetConfig sesConfig(
      IO::BaseDataType::F32, {0, 2, 5}, {10, 0, 0});
  auto ses = NWB::SpikeEventSeries::create("/ses", io);
  REQUIRE(ses->initialize(sesConfig, mockArrays[0], "ses") == Status::Success);

  IO::ArrayDataSetConfig tsConfig(IO::BaseDataType::F32, {0}, {10});
  auto ts = NWB::TimeSeries::create("/ts", io);
  REQUIRE(ts->initialize(tsConfig,
                         "m/s",
                         "description",
                         "comments",
                         1.0f,
                         -1.0f,
                         0.0f,
                         NWB::TimeSeries::ContinuityType::Continuous,
                         0.0,  // starting time
                         10.0f)
          == Status::Success);

  IO::ArrayDataSetConfig asConfig(IO::BaseDataType::V_STR, {0}, {10});
  auto as = NWB::AnnotationSeries::create("/as", io);
  REQUIRE(as->initialize("annotations", "comments", asConfig)
          == Status::Success);

  auto module = NWB::ProcessingModule::create("/module", io);
  REQUIRE(module->initialize("module") == Status::Success);

  NWB::Subject::SubjectSpec subjectSpec;  // deprecated alias of Subject::Spec
  subjectSpec.withSubjectId("subject");
  static_assert(std::is_same_v<NWB::Subject::SubjectSpec, NWB::Subject::Spec>,
                "SubjectSpec must be an alias of Subject::Spec");
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readEs =
      NWB::RegisteredType::create<NWB::ElectricalSeries>("/es", readIO);
  // the comments of the deprecated method are taken from the first channel
  REQUIRE(readEs->readComments()->values().data[0]
          == mockArrays[0][0].getComments());
  auto readTs = NWB::RegisteredType::create<NWB::TimeSeries>("/ts", readIO);
  REQUIRE(readTs->readStartingTime()->exists());
  auto readDevice = NWB::RegisteredType::create<NWB::Device>(
      "/general/devices/" + groupName, readIO);
  REQUIRE(readDevice->readManufacturer()->values().data[0] == "unknown");
  readIO->close();
}

#if defined(__clang__) || defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

TEST_CASE("TimeSeries::appendSamples", "[spec]")
{
  std::string path = getTestFilePath("testAppendSamples.h5");
  {
    std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
    io->open();
    auto rate = NWB::TimeSeries::create("/rate", io);
    REQUIRE(rate->initialize(NWB::TimeSeries::Spec()
                                 .withData(IO::ArrayDataSetConfig(
                                     IO::BaseDataType::F32, {0, 2}, {10, 2}))
                                 .withUnit("volts")
                                 .withStartingTime(0.0)
                                 .withStartingTimeRate(10.0f))
            == Status::Success);
    auto stamped = NWB::TimeSeries::create("/stamped", io);
    REQUIRE(stamped->initialize(NWB::TimeSeries::Spec()
                                    .withData(IO::ArrayDataSetConfig(
                                        IO::BaseDataType::F32, {0}, {10}))
                                    .withUnit("volts"))
            == Status::Success);
    auto linked = NWB::TimeSeries::create("/linked", io);
    REQUIRE(linked->initialize(
                NWB::TimeSeries::Spec()
                    .withData(IO::LinkArrayDataSetConfig("/stamped/data"))
                    .withUnit("volts"))
            == Status::Success);

    std::vector<float> values = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<double> timestamps = {0.0, 0.1};
    REQUIRE(rate->appendSamples(2, values.data()) == Status::Success);
    // The generic helper appends to any TimeSeries recording object
    REQUIRE(IO::appendTimeSeriesData(io->getRecordingObjects(),
                                     rate->getRecordingObjectIndex(),
                                     1,
                                     values.data())
            == Status::Success);
    REQUIRE(IO::appendTimeSeriesData(nullptr, 0, 1, values.data())
            == Status::Failure);
    // Timing mode mismatches fail instead of writing inconsistent data
    REQUIRE(rate->appendSamples(2, values.data(), timestamps.data())
            == Status::Failure);
    REQUIRE(rate->writeData({1, 2}, {2, 0}, values.data(), timestamps.data())
            == Status::Failure);  // no timestamps dataset
    REQUIRE(
        rate->writeData({1, 2}, {2, 0}, values.data(), nullptr, values.data())
        == Status::Failure);  // no control dataset
    REQUIRE(stamped->appendSamples(2, values.data()) == Status::Failure);
    REQUIRE(stamped->appendSamples(2, values.data(), timestamps.data())
            == Status::Success);
    // Writing to linked data is refused
    REQUIRE(linked->appendSamples(1, values.data(), timestamps.data())
            == Status::Failure);
    io->close();
  }

  // Append after reopening the file
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open(IO::FileMode::ReadWrite);
  auto rate = NWB::RegisteredType::create<NWB::TimeSeries>("/rate", io);
  std::vector<float> values = {5.0f, 6.0f};
  REQUIRE(rate->appendSamples(1, values.data()) == Status::Success);
  // The timing mode is determined from the file after reopening
  auto stamped = NWB::RegisteredType::create<NWB::TimeSeries>("/stamped", io);
  std::vector<double> timestamps = {0.2};
  REQUIRE(stamped->appendSamples(1, values.data()) == Status::Failure);
  REQUIRE(stamped->appendSamples(1, values.data(), timestamps.data())
          == Status::Success);
  io->close();

  std::shared_ptr<IO::BaseIO> readIO = createIO("HDF5", path);
  readIO->open(IO::FileMode::ReadOnly);
  auto readRate = NWB::RegisteredType::create<NWB::TimeSeries>("/rate", readIO);
  auto data = readRate->readData<float>()->values();
  REQUIRE(data.shape == SizeArray {4, 2});
  REQUIRE(
      data.data
      == std::vector<float> {1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 5.0f, 6.0f});
  auto readStamped =
      NWB::RegisteredType::create<NWB::TimeSeries>("/stamped", readIO);
  REQUIRE(readStamped->readData<float>()->values().shape == SizeArray {3});
  REQUIRE(readStamped->readTimestamps()->values().data
          == std::vector<double> {0.0, 0.1, 0.2});
  readIO->close();
}

TEST_CASE("Failed checks in initialize do not write anything", "[spec]")
{
  std::string path = getTestFilePath("testSpecNoPartialObjects.h5");
  std::shared_ptr<IO::BaseIO> io = createIO("HDF5", path);
  io->open();

  SECTION("invalid spec")
  {
    auto group = NWB::ElectrodeGroup::create("/group", io);
    REQUIRE(group->initialize(NWB::ElectrodeGroup::Spec()) == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/group"));
  }

  SECTION("invalid column specs of a table")
  {
    std::vector<NWB::DynamicTable::DataSpecPtr> columns = {
        NWB::VectorData::createDataSpec(
            "values",
            IO::ArrayDataSetConfig(IO::BaseDataType::F32, {0}, {10}),
            "no id column")};
    auto table = NWB::DynamicTable::create("/table", io);
    REQUIRE(
        table->initialize(
            NWB::DynamicTable::Spec().withDescription("d").withColumns(columns))
        == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/table"));
  }

  SECTION("missing electrodes table")
  {
    auto es = NWB::ElectricalSeries::create("/es", io);
    REQUIRE(
        es->initialize(NWB::ElectricalSeries::Spec()
                           .withData(IO::ArrayDataSetConfig(
                               IO::BaseDataType::F32, {0, 2}, {10, 2}))
                           .withChannelVector(getMockChannelArrays(2, 1)[0]))
        == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/es"));
  }

  SECTION("link target that is not in the file")
  {
    // The device object exists but has not been initialized in the file
    auto device = NWB::Device::create("/device", io);
    auto group = NWB::ElectrodeGroup::create("/group", io);
    REQUIRE(group->initialize(NWB::ElectrodeGroup::Spec()
                                  .withDescription("a group")
                                  .withLocation("CA1")
                                  .withDevice(device))
            == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/group"));
  }

  SECTION("linked data with a missing target")
  {
    auto series = NWB::TimeSeries::create("/series", io);
    REQUIRE(series->initialize(
                NWB::TimeSeries::Spec()
                    .withData(IO::LinkArrayDataSetConfig("/missing/data"))
                    .withUnit("volts"))
            == Status::Failure);
    REQUIRE_FALSE(io->objectExists("/series"));
  }
  io->close();
}
