#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"

namespace AQNWB
{
namespace NWB
{

/**
 * @brief Alias for AQNWB::Types::StorageObjectType::Attribute
 */
constexpr auto AttributeField = AQNWB::Types::StorageObjectType::Attribute;

/**
 * @brief Alias for AQNWB::Types::StorageObjectType::Dataset
 */
constexpr auto DatasetField = AQNWB::Types::StorageObjectType::Dataset;

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
  inline std::string getPath() const { return m_path; }

  /**
   * @brief Get a shared pointer to the IO object.
   * @return Shared pointer to the IO object.
   */
  inline std::shared_ptr<IO::BaseIO> getIO() const { return m_io; }

  /**
   * @brief Get the registry of subclass names.
   *
   * The registry is a function-local static variable, which means it
   * is lazily initialized on the first call to this function and
   * persists for the duration of the program. This implementation
   * provides thread-safety and avoids the static initialization order fiasco.
   *
   * @return A reference to an unordered_set containing the names of all
   * registered subclasses.
   */
  static std::unordered_set<std::string>& getRegistry();

  /**
   * @brief Get the factory map for creating instances of subclasses.
   *
   * The factory map is a function-local static variable, which means it
   * is lazily initialized on the first call to this function and
   * persists for the duration of the program. This implementation
   * provides thread-safety and avoids the static initialization order fiasco.
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

  /**
   * @brief Get the full name of type, i.e., `namespace::typename`
   *
   * This is just a simple convenience function that uses the getNamespace
   * and getTypeName methods.
   *
   * @return The full name of the type consisting of `namespace::typename`
   */
  inline std::string getFullName() const
  {
    return (getNamespace() + "::" + getTypeName());
  }

  /**
   * @brief Support reading of arbitrary fields by their relative path
   *
   * This function provided as a general "backup" to support reading of
   * arbitrary fields even if the sub-class may not have an explicit
   * DEFINE_FIELD specified for the field. If a DEFINE_FIELD exists then
   * the corresponding read method should be used as it avoids the need
   * for specifying most (if not all) of the function an template
   * parameteres needed by this function.
   *
   * @param fieldPath The relative path of the field within the current type,
   * i.e., relative to `m_path`
   * @tparam SOT The storage object type. This must be a either
   * StorageObjectType::Dataset or StorageObjectType::Attribute
   * @tparam VTYPE The value type of the field to be read.
   * @tparam Enable SFINAE (Substitution Failure Is Not An Error) mechanism
   * to enable this function only if SOT is a Dataset or Attribute.
   *
   * @return ReadDataWrapper object for lazy reading of the field
   */
  template<StorageObjectType SOT,
           typename VTYPE,
           typename std::enable_if<Types::IsDataStorageObjectType<SOT>::value,
                                   int>::type = 0>
  inline std::unique_ptr<IO::ReadDataWrapper<SOT, VTYPE>> readField(
      const std::string& fieldPath) const
  {
    return std::make_unique<IO::ReadDataWrapper<SOT, VTYPE>>(
        m_io, AQNWB::mergePaths(m_path, fieldPath));
  }

  /**
   * @brief Read a field that is itself a RegisteredType
   *
   * @param fieldPath The relative path of the field within the current type,
   * i.e., relative to `m_path. The field must itself be RegisteredType
   *
   * @return A unique_ptr to the created instance of the subclass.
   */
  inline std::shared_ptr<AQNWB::NWB::RegisteredType> readField(
      const std::string& fieldPath)
  {
    return this->create(AQNWB::mergePaths(m_path, fieldPath), m_io);
  }

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
 * @param TYPENAME The name of the type (usually the class name).
 */
#define REGISTER_SUBCLASS_WITH_TYPENAME(T, NAMESPACE, TYPENAME) \
  static bool registerSubclass() \
  { \
    AQNWB::NWB::RegisteredType::registerSubclass( \
        NAMESPACE "::" #T, \
        [](const std::string& path, std::shared_ptr<IO::BaseIO> io) \
            -> std::unique_ptr<AQNWB::NWB::RegisteredType> \
        { return std::make_unique<T>(path, io); }, \
        TYPENAME, \
        NAMESPACE); \
    return true; \
  } \
  static bool registered_; \
  virtual std::string getTypeName() const override \
  { \
    return TYPENAME; \
  } \
  virtual std::string getNamespace() const override \
  { \
    return NAMESPACE; \
  }

/**
 * @brief Macro to register a subclass with the RegisteredType class registry.
 *
 * This macro is a convenience wrapper around the main REGISTER_SUBCLASS macro,
 * providing a default value for TYPENAME.
 *
 * @param T The subclass type to register. The name must match the type in the
 * schema.
 * @param NAMESPACE The namespace of the subclass type in the format schema
 */
#define REGISTER_SUBCLASS(T, NAMESPACE) \
  REGISTER_SUBCLASS_WITH_TYPENAME(T, NAMESPACE, #T)

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

/**
 * @brief Defines a lazy-loaded field accessor function.
 *
 * This macro generates a function that returns a lazy-loaded wrapper for a
 * field.
 *
 * \note
 * The Doxyfile.in defines a simplified expansion of this function
 * for generating the documentation for the autogenerated function.
 * This means: 1) When updating the macro here, we also need to ensure
 * that the expansion in the Doxyfile.in is still accurate and 2) the
 * docstring that is defined by the macro here is not being used by
 * Doxygen but the version generated by its on PREDEFINED expansion.
 *
 * @param name The name of the function to generate.
 * @param storageObjectType The type of storage object (Attribute or Dataset).
 * @param default_type The default type of the field.
 * @param fieldPath The path to the field.
 * @param description A detailed description of the field.
 */
#define DEFINE_FIELD( \
    name, storageObjectType, default_type, fieldPath, description) \
  /** \
   * @brief Returns a lazy-loaded wrapper for the ##name field. \
   * \
   * @tparam VTYPE The type of the field (default: ##default_type) \
   * @return A unique pointer to a ReadDataWrapper for the field \
   * \
   * description \
   */ \
  template<typename VTYPE = default_type> \
  inline std::unique_ptr<IO::ReadDataWrapper<storageObjectType, VTYPE>> name() \
      const \
  { \
    return std::make_unique<IO::ReadDataWrapper<storageObjectType, VTYPE>>( \
        m_io, AQNWB::mergePaths(m_path, fieldPath)); \
  }

}  // namespace NWB
}  // namespace AQNWB
