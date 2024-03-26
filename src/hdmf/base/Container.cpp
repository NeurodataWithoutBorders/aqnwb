#include "Container.hpp"

using namespace AQNWBIO;

// Container

/** Constructor */
Container::Container(const std::string& path, std::shared_ptr<BaseIO> io)
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
