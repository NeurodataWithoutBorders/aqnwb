#include "RegisteredType.hpp"

#include "io/ReadIO.hpp"
#include "Utils.hpp"

using namespace AQNWB::NWB;

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

/**
 * @brief Utility function to create an instance of a RegisteredType subclass
 *        based on the "namespace" and "neurodata_type" attributes at a given
 * path.
 *
 * @param path The path in the file from which to read the attributes.
 * @param io A shared pointer to the IO object.
 * @return A unique pointer to the created RegisteredType instance, or nullptr
 * if creation fails.
 */
std::shared_ptr<AQNWB::NWB::RegisteredType> RegisteredType::create(
    const std::string& path, std::shared_ptr<IO::BaseIO> io)
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
    return AQNWB::NWB::RegisteredType::create(fullClassName, path, io);
  } catch (const std::exception& e) {
    std::cerr << "Error creating RegisteredType instance: " << e.what()
              << std::endl;
    return nullptr;
  }
}
