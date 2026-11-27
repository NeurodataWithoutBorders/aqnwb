#include "nwb/image/RGBImage.hpp"

using namespace AQNWB::NWB;

// RGBImage
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(RGBImage)

/** Constructor */
RGBImage::RGBImage(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : Image(path, io)
{
}

/** Destructor */
RGBImage::~RGBImage() {}

bool RGBImage::isValidShape(const SizeArray& shape) const
{
  return shape.size() == 3 && shape[2] == 3;
}
