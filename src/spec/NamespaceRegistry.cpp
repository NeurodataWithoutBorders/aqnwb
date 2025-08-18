#include "NamespaceRegistry.hpp"

namespace AQNWB::SPEC
{

NamespaceRegistry& NamespaceRegistry::instance()
{
  static NamespaceRegistry registry;
  return registry;
}

void NamespaceRegistry::registerNamespace(const std::string& name,
                                          const Types::NamespaceInfo& info)
{
  registry_[name] = info;
}

const Types::NamespaceInfo* NamespaceRegistry::getNamespaceInfo(
    const std::string& name) const
{
  auto it = registry_.find(name);
  if (it != registry_.end()) {
    return &it->second;
  }
  return nullptr;
}

const NamespaceRegistry::RegistryType& NamespaceRegistry::getAllNamespaces()
    const
{
  return registry_;
}

}  // namespace AQNWB::SPEC
