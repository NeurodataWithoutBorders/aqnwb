#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "io/BaseIO.hpp"

namespace AQNWB
{
namespace NWB
{

/**
 * @brief Base class for types defined in the NWB schema
 *
 * This class maintains a static registry of all subclasses that inherit from
 * it. To register a new class we need to call the macro
 * "REGISTER_SUBCLASS(MySubClass)" in the definition of the subclass. All
 * subclasses must also implement the constructor with the ``path`` and ``io``
 * as sole input arguments. This in turn allows us to create any subclass of
 * container based on its name because we can : 1) look-up classes by their name
 * in the registry and 2) all subclasses have a consistent constructor. The
 * registry is static in the sense that it is a static member of the Container
 * class (and as such it is shared among all instances of Container and its
 * subclasses). However, the registry is not static in the sense that it is
 * being created at compile time; but it is created and can be modified at
 * runtime.
 *
 */
class RegisteredType
{
public:
  /**
   * @brief Constructor.
   *
   * All registered subclasses of RegisteredType must implement a constructor
   * with these arguments.
   *
   * @param path The path of the registered type.
   * @param io A shared pointer to the IO object.
   */
  RegisteredType(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor.
   */
  virtual ~RegisteredType();

  /**
   * @brief Gets the path of the registered type.
   * @return The path of the registered type.
   */
  std::string getPath() const;

  /**
   * @brief Get the registry of subclass names.
   *
   * @return A reference to an unordered_set containing the names of all
   * registered subclasses.
   */
  static std::unordered_set<std::string>& getRegistry();

  /**
   * @brief Get the factory map for creating instances of subclasses.
   *
   * @return A reference to an unordered_map containing factory functions for
   * registered subclasses.
   */
  static std::unordered_map<
      std::string,
      std::pair<std::function<std::unique_ptr<RegisteredType>(
                    const std::string&, std::shared_ptr<IO::BaseIO>)>,
                std::pair<std::string, std::string>>>&
  getFactoryMap();

  /**
   * @brief Create an instance of a registered subclass by name.
   *
   * @param fullClassName The combined namespace and class name to instantiate,
   * i.e., namespace::class
   * @param path The path of the registered type.
   * @param io A shared pointer to the IO object.
   * @return A unique_ptr to the created instance of the subclass, or nullptr if
   * the subclass is not found.
   */
  static inline std::shared_ptr<RegisteredType> create(
      const std::string& fullClassName,
      const std::string& path,
      std::shared_ptr<IO::BaseIO> io)
  {
    auto it = getFactoryMap().find(fullClassName);
    if (it != getFactoryMap().end()) {
      return it->second.first(path, io);
    }
    return nullptr;
  }

  /**
   * @brief Factory method to create an instance of a subclass of RegisteredType
   * by type.
   *
   * @tparam T The subclass of RegisteredType to instantiate.
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   * @return A unique_ptr to the created instance of the subclass.
   */
  template<typename T>
  static inline std::shared_ptr<T> create(const std::string& path,
                                          std::shared_ptr<IO::BaseIO> io)
  {
    static_assert(std::is_base_of<RegisteredType, T>::value,
                  "T must be a derived class of RegisteredType");
    return std::shared_ptr<T>(new T(path, io));
  }

  /**
   * @brief Factory method to create an instance of a subclass of RegisteredType
   * from file
   *
   * The function: 1) reads the  "namespace" and "neurodata_type" attributes at
   * the given path, 2) looks up the corresponding subclass of  RegisteredType
   * for that type in the type registry 3) instantiates the subclass to
   * represent the object at the path.
   *
   * @return A unique pointer to the created RegisteredType instance, or nullptr
   * if creation fails.
   */
  static std::shared_ptr<AQNWB::NWB::RegisteredType> create(
      const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Get the name of the class type.
   *
   * The REGISTER_SUBCLASS macro defines an  automatic override
   * for this function to return the unmangled name of the class.
   * The name  of the class must be the same as the neurodata_type
   * that it implements.
   *
   * @return The name of the type as a string
   */
  virtual std::string getTypeName() const;

  /**
   * @brief Get the schema namespace of the class type.
   *
   * This is the namespace of the neurodata_type in the format
   * schema and NOT the namespace of the class in C++.
   * The REGISTER_SUBCLASS macro defines an  automatic override
   * for this function to return the namespace as defined when
   * the class was registered.
   *
   * @return The namespace of the type as a string.
   */
  virtual std::string getNamespace() const;

protected:
  /**
   * @brief Register a subclass name and its factory function in the registry.
   *
   * @param fullClassName The combined namespace and class name to register.
   * @param factoryFunction The factory function to create instances of the
   * subclass.
   * @param typeName The name of the type (usually the class name).
   * @param typeNamespace The namespace of the type.
   */
  static void registerSubclass(
      const std::string& fullClassName,
      std::function<std::unique_ptr<RegisteredType>(
          const std::string&, std::shared_ptr<IO::BaseIO>)> factoryFunction,
      const std::string& typeName,
      const std::string& typeNamespace);

  /**
   * @brief The path of the registered type.
   */
  std::string m_path;

  /**
   * @brief A shared pointer to the IO object.
   */
  std::shared_ptr<IO::BaseIO> m_io;
};

/**
 * @brief Macro to register a subclass with the RegisteredType class registry.
 *
 * This macro defines:
 * - A static method `registerSubclass` that triggers registration of the
 * subclass type when the subclass type is loaded.
 * - A static member `registered_` that ensures the registration occurs.
 * - override getTypeName for the class to return the correct type name
 * - override getNamespace for the class to return the correct namespace used
 *
 * @param T The subclass type to register. The name must match the type in the
 * schema.
 * @param NAMESPACE The namespace of the subclass type in the format schema
 */
#define REGISTER_SUBCLASS(T, NAMESPACE) \
  static bool registerSubclass() \
  { \
    AQNWB::NWB::RegisteredType::registerSubclass( \
        NAMESPACE "::" #T, \
        [](const std::string& path, std::shared_ptr<IO::BaseIO> io) \
            -> std::unique_ptr<AQNWB::NWB::RegisteredType> \
        { return std::make_unique<T>(path, io); }, \
        #T, \
        NAMESPACE); \
    return true; \
  } \
  static bool registered_; \
  virtual std::string getTypeName() const override \
  { \
    return #T; \
  } \
  virtual std::string getNamespace() const override \
  { \
    return NAMESPACE; \
  }

/**
 * @brief Macro to initialize the static member `registered_` to trigger
 * registration.
 *
 * This macro ensures that the registration of the subclass occurs when the
 * program starts.
 *
 * @param T The subclass type to register.
 */
#define REGISTER_SUBCLASS_IMPL(T) bool T::registered_ = T::registerSubclass();

}  // namespace NWB
}  // namespace AQNWB
