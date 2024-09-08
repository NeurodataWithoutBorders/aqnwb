#include "nwb/hdmf/table/VectorData.hpp"

using namespace AQNWB::NWB;

/** Constructor */
VectorData::VectorData(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
    : Data(path, io)
{
}


// VectorData
std::string VectorData::getDescription() const
{
  return description;
}
