#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace AQNWB::SPEC
{

/**
 * @brief Struct to hold namespace information.
 */
struct NamespaceInfo
{
  std::string name;  ///< The name of the namespace.
  std::string version;  ///< The version of the namespace.
  std::vector<std::pair<std::string_view, std::string_view>>
      specVariables;  ///< The specVariables of the namespace.
};

/**
 * @brief Singleton class to manage the registration and lookup of namespace
 * information.
 */
class NamespaceRegistry
{
public:
  using RegistryType = std::unordered_map<std::string, NamespaceInfo>;

  /**
   * @brief Get the singleton instance of the NamespaceRegistry.
   * @return Reference to the NamespaceRegistry instance.
   */
  static NamespaceRegistry& instance()
  {
    static NamespaceRegistry registry;
    return registry;
  }

  /**
   * @brief Register a namespace with its information.
   * @param name The name of the namespace.
   * @param info The NamespaceInfo struct containing the namespace information.
   */
  void registerNamespace(const std::string& name, const NamespaceInfo& info)
  {
    registry_[name] = info;
  }

  /**
   * @brief Get the namespace information for a given namespace name.
   * @param name The name of the namespace.
   * @return Pointer to the NamespaceInfo struct, or nullptr if the namespace is
   * not found.
   */
  const NamespaceInfo* getNamespaceInfo(const std::string& name) const
  {
    auto it = registry_.find(name);
    if (it != registry_.end()) {
      return &it->second;
    }
    return nullptr;
  }

  /**
   * @brief Get all registered namespaces.
   * @return Reference to the registry containing all NamespaceInfo structs.
   */
  const RegistryType& getAllNamespaces() const { return registry_; }

private:
  RegistryType registry_;  ///< The registry storing namespace information.
};

/**
 * @brief Macro to register a namespace with the global registry.
 * @param name The name of the namespace.
 * @param version The version of the namespace.
 * @param specVariables The specVariables of the namespace.
 */
#define REGISTER_NAMESPACE(name, version, specVariables) \
  namespace \
  { \
  struct NamespaceRegistrar \
  { \
    NamespaceRegistrar() \
    { \
      NamespaceRegistry::instance().registerNamespace( \
          name, {name, version, specVariables}); \
    } \
  }; \
  static NamespaceRegistrar registrar; \
  }

}  // namespace AQNWB::SPEC