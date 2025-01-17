#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB::NWB;

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
    // many
    //      were defined at the time of implementation of this test. We know we
    //      will add more, but we would like to avoid having to update this test
    //      every time, so we are only checking for at least 10
    REQUIRE(registry.size() >= 10);
    REQUIRE(factoryMap.size() >= 10);
    REQUIRE(registry.size() == factoryMap.size());

    // Test that we can indeed instantiate all registered types
    // This also ensures that each factory function works correctly,
    // and hence, that  all subtypes implement the expected constructor
    // for the RegisteredType::create method. This is similar to
    // checking:
    // for (const auto& pair : factoryMap) {
    //    auto instance = pair.second(examplePath, io);
    //    REQUIRE(instance != nullptr);
    // }
    std::cout << "Registered Types:" << std::endl;
    for (const auto& entry : factoryMap) {
      const std::string& subclassFullName = entry.first;
      const std::string& typeName = entry.second.second.first;
      const std::string& typeNamespace = entry.second.second.second;

      std::cout << subclassFullName << std::endl;

      // NWBFile and ElectrodeTable enforce a specific path so we need
      // to make sure our path matches their expectations
      if (subclassFullName == "core::NWBFile") {
        examplePath = "/";
      } else if (subclassFullName == "core::ElectrodeTable") {
        examplePath = ElectrodeTable::electrodeTablePath;
      } else {
        examplePath = "/example/path";
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
  }
}
