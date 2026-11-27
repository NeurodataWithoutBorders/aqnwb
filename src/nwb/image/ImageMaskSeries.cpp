#include "nwb/image/ImageMaskSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// ImageMaskSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ImageMaskSeries)

/** Constructor */
ImageMaskSeries::ImageMaskSeries(const std::string& path,
                                 std::shared_ptr<IO::BaseIO> io)
    : ImageSeries(path, io)
{
}

/** Destructor */
ImageMaskSeries::~ImageMaskSeries() {}

Status ImageMaskSeries::Spec::validate() const
{
  Status status = ImageSeries::Spec::validate();
  if (maskedImageSeries == nullptr) {
    std::cerr << "ImageMaskSeries::Spec::validate: maskedImageSeries must be "
                 "set."
              << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status ImageMaskSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "ImageMaskSeries::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }
  // Check the link targets before anything is written
  if (checkLinkTarget(spec.maskedImageSeries, "maskedImageSeries")
      != Status::Success)
  {
    return Status::Failure;
  }
  Status status = ImageSeries::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }
  return ioPtr->createLink(
      AQNWB::mergePaths("/" + m_path, "masked_imageseries"),
      AQNWB::mergePaths("/", spec.maskedImageSeries->getPath()));
}
