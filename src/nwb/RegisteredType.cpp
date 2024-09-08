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
    std::function<std::unique_ptr<RegisteredType>(
        const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>>&
RegisteredType::getFactoryMap()
{
  static std::unordered_map<
      std::string,
      std::function<std::unique_ptr<RegisteredType>(
          const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>>
      factoryMap;
  return factoryMap;
}

void RegisteredType::registerSubclass(
    const std::string& subclassName,
    std::function<std::unique_ptr<RegisteredType>(
        const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>
        factoryFunction)
{
  getRegistry().insert(subclassName);
  getFactoryMap()[subclassName] = factoryFunction;
}

std::unique_ptr<RegisteredType> RegisteredType::create(
    const std::string& subclassName,
    const std::string& path,
    std::shared_ptr<AQNWB::IO::BaseIO> io)
{
  auto it = getFactoryMap().find(subclassName);
  if (it != getFactoryMap().end()) {
    return it->second(path, io);
  }
  return nullptr;
}
