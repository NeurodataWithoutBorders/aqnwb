#include <iostream>

#include <catch2/catch_all.hpp>

#include "spec/NamespaceRegistry.hpp"
#include "spec/core.hpp"
#include "spec/hdmf_common.hpp"
#include "spec/hdmf_experimental.hpp"

using namespace AQNWB::SPEC;

TEST_CASE("Test NamespaceRegistry", "[NamespaceRegistry]")
{
  const auto& allNamespaces = NamespaceRegistry::instance().getAllNamespaces();

  SECTION("Check total number of registered namespaces")
  {
    REQUIRE(allNamespaces.size()
            == 3);  // We expect 3 namespaces to be registered
  }

  SECTION("Check that all expected namespaces are registered")
  {
    REQUIRE(allNamespaces.find("core") != allNamespaces.end());
    REQUIRE(allNamespaces.find("hdmf-common") != allNamespaces.end());
    REQUIRE(allNamespaces.find("hdmf-experimental") != allNamespaces.end());
  }

  SECTION("Test retrieving all expected namespaces")
  {
    const NamespaceInfo* coreInfo =
        NamespaceRegistry::instance().getNamespaceInfo("core");
    REQUIRE(coreInfo != nullptr);
    REQUIRE(coreInfo->specVariables.size() > 1);

    const NamespaceInfo* hdmfCommonInfo =
        NamespaceRegistry::instance().getNamespaceInfo("hdmf-common");
    REQUIRE(hdmfCommonInfo != nullptr);
    REQUIRE(hdmfCommonInfo->specVariables.size() > 1);

    const NamespaceInfo* hdmfExperimentalInfo =
        NamespaceRegistry::instance().getNamespaceInfo("hdmf-experimental");
    REQUIRE(hdmfExperimentalInfo != nullptr);
    REQUIRE(hdmfExperimentalInfo->specVariables.size() > 1);
  }

  SECTION("Test that non-existent namespace returns nullptr")
  {
    const NamespaceInfo* info =
        NamespaceRegistry::instance().getNamespaceInfo("NonExistentNamespace");
    REQUIRE(info == nullptr);  // Check that the namespace was not found
  }
}
