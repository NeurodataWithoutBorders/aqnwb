#include "RegisteredType.hpp"

using namespace AQNWB::NWB;

RegisteredType::RegisteredType(const std::string& path,
                               std::shared_ptr<AQNWB::IO::BaseIO> io)
    : path(path)
    , io(io)
{
}

RegisteredType::~RegisteredType() {}

std::string RegisteredType::getPath() const
{
  return path;
}

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

std::string RegisteredType::getTypeName() const
{
  for (const auto& entry : getFactoryMap()) {
    if (entry.second.first(path, io)->getTypeName() == typeName) {
      return entry.second.second.first;
    }
  }
  return "";
}

std::string RegisteredType::getNamespace() const
{
  for (const auto& entry : getFactoryMap()) {
    if (entry.second.first(path, io)->getNamespace() == typeNamespace) {
      return entry.second.second.second;
    }
  }
  return "";
}
