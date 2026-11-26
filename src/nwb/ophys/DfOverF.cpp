#include "nwb/ophys/DfOverF.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// DfOverF
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(DfOverF)

/** Constructor */
DfOverF::DfOverF(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : NWBDataInterface(path, io)
{
}

/** Destructor */
DfOverF::~DfOverF() {}

Status DfOverF::initialize(const Spec& spec)
{
  return NWBDataInterface::initialize(spec);
}
