#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB::NWB;

// Test class with custom type name
class CustomNameType : public RegisteredType
{
public:
  CustomNameType(const std::string& path, std::shared_ptr<IO::BaseIO> io)
      : RegisteredType(path, io)
  {
  }

  virtual std::string getTypeName() const override { return "CustomType"; }
  virtual std::string getNamespace() const override { return "test"; }
};

// Test class with field definitions
class TestFieldType : public RegisteredType
{
public:
  TestFieldType(const std::string& path, std::shared_ptr<IO::BaseIO> io)
      : RegisteredType(path, io)
  {
  }

  virtual std::string getTypeName() const override { return "TestFieldType"; }
  virtual std::string getNamespace() const override { return "test"; }

  DEFINE_FIELD(testAttribute,
               AttributeField,
               int32_t,
               "test_attr",
               "Test attribute field")
  DEFINE_FIELD(
      testDataset, DatasetField, float, "test_dataset", "Test dataset field")
};

TEST_CASE("RegisterType", "[base]")
{
  SECTION("test that the registry is working")
  {
    std::string filename = getTestFilePath("testRegisteredTypeRegistry.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<IO::HDF5::HDF5IO>(filename);
    std::string examplePath("/example/path");

    // Test that we instantiate Container as an example subtype of RegisterType
    auto containerInstance =
        RegisteredType::create("hdmf-common::Container", examplePath, io);
    REQUIRE(containerInstance != nullptr);

    // Test that we have all types registered
    auto registry = RegisteredType::getRegistry();
    auto factoryMap = RegisteredType::getFactoryMap();
    // TODO we are checking for at least 10 registered types because that is how
    // many were defined at the time of implementation of this test. We know we
    // will add more, but we would like to avoid having to update this test
    // every time, so we are only checking for at least 10
    REQUIRE(registry.size() >= 10);
    REQUIRE(factoryMap.size() >= 10);
    REQUIRE(registry.size() == factoryMap.size());

    // Test that we can indeed instantiate all registered types
    std::cout << "Registered Types:" << std::endl;
    for (const auto& entry : factoryMap) {
      const std::string& subclassFullName = entry.first;
      const std::string& typeName = entry.second.second.first;
      const std::string& typeNamespace = entry.second.second.second;

      std::cout << subclassFullName << std::endl;

      // NWBFile and ElectrodeTable enforce a specific path so we need
      // to make sure our path matches their expectations
      std::string exampleName = "";
      if (subclassFullName == "core::NWBFile") {
        examplePath = "/";
      } else if (subclassFullName == "core::ElectrodeTable") {
        examplePath = ElectrodeTable::electrodeTablePath;
        exampleName = "electrodes";
      } else {
        examplePath = "/example/path";
        exampleName = "path";
      }

      // Create the type
      auto instance = RegisteredType::create(subclassFullName, examplePath, io);
      REQUIRE(instance != nullptr);  // Check that the object was created
      // Check that the name of the type matches the classname.
      REQUIRE(instance->getTypeName() == typeName);

      // Check that the getNamespace returns the expected namespace
      REQUIRE(instance->getNamespace() == typeNamespace);

      // Check that the examplePath is set as expected
      REQUIRE(instance->getPath() == examplePath);

      // Check that the name is being computed correctly
      std::cout << instance->getName() << std::endl;
      REQUIRE(instance->getName() == exampleName);

      // Test getFullTypeName
      REQUIRE(instance->getFullTypeName() == (typeNamespace + "::" + typeName));
    }
  }

  SECTION("test create for select container")
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
    std::vector<std::string> controlDescription = {
        "c0", "c1", "c0", "c1", "c0", "c1", "c0", "c1", "c0", "c1"};
    std::string filename = getTestFilePath("testRegisteredTypeTimeseries.h5");
    std::string examplePath = "/tsdata";

    // setup timeseries object
    std::shared_ptr<BaseIO> io = createIO("HDF5", filename);
    io->open();
    // Test that create with template argument works
    std::shared_ptr<NWB::TimeSeries> ts =
        RegisteredType::create<NWB::TimeSeries>(examplePath, io);
    REQUIRE(ts != nullptr);
    ts->initialize(dataType, "unit");

    // Write data to file
    Status writeStatus = ts->writeData(
        dataShape, positionOffset, data.data(), timestamps.data());
    REQUIRE(writeStatus == Status::Success);
    io->flush();

    // Read the "namespace" attribute
    DataBlockGeneric namespaceData =
        io->readAttribute(examplePath + "/namespace");
    auto namespaceBlock = DataBlock<std::string>::fromGeneric(namespaceData);
    std::string typeNamespace = namespaceBlock.data[0];
    REQUIRE(typeNamespace == "core");

    // Read the "neurodata_type" attribute
    DataBlockGeneric typeData =
        io->readAttribute(examplePath + "/neurodata_type");
    auto typeBlock = DataBlock<std::string>::fromGeneric(typeData);
    std::string typeName = typeBlock.data[0];
    REQUIRE(typeName == "TimeSeries");

    // Combine the namespace and type name to get the full class name
    std::string fullClassName = typeNamespace + "::" + typeName;
    // Create an instance of the corresponding RegisteredType subclass
    auto readContainer =
        AQNWB::NWB::RegisteredType::create(fullClassName, examplePath, io);
    std::string containerType = readContainer->getTypeName();
    REQUIRE(containerType == "TimeSeries");

    // Open the TimeSeries container directly from file using the utility method
    // This method does the same steps as above, i.e., read the attributes and
    // then create the type from the given name
    auto readTS = AQNWB::NWB::RegisteredType::create(examplePath, io);
    std::string readTSType = readContainer->getTypeName();
    REQUIRE(readTSType == "TimeSeries");

    // Attempt to read the TimeSeries using the generic readField method
    // By providing an empty path we tell readField to read itself
    std::shared_ptr<AQNWB::NWB::RegisteredType> readRegisteredType =
        readContainer->readField(std::string(""));
    REQUIRE(readRegisteredType != nullptr);
    std::shared_ptr<AQNWB::NWB::TimeSeries> readRegisteredTypeAsTimeSeries =
        std::dynamic_pointer_cast<AQNWB::NWB::TimeSeries>(readRegisteredType);
    REQUIRE(readRegisteredTypeAsTimeSeries != nullptr);
  }

  SECTION("test error handling for invalid type creation")
  {
    std::string filename = getTestFilePath("testInvalidType.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<IO::HDF5::HDF5IO>(filename);
    std::string examplePath("/example/path");

    // Test creating with non-existent type name
    auto invalidInstance =
        RegisteredType::create("invalid::Type", examplePath, io);
    REQUIRE(invalidInstance == nullptr);

    // Test creating with empty type name
    auto emptyInstance = RegisteredType::create("", examplePath, io);
    REQUIRE(emptyInstance == nullptr);

    // Test creating with malformed type name (missing namespace)
    auto malformedInstance =
        RegisteredType::create("NoNamespace", examplePath, io);
    REQUIRE(malformedInstance == nullptr);
  }

  SECTION("test custom type name")
  {
    std::string filename = getTestFilePath("testCustomType.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<IO::HDF5::HDF5IO>(filename);
    std::string examplePath("/example/path");

    // Create instance of custom named type
    auto customInstance = std::make_shared<CustomNameType>(examplePath, io);
    REQUIRE(customInstance != nullptr);
    REQUIRE(customInstance->getTypeName() == "CustomType");
    REQUIRE(customInstance->getNamespace() == "test");
    REQUIRE(customInstance->getFullTypeName() == "test::CustomType");
  }

  SECTION("test field definitions")
  {
    std::string filename = getTestFilePath("testFields.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<IO::HDF5::HDF5IO>(filename);
    io->open();
    std::string examplePath("/test_fields");

    // Create test instance
    auto testInstance = std::make_shared<TestFieldType>(examplePath, io);
    REQUIRE(testInstance != nullptr);

    // Create parent group
    io->createGroup(examplePath);

    // Create test data
    const int32_t attrValue = 42;
    const std::vector<float> datasetValues = {1.0f, 2.0f, 3.0f};

    // Write test data
    io->createAttribute(
        BaseDataType::I32, &attrValue, examplePath, "test_attr");
    IO::ArrayDataSetConfig datasetConfig(
        BaseDataType::F32, SizeArray {3}, SizeArray {3});
    auto datasetRecordingData =
        io->createArrayDataSet(datasetConfig, examplePath + "/test_dataset");
    datasetRecordingData->writeDataBlock(
        SizeArray {3}, SizeArray {0}, BaseDataType::F32, datasetValues.data());

    // Test attribute field
    auto attrWrapper = testInstance->testAttribute();
    REQUIRE(attrWrapper != nullptr);
    auto attrData = attrWrapper->values();
    REQUIRE(attrData.data[0] == attrValue);

    // Test dataset field
    auto datasetWrapper = testInstance->testDataset();
    REQUIRE(datasetWrapper != nullptr);
    auto datasetData = datasetWrapper->values();
    REQUIRE(datasetData.data == datasetValues);

    // Test reading using the general readField method
    // Read test_attr via readField
    auto attrWrapper2 = testInstance->readField<AttributeField, int32_t>(
        std::string("test_attr"));
    REQUIRE(attrWrapper2 != nullptr);
    auto attrData2 = attrWrapper2->values();
    REQUIRE(attrData2.data[0] == attrValue);

    // Read test_dataset via readField
    auto datasetWrapper2 = testInstance->readField<DatasetField, float>(
        std::string("test_dataset"));
    REQUIRE(datasetWrapper2 != nullptr);
    auto datasetData2 = datasetWrapper2->values();
    REQUIRE(datasetData2.data == datasetValues);

    io->close();
  }
}
