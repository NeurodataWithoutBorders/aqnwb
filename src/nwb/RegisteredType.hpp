#pragma once

#include <filesystem>
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
  RegisteredType(const std::string& path,
                 std::shared_ptr<AQNWB::IO::BaseIO> io);

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
   * @brief Get the name of the object
   *
   * This is the last part of getPath(), much like the filename portion
   * of a file system path.
   * @return String with the name of the object
   */
  inline std::string getName() const
  {
    return std::filesystem::path(m_path).filename().string();
  }

  /**
   * @brief Get a shared pointer to the IO object.
   * @return Shared pointer to the IO object.
   */
  inline std::shared_ptr<AQNWB::IO::BaseIO> getIO() const { return m_io; }

  /**
   * @brief Clear the BaseRecordingData object cache to reset the recording
   * state
   */
  inline void clearRecordingDataCache() { this->m_recordingDataCache.clear(); }

  /**
   * @brief Get the cache of BaseRecordingData objects
   * @return A reference to the cache of BaseRecordingData objects
   */
  inline const std::unordered_map<std::string,
                                  std::shared_ptr<IO::BaseRecordingData>>&
  getCacheRecordingData() const
  {
    return this->m_recordingDataCache;
  }

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
                    const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>,
                std::pair<std::string, std::string>>>&
  getFactoryMap();

  /**
   * @brief Create an instance of a registered subclass by name.
   *
   * @param fullClassName The combined namespace and class name to instantiate,
   * i.e., namespace::class
   * @param path The path of the registered type.
   * @param io A shared pointer to the IO object.
   * @param fallbackToBase If true, return a base class instance if the
   *        full class is not found. The base class to use is defined by
   *        m_defaultUnregisteredGroupTypeClass and
   * m_defaultUnregisteredDatasetTypeClass depending on whether the type is a
   * group or dataset.
   * @return A unique_ptr to the created instance of the subclass, or nullptr if
   * the subclass is not found.
   */
  static std::shared_ptr<RegisteredType> create(
      const std::string& fullClassName,
      const std::string& path,
      std::shared_ptr<IO::BaseIO> io,
      bool fallbackToBase = false);

  /**
   * @brief Factory method to create an instance of a subclass of RegisteredType
   * from file
   *
   * The function: 1) reads the  "namespace" and "neurodata_type" attributes at
   * the given path, 2) looks up the corresponding subclass of  RegisteredType
   * for that type in the type registry 3) instantiates the subclass to
   * represent the object at the path.
   *
   * @param path The path of the registered type.
   * @param io A shared pointer to the IO object.
   * @param fallbackToBase If true, return a base class instance if the
   *        full class is not found. The base class to use is defined by
   *        m_defaultUnregisteredGroupTypeClass and
   * m_defaultUnregisteredDatasetTypeClass depending on whether the type is a
   * group or dataset.
   *
   * @return A unique pointer to the created RegisteredType instance, or nullptr
   * if creation fails.
   */
  static std::shared_ptr<AQNWB::NWB::RegisteredType> create(
      const std::string& path,
      std::shared_ptr<IO::BaseIO> io,
      bool fallbackToBase = false);

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
                                          std::shared_ptr<AQNWB::IO::BaseIO> io)
  {
    static_assert(std::is_base_of<RegisteredType, T>::value,
                  "T must be a derived class of RegisteredType");
    return std::shared_ptr<T>(new T(path, io));
  }

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
   * @brief Get the full name of the type, i.e., `namespace::typename`
   *
   * This is just a simple convenience function that uses the getNamespace
   * and getTypeName methods.
   *
   * @return The full name of the type consisting of `namespace::typename`
   */
  inline std::string getFullTypeName() const
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
   * parameters needed by this function.
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
  inline std::unique_ptr<AQNWB::IO::ReadDataWrapper<SOT, VTYPE>> readField(
      const std::string& fieldPath) const
  {
    return std::make_unique<AQNWB::IO::ReadDataWrapper<SOT, VTYPE>>(
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
      const std::string& fieldPath) const
  {
    return this->create(AQNWB::mergePaths(m_path, fieldPath), m_io);
  }

  /**
   * @brief Find all typed objects that are owned by this object, i.e.,
   * objects that have a neurodata_type and namespace attribute and have
   * this object as there closest parent with an assigned type.
   *
   * This is a shorthand for calling
   * `getIO()->findTypes(m_path, types, IO::SearchMode::STOP_ON_TYPE, true);`
   *
   * @param types The set of types to search for. If an empty set is provided,
   * then all objects with an assigned type (i.e., object that have a
   * neurodata_type and namespace attributed) will be returned.
   * @param search_mode The search mode to use. By default
   * IO::SearchMode::STOP_ON_TYPE is used to only retrieve objects that are
   * owned by this object. To recursively search though all types nested within
   * the object set to IO::SearchMode::CONTINUE_ON_TYPE
   * @return An unordered map where each key is the path to an object and its
   * corresponding value is the type of the object.
   */
  virtual std::unordered_map<std::string, std::string> findOwnedTypes(
      const std::unordered_set<std::string>& types = {},
      const AQNWB::IO::SearchMode& search_mode =
          AQNWB::IO::SearchMode::STOP_ON_TYPE) const;

protected:
  /// @brief Save the default RegisteredType to use for reading Group types that
  /// are not registered
  static const std::string m_defaultUnregisteredGroupTypeClass;

  /// @brief Save the default RegisteredType to use for reading Dataset types
  /// that are not registered
  static const std::string m_defaultUnregisteredDatasetTypeClass;

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
          const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>
          factoryFunction,
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

  /**
   * @brief Cache for BaseRecordingData objects for datasets to retain recording
   * state.
   *
   * This map stores shared pointers to BaseRecordingData objects that have been
   * previously requested, using the field path as the key. This allows us to
   * reuse the same object when it is requested multiple times, improving
   * performance and more importantly, retaining the recording position so that
   * we can append to the dataset from the last position that we recorded to.
   * This is important for writing data to the dataset in a streaming fashion.
   * The cache is mutable to allow modification in const methods.
   */
  std::unordered_map<std::string, std::shared_ptr<IO::BaseRecordingData>>
      m_recordingDataCache;
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

 * @param NAMESPACE_VAR The namespace of the subclass type in the format schema.
 * May be specified via a const variable or as a literal string.
 * @param TYPENAME The name of the type (usually the class name).
 */
#define REGISTER_SUBCLASS_WITH_TYPENAME(T, NAMESPACE_VAR, TYPENAME) \
  static bool registerSubclass() \
  { \
    AQNWB::NWB::RegisteredType::registerSubclass( \
        std::string(NAMESPACE_VAR) + "::" + #T, \
        [](const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io) \
            -> std::unique_ptr<AQNWB::NWB::RegisteredType> \
        { return std::make_unique<T>(path, io); }, \
        TYPENAME, \
        NAMESPACE_VAR); \
    return true; \
  } \
  static bool registered_; \
  virtual std::string getTypeName() const override \
  { \
    return TYPENAME; \
  } \
  virtual std::string getNamespace() const override \
  { \
    return NAMESPACE_VAR; \
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
 * @brief Defines a lazy-loaded attribute field accessor function.
 *
 * This macro generates a function that returns a lazy-loaded wrapper for an
 * attribute field.
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
 * @param default_type The default type of the field.
 * @param fieldPath The path to the field.
 * @param description A detailed description of the field.
 */
#define DEFINE_ATTRIBUTE_FIELD(name, default_type, fieldPath, description) \
  /** \
   * @brief Returns a lazy-loaded wrapper for the ##name attribute field. \
   * \
   * @tparam VTYPE The type of the field (default: ##default_type) \
   * @return A unique pointer to a ReadDataWrapper for the field \
   * \
   * description \
   */ \
  template<typename VTYPE = default_type> \
  inline std::unique_ptr<AQNWB::IO::ReadDataWrapper<AttributeField, VTYPE>> \
  name() const \
  { \
    return std::make_unique< \
        AQNWB::IO::ReadDataWrapper<AttributeField, VTYPE>>( \
        m_io, AQNWB::mergePaths(m_path, fieldPath)); \
  }

/**
 * @brief Defines a lazy-loaded dataset field accessor function.
 *
 * This macro generates two functions:
 * 1. A read function that returns a lazy-loaded wrapper for a dataset field
 * 2. A write function that returns the dataset object directly
 *
 * \note
 * The Doxyfile.in defines a simplified expansion of this function
 * for generating the documentation for the autogenerated function.
 * This means: 1) When updating the macro here, we also need to ensure
 * that the expansion in the Doxyfile.in is still accurate and 2) the
 * docstring that is defined by the macro here is not being used by
 * Doxygen but the version generated by its on PREDEFINED expansion.
 *
 * @param readName The name of the read function to generate.
 * @param writeName The name of the write function to generate.
 * @param default_type The default type of the field.
 * @param fieldPath The path to the field.
 * @param description A detailed description of the field.
 */
#define DEFINE_DATASET_FIELD( \
    readName, writeName, default_type, fieldPath, description) \
  /** \
   * @brief Returns a lazy-loaded wrapper for the ##readName dataset field. \
   * \
   * @tparam VTYPE The type of the field (default: ##default_type) \
   * @return A unique pointer to a ReadDataWrapper for the field \
   * \
   * description \
   */ \
  template<typename VTYPE = default_type> \
  inline std::unique_ptr<IO::ReadDataWrapper<DatasetField, VTYPE>> readName() \
      const \
  { \
    return std::make_unique<IO::ReadDataWrapper<DatasetField, VTYPE>>( \
        m_io, AQNWB::mergePaths(m_path, fieldPath)); \
  } \
  /** \
   * @brief Returns the dataset object for the ##writeName field. \
   * \
   * This functions modifies the m_recordingDataCache as a side effect \
   * to retain the recording state. \
   * \
   * @param reset If true, the dataset will be reset to the beginning \
   *        by creating a new BaseRecordingData object via m_io->getDataSet \
   * \
   * @return A shared pointer to a BaseRecordingData for the dataset \
   * \
   * description \
   */ \
  inline std::shared_ptr<IO::BaseRecordingData> writeName(bool reset = false) \
  { \
    std::string fullPath = AQNWB::mergePaths(m_path, fieldPath); \
    if (!reset) { \
      /* Check if the dataset is already in the cache */ \
      auto it = m_recordingDataCache.find(fullPath); \
      if (it != m_recordingDataCache.end()) { \
        return it->second; \
      } \
    } \
    /* Get the dataset from IO and cache it */ \
    auto dataset = m_io->getDataSet(fullPath); \
    if (dataset) { \
      m_recordingDataCache[fullPath] = dataset; \
    } \
    return dataset; \
  }

/**
 * @brief Defines a lazy-loaded accessor function for reading fields that are
 * RegisteredTypes
 *
 * This macro generates a function that returns the approbriate subtype of
 * RegisteredType, e.g., to read VectorData from a DynamicTable or a
 * TimeSeries from and NWBFile.
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
 * @param registeredType The specific subclass of registered type to use
 * @param fieldPath The path to the field.
 * @param description A detailed description of the field.
 */
#define DEFINE_REGISTERED_FIELD(name, registeredType, fieldPath, description) \
  /** \
   * @brief Returns the instance of the class representing the ##name field. \
   * \
   * @tparam RTYPE The RegisteredType of the field (default: ##registeredType) \
   * In most cases this should not be changed. But in the case of templated \
   * types, e.g,. VectorData<std::any> a user may want to change this to a \
   * more specific subtype to use, e.g., VectorData<int> \
   * @return A shared pointer to an instance of ##registeredType representing \
   * the object. May return nullptr if the path does not exist \
   * \
   * description \
   */ \
  template<typename RTYPE = registeredType> \
  inline std::shared_ptr<RTYPE> name() const \
  { \
    std::string objectPath = AQNWB::mergePaths(m_path, fieldPath); \
    if (m_io->objectExists(objectPath)) { \
      return RegisteredType::create<RTYPE>(objectPath, m_io); \
    } \
    return nullptr; \
  }

/**
 * @brief Defines a lazy-loaded accessor function for reading fields that are
 * RegisteredTypes that are linked to by a reference attribute
 *
 * This macro generates a function that returns the appropriate subtype of
 * RegisteredType, e.g., to read VectorData from a DynamicTable or a
 * TimeSeries from an NWBFile.
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
 * @param registeredType The specific subclass of registered type to use
 * @param fieldPath The path to the attribute that stores reference to the field
 * @param description A detailed description of the field.
 */
#define DEFINE_REFERENCED_REGISTERED_FIELD( \
    name, registeredType, fieldPath, description) \
  /** \
   * @brief Returns the instance of the class representing the ##name field. \
   * \
   * @tparam RTYPE The RegisteredType of the field (default: ##registeredType) \
   * In most cases this should not be changed. But in the case of templated \
   * types, e.g,. VectorData<std::any> a user may want to change this to a \
   * more specific subtype to use, e.g., VectorData<int> \
   * @return A shared pointer to an instance of ##registeredType representing \
   * the object. May return nullptr if the path does not exist \
   * \
   * description \
   */ \
  template<typename RTYPE = registeredType> \
  inline std::shared_ptr<RTYPE> name() const \
  { \
    try { \
      std::string attrPath = AQNWB::mergePaths(m_path, fieldPath); \
      std::string objectPath = m_io->readReferenceAttribute(attrPath); \
      if (m_io->objectExists(objectPath)) { \
        return RegisteredType::create<RTYPE>(objectPath, m_io); \
      } \
    } catch (const std::exception& e) { \
      return nullptr; \
    } \
    return nullptr; \
  }

}  // namespace NWB
}  // namespace AQNWB
