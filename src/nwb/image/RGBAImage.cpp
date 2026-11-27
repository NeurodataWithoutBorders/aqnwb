#include "nwb/image/RGBAImage.hpp"

using namespace AQNWB::NWB;

// RGBAImage
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(RGBAImage)

/** Constructor */
RGBAImage::RGBAImage(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : Image(path, io)
{
}

/** Destructor */
RGBAImage::~RGBAImage() {}

bool RGBAImage::isValidShape(const SizeArray& shape) const
{
  return shape.size() == 3 && shape[2] == 4;
}
