#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/RegisteredType.hpp"
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
        RegisteredType::create("Container", examplePath, io);
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
    /*
    std::cout<<"Registered Types:"<<std::endl;
    for (const auto& subclassName : registry) {
      std::cout<<subclassName<<std::endl;
      // NWBFile and ElectrodeTable enforce a specific path so we need
      // to make sure our path matches their expectations
      if (subclassName == "NWBFile")
      {
         examplePath = "/";
      }else if (subclassName == "ElectrodeTable"){
         examplePath = ElectrodeTable::electrodeTablePath;
      }
      else{
         examplePath = "/example/path";
      }
      std::string expectedTypeName = subclassName;
      // Create the type
      auto instance = RegisteredType::create(subclassName, examplePath, io);
      REQUIRE(instance != nullptr);  // Check that the object was created
      // Check that the name of the type matches the classname.
      // NOTE: Currently the expected typename is always the classname but
      //       if a type overwrites the getTypeName method than it could be
    different REQUIRE(instance->getTypeName() == expectedTypeName);
      // Check that the examplePath is set as expected
      REQUIRE(instance->getPath() == examplePath);
    }*/
  }
}
