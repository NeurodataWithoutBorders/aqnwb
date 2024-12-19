#include "nwb/hdmf/base/Data.hpp"

using namespace AQNWB::NWB;

// Container
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(Data)

/** Constructor */
Data::Data(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
    : RegisteredType(path, io)
{
}