#include "nwb/ophys/Fluorescence.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// Fluorescence
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(Fluorescence)

/** Constructor */
Fluorescence::Fluorescence(const std::string& path,
                           std::shared_ptr<IO::BaseIO> io)
    : NWBDataInterface(path, io)
{
}

/** Destructor */
Fluorescence::~Fluorescence() {}

Status Fluorescence::initialize(const Spec& spec)
{
  return NWBDataInterface::initialize(spec);
}
