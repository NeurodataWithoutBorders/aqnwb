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
  return status;
}

Status Images::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Images::initialize: IO object is not valid." << std::endl;
    return Status::Failure;
  }
  Status status = NWBDataInterface::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }
  return ioPtr->createAttribute(spec.description, m_path, "description");
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
