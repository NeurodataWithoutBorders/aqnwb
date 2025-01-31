#include "nwb/hdmf/table/VectorData.hpp"

using namespace AQNWB::NWB;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(VectorData)

/** Constructor */
VectorData::VectorData(const std::string& path,
                       std::shared_ptr<AQNWB::IO::BaseIO> io)
    : Data(path, io)
{
}

Status VectorData::initialize(
    std::unique_ptr<AQNWB::IO::BaseRecordingData>&& dataset,
    const std::string& description)
{
  Status dataStatus = Data::initialize(std::move(dataset));
  m_io->createAttribute(description, m_path, "description");
  return dataStatus;
}
