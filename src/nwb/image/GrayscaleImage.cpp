#include "nwb/image/GrayscaleImage.hpp"

using namespace AQNWB::NWB;

// GrayscaleImage
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(GrayscaleImage)

/** Constructor */
GrayscaleImage::GrayscaleImage(const std::string& path,
                               std::shared_ptr<IO::BaseIO> io)
    : Image(path, io)
{
}

/** Destructor */
GrayscaleImage::~GrayscaleImage() {}

bool GrayscaleImage::isValidShape(const SizeArray& shape) const
{
  return shape.size() == 2;
}
