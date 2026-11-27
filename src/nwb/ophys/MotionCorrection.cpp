#include "nwb/ophys/MotionCorrection.hpp"

using namespace AQNWB::NWB;

// MotionCorrection
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(MotionCorrection)

/** Constructor */
MotionCorrection::MotionCorrection(const std::string& path,
                                   std::shared_ptr<IO::BaseIO> io)
    : NWBDataInterface(path, io)
{
}

/** Destructor */
MotionCorrection::~MotionCorrection() {}

Status MotionCorrection::initialize(const Spec& spec)
{
  return NWBDataInterface::initialize(spec);
}
