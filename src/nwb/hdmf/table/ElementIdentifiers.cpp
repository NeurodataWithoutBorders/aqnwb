#include "nwb/hdmf/table/ElementIdentifiers.hpp"
// #include "nwb/hdmf/table/VectorData.hpp"

using namespace AQNWB::NWB;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ElementIdentifiers)
// REGISTER_SUBCLASS_IMPL(Data)
// REGISTER_SUBCLASS_IMPL(VectorData)

/** Constructor */
ElementIdentifiers::ElementIdentifiers(const std::string& path,
                                       std::shared_ptr<AQNWB::IO::BaseIO> io)
    : Data(path, io)
{
}
