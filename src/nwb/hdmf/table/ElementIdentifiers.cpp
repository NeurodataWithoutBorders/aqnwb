#include "nwb/hdmf/table/ElementIdentifiers.hpp"

using namespace AQNWB::NWB;

/** Constructor */
ElementIdentifiers::ElementIdentifiers(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
    : Data(path, io)
{
}

