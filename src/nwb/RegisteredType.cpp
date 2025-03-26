#include "RegisteredType.hpp"

#include "Utils.hpp"
#include "io/ReadIO.hpp"

using namespace AQNWB::NWB;

/// Set the default RegisteredType to use for unregistered Group and Dataset
/// types
const std::string RegisteredType::m_defaultUnregisteredGroupTypeClass =
    "hdmf-common::Container";
const std::string RegisteredType::m_defaultUnregisteredDatasetTypeClass =
    "hdmf-common::Data";

RegisteredType::RegisteredType(const std::string& path,
                               std::shared_ptr<AQNWB::IO::BaseIO> io)
    : m_path(path)
    , m_io(io)
{
}

RegisteredType::~RegisteredType() {}

std::unordered_set<std::string>& RegisteredType::getRegistry()
{
  static std::unordered_set<std::string> registry;
  return registry;
}

std::unordered_map<
    std::string,
    std::pair<std::function<std::unique_ptr<RegisteredType>(
                  const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>,
              std::pair<std::string, std::string>>>&
RegisteredType::getFactoryMap()
{
  static std::unordered_map<
      std::string,
      std::pair<std::function<std::unique_ptr<RegisteredType>(
                    const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>,
                std::pair<std::string, std::string>>>
      factoryMap;
  return factoryMap;
}

void RegisteredType::registerSubclass(
    const std::string& fullClassName,
    std::function<std::unique_ptr<RegisteredType>(
        const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>
        factoryFunction,
    const std::string& typeName,
    const std::string& typeNamespace)
{
  getRegistry().insert(fullClassName);
  getFactoryMap()[fullClassName] = {factoryFunction, {typeName, typeNamespace}};
}

// getTypeName has automatic override for subclasses in the REGISTER_SUBCLASS
// macro
std::string RegisteredType::getTypeName() const
{
  return "";
}

// getNamespace has automatic override for subclasses in the REGISTER_SUBCLASS
// macro
std::string RegisteredType::getNamespace() const
{
  return "";
}

std::shared_ptr<AQNWB::NWB::RegisteredType> RegisteredType::create(
    const std::string& fullClassName,
    const std::string& path,
    std::shared_ptr<IO::BaseIO> io,
    bool fallbackToBase)
{
  //  Look up the factory RegisteredType for the fullClassName the registry
  auto it = getFactoryMap().find(fullClassName);
  if (it != getFactoryMap().end()) {
    return it->second.first(path, io);
  }
  // If the class is not found, return a base class instance by calling this
  // function again with the fallback base class to use for Group and Dataset
  // types respectively
  if (fallbackToBase) {
    StorageObjectType sot = io->getStorageObjectType(path);
    if (sot == StorageObjectType::Group) {
      return create(m_defaultUnregisteredGroupTypeClass, path, io);
    } else if (sot == StorageObjectType::Dataset) {
      return create(m_defaultUnregisteredDatasetTypeClass, path, io);
    }
  }

  // If the class is not found and we are not falling back to a base class,
  // return nullptr
  return nullptr;
}

std::shared_ptr<AQNWB::NWB::RegisteredType> RegisteredType::create(
    const std::string& path,
    std::shared_ptr<IO::BaseIO> io,
    bool fallbackToBase)
{
  try {
    // Read the "namespace" attribute
    AQNWB::IO::DataBlockGeneric namespaceData =
        io->readAttribute(AQNWB::mergePaths(path, "namespace"));
    auto namespaceBlock =
        AQNWB::IO::DataBlock<std::string>::fromGeneric(namespaceData);
    std::string typeNamespace = namespaceBlock.data[0];

    // Read the "neurodata_type" attribute
    AQNWB::IO::DataBlockGeneric typeData =
        io->readAttribute(AQNWB::mergePaths(path, "neurodata_type"));
    auto typeBlock = AQNWB::IO::DataBlock<std::string>::fromGeneric(typeData);
    std::string typeName = typeBlock.data[0];

    // Combine the namespace and type name to get the full class name
    std::string fullClassName = typeNamespace + "::" + typeName;

    // Create an instance of the corresponding RegisteredType subclass
    return AQNWB::NWB::RegisteredType::create(
        fullClassName, path, io, fallbackToBase);
  } catch (const std::exception& e) {
    std::cerr << "Error creating RegisteredType instance: " << e.what()
              << std::endl;
    return nullptr;
  }
}

std::unordered_map<std::string, std::string> RegisteredType::findOwnedTypes(
    const std::unordered_set<std::string>& types) const
{
  return m_io->findTypes(m_path, types, IO::SearchMode::STOP_ON_TYPE, true);
}
