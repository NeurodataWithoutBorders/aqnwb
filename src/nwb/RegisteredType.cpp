#include <limits>

#include "RegisteredType.hpp"

#include "Utils.hpp"
#include "io/ReadIO.hpp"
#include "io/RecordingObjects.hpp"

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
    std::pair<std::function<std::shared_ptr<RegisteredType>(
                  const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>,
              std::pair<std::string, std::string>>>&
RegisteredType::getFactoryMap()
{
  static std::unordered_map<
      std::string,
      std::pair<std::function<std::shared_ptr<RegisteredType>(
                    const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>,
                std::pair<std::string, std::string>>>
      factoryMap;
  return factoryMap;
}

void RegisteredType::registerSubclass(
    const std::string& fullClassName,
    std::function<std::shared_ptr<RegisteredType>(
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

std::shared_ptr<RegisteredType> RegisteredType::getExistingInstance(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
{
  auto recordingObjects = io->getRecordingObjects();
  auto existingObject = recordingObjects->getRecordingObject(path);
  return existingObject;
}

std::shared_ptr<AQNWB::NWB::RegisteredType> RegisteredType::create(
    const std::string& fullClassName,
    const std::string& path,
    std::shared_ptr<IO::BaseIO> io,
    bool fallbackToBase)
{
  // Check the RecordingObjects cache if an object already exists
  auto existingObject = RegisteredType::getExistingInstance(path, io);
  if (existingObject != nullptr){
    return existingObject;
  }
  
  // If no object exists for the path, or if the existing object is of a
  // different type, create a new instance
  //  Look up the factory RegisteredType for the fullClassName the registry
  auto it = getFactoryMap().find(fullClassName);
  if (it != getFactoryMap().end()) {
    return it->second.first(path, io);
  }
  // If the class is not found, return a base class instance by calling this
  // function again with the fallback base class to use for Group and Dataset
  // types respectively
  std::shared_ptr<AQNWB::NWB::RegisteredType> result = nullptr;
  if (fallbackToBase) {
    StorageObjectType sot = io->getStorageObjectType(path);
    if (sot == StorageObjectType::Group) {
      result = create(m_defaultUnregisteredGroupTypeClass, path, io);
    } else if (sot == StorageObjectType::Dataset) {
      result = create(m_defaultUnregisteredDatasetTypeClass, path, io);
    }
  }

  // Ensure the object is registered for recording if it is not already
  if (result != nullptr) {
    result->registerRecordingObject();
  }

  // Return the result, which may be nullptr if creation failed
  return result;
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

SizeType RegisteredType::registerRecordingObject()
{
  // Add this object to the RecordingObjects object of the I/O it is associated
  // with This ensures that all RegisteredType objects used for recording are
  // automatically tracked
  auto ioPtr = getIO();
  if (ioPtr) {
    auto recordingObjects = ioPtr->getRecordingObjects();
    if (recordingObjects) {
      // Get a shared pointer to this object
      std::shared_ptr<RegisteredType> sharedThis = shared_from_this();
      SizeType recordingIndex =
          recordingObjects->addRecordingObject(sharedThis);
      return recordingIndex;
    }
  }
  // Return sentinel value for failure
  return std::numeric_limits<SizeType>::max();
}

AQNWB::Types::Status RegisteredType::finalize()
{
  return AQNWB::Types::Status::Success;
}

std::unordered_map<std::string, std::string> RegisteredType::findOwnedTypes(
    const std::unordered_set<std::string>& types,
    const IO::SearchMode& search_mode) const
{
  auto ioPtr = getIO();
  if (ioPtr != nullptr) {
    return ioPtr->findTypes(m_path, types, search_mode, true);
  } else {
    std::cerr << "IO object has been deleted. Can't find owned types for: "
              << m_path << std::endl;
    return {};
  }
}

SizeType RegisteredType::getRecordingObjectIndex() const
{
  auto ioPtr = getIO();
  if (ioPtr) {
    auto recordingObjects = ioPtr->getRecordingObjects();
    if (recordingObjects) {
      return recordingObjects->getRecordingIndex(shared_from_this());
    }
  }
  // Return sentinel value for failure
  return AQNWB::Types::SizeTypeNotSet;
}
