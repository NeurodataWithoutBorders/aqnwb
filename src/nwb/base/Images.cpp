#include <set>

#include "nwb/base/Images.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// Initialize the static registered_ members to trigger registration
REGISTER_SUBCLASS_IMPL(ImageReferences)
REGISTER_SUBCLASS_IMPL(Images)

/** Constructor */
ImageReferences::ImageReferences(const std::string& path,
                                 std::shared_ptr<IO::BaseIO> io)
    : NWBData(path, io)
{
}

/** Destructor */
ImageReferences::~ImageReferences() {}

Status ImageReferences::initialize(const std::vector<std::string>& imagePaths)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "ImageReferences::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }
  for (const auto& imagePath : imagePaths) {
    if (!ioPtr->objectExists(imagePath)) {
      std::cerr << "ImageReferences::initialize: The image " << imagePath
                << " does not exist." << std::endl;
      return Status::Failure;
    }
  }
  Status status = ioPtr->createReferenceDataSet(m_path, imagePaths);
  status = status
      && ioPtr->createCommonNWBAttributes(
          m_path, getNamespace(), getTypeName());
  return status;
}

/** Constructor */
Images::Images(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : NWBDataInterface(path, io)
{
}

/** Destructor */
Images::~Images() {}

Status Images::Spec::validate() const
{
  Status status = NWBDataInterface::Spec::validate();
  if (description.empty()) {
    std::cerr << "Images::Spec::validate: description must not be empty."
              << std::endl;
    status = Status::Failure;
  }
  if (images.empty()) {
    std::cerr << "Images::Spec::validate: At least one image is required."
              << std::endl;
    status = Status::Failure;
  }
  std::set<std::string> names;
  for (const auto& image : images) {
    if (image.name.empty() || !names.insert(image.name).second) {
      std::cerr << "Images::Spec::validate: The names of the images must be "
                   "unique and not empty."
                << std::endl;
      status = Status::Failure;
    }
    if (image.data == nullptr || !image.create) {
      std::cerr << "Images::Spec::validate: The image " << image.name
                << " has no data (use withImage)." << std::endl;
      status = Status::Failure;
    }
    status = status && image.spec.validate();
  }
  if (orderOfImages.has_value()) {
    for (const auto& name : orderOfImages.value()) {
      if (names.count(name) == 0) {
        std::cerr << "Images::Spec::validate: orderOfImages refers to the "
                     "unknown image "
                  << name << std::endl;
        status = Status::Failure;
      }
    }
  }
  return status;
}

Status Images::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Images::initialize: IO object is not valid." << std::endl;
    return Status::Failure;
  }
  // Check the spec and the images (e.g., the shapes for the image types)
  // before anything is written
  if (spec.validate() != Status::Success) {
    return Status::Failure;
  }
  std::vector<std::shared_ptr<Image>> images;
  for (const auto& entry : spec.images) {
    auto image = entry.create(AQNWB::mergePaths(m_path, entry.name), ioPtr);
    if (image == nullptr || image->checkSpec(entry.spec) != Status::Success) {
      return Status::Failure;
    }
    images.push_back(image);
  }

  // Create the collection, the images, and the order of the images
  Status status = NWBDataInterface::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }
  status = ioPtr->createAttribute(spec.description, m_path, "description");
  for (size_t i = 0; i < images.size(); ++i) {
    status = status
        && writeImage(*images[i], spec.images[i].spec, spec.images[i].data);
  }
  if (spec.orderOfImages.has_value()) {
    status = status && setOrderOfImages(spec.orderOfImages.value());
  }
  return status;
}

bool Images::hasOrderOfImages() const
{
  auto ioPtr = getIO();
  return ioPtr != nullptr
      && ioPtr->objectExists(AQNWB::mergePaths(m_path, "order_of_images"));
}

Status Images::writeImage(Image& image,
                          const Image::Spec& spec,
                          const void* data)
{
  if (data == nullptr || image.initialize(spec) != Status::Success) {
    return Status::Failure;
  }
  SizeArray shape, chunking;
  IO::BaseDataType dataType;
  if (spec.data->getProperties(getIO().get(), shape, chunking, dataType)
      != Status::Success)
  {
    return Status::Failure;
  }
  return image.writeImage(shape, dataType, data);
}

Status Images::setOrderOfImages(const std::vector<std::string>& imageNames)
{
  std::vector<std::string> imagePaths;
  for (const auto& name : imageNames) {
    imagePaths.push_back(AQNWB::mergePaths(m_path, name));
  }
  auto order = ImageReferences::create(
      AQNWB::mergePaths(m_path, "order_of_images"), getIO());
  if (order == nullptr) {
    return Status::Failure;
  }
  return order->initialize(imagePaths);
}
