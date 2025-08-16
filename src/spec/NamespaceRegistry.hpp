#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Types.hpp"

/*!
 * \namespace AQNWB::SPEC
 * \brief The namespace for managing format schema and namespaces.
 */
namespace AQNWB::SPEC
{
/**
 * @brief Singleton class to manage the registration and lookup of namespace
 * information.
 */
class NamespaceRegistry
{
public:
  using RegistryType = std::unordered_map<std::string, Types::NamespaceInfo>;

  /**
   * @brief Get the singleton instance of the NamespaceRegistry.
   * @return Reference to the NamespaceRegistry instance.
   */
  static NamespaceRegistry& instance();

  /**
   * @brief Register a namespace with its information.
   * @param name The name of the namespace.
   * @param info The NamespaceInfo struct containing the namespace information.
   */
  void registerNamespace(const std::string& name,
                         const Types::NamespaceInfo& info);

  /**
   * @brief Get the namespace information for a given namespace name.
   * @param name The name of the namespace.
   * @return Pointer to the NamespaceInfo struct, or nullptr if the namespace is
   * not found.
   */
  const Types::NamespaceInfo* getNamespaceInfo(const std::string& name) const;

  /**
   * @brief Get all registered namespaces.
   * @return Reference to the registry containing all NamespaceInfo structs.
   */
  const RegistryType& getAllNamespaces() const;

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