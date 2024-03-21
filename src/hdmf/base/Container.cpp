#include "Container.hpp"

// Container

/** Constructor */
Container::Container(const std::string& path, std::shared_ptr<BaseIO> io)
  : path(path)
  , io(io)
{
  // create group if it does not exist
  io->createGroup(path);
}

/** Destructor */
Container::~Container() {}

/** Getter for path */
std::string Container::getPath() const {
  return path;
}
