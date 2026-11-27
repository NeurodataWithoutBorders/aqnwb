#include "nwb/ophys/ImageSegmentation.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// ImageSegmentation
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ImageSegmentation)

/** Constructor */
ImageSegmentation::ImageSegmentation(const std::string& path,
                                     std::shared_ptr<IO::BaseIO> io)
    : NWBDataInterface(path, io)
{
}

/** Destructor */
ImageSegmentation::~ImageSegmentation() {}

Status ImageSegmentation::initialize(const Spec& spec)
{
  return NWBDataInterface::initialize(spec);
}
