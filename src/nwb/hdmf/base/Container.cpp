#include "nwb/hdmf/base/Container.hpp"

using namespace AQNWB::NWB;

// Container

/** Constructor */
Container::Container(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
    : path(path)
    , io(io)
{
}

/** Destructor */
Container::~Container() {}

/** Initialize */
void Container::initialize()
{
  io->createGroup(path);
}

/** Getter for path */
std::string Container::getPath() const
{
  return path;
}

std::unordered_set<std::string>& Container::getRegistry() {
    static std::unordered_set<std::string> registry;
    return registry;
}

std::unordered_map<std::string, std::function<std::unique_ptr<Container>(const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>>& Container::getFactoryMap() {
    static std::unordered_map<std::string, std::function<std::unique_ptr<Container>(const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)>> factoryMap;
    return factoryMap;
}

void Container::registerSubclass(const std::string& subclassName, std::function<std::unique_ptr<Container>(const std::string&, std::shared_ptr<AQNWB::IO::BaseIO>)> factoryFunction) {
    getRegistry().insert(subclassName);
    getFactoryMap()[subclassName] = factoryFunction;
}

std::unique_ptr<Container> Container::create(const std::string& subclassName, const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io) {
    auto it = getFactoryMap().find(subclassName);
    if (it != getFactoryMap().end()) {
        return it->second(path, io);
    }
    return nullptr;
}