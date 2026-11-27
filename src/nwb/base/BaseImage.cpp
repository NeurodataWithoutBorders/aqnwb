#include "nwb/base/BaseImage.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// Initialize the static registered_ members to trigger registration
REGISTER_SUBCLASS_IMPL(BaseImage)
REGISTER_SUBCLASS_IMPL(Image)

/** Constructor */
BaseImage::BaseImage(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : NWBData(path, io)
{
}

/** Destructor */
BaseImage::~BaseImage() {}

Status BaseImage::initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                             const std::optional<std::string>& description)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "BaseImage::initialize: IO object is not valid." << std::endl;
    return Status::Failure;
  }
  Status status = NWBData::initialize(dataConfig);
  if (status == Status::Success && description.has_value()) {
    status = ioPtr->createAttribute(description.value(), m_path, "description");
  }
  return status;
}

/** Constructor */
Image::Image(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : BaseImage(path, io)
{
}

/** Destructor */
Image::~Image() {}

bool Image::isValidShape(const SizeArray& shape) const
{
  return shape.size() == 2
      || (shape.size() == 3 && (shape[2] == 3 || shape[2] == 4));
}

Status Image::initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                         const std::optional<std::string>& description,
                         const std::optional<float>& resolution)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Image::initialize: IO object is not valid." << std::endl;
    return Status::Failure;
  }
  SizeArray shape, chunking;
  IO::BaseDataType dataType;
  if (dataConfig.getProperties(ioPtr.get(), shape, chunking, dataType)
      != Status::Success)
  {
    return Status::Failure;
  }
  if (!isValidShape(shape)) {
    std::cerr << "Image::initialize: Invalid shape of the " << getTypeName()
              << " " << m_path << std::endl;
    return Status::Failure;
  }
  if (dataType.type == IO::BaseDataType::Type::T_STR
      || dataType.type == IO::BaseDataType::Type::V_STR
      || dataType.isCompound())
  {
    std::cerr << "Image::initialize: The data of an image must be numeric."
              << std::endl;
    return Status::Failure;
  }

  Status status = BaseImage::initialize(dataConfig, description);
  if (status == Status::Success && resolution.has_value()) {
    const float resolutionValue = resolution.value();
    status = ioPtr->createAttribute(
        IO::BaseDataType::F32, &resolutionValue, m_path, "resolution");
  }
  return status;
}

Status Image::writeImage(const SizeArray& shape,
                         const IO::BaseDataType& dataType,
                         const void* data)
{
  auto recorder = recordData();
  if (recorder == nullptr) {
    return Status::Failure;
  }
  return recorder->writeDataBlock(
      shape, SizeArray(shape.size(), 0), dataType, data);
}
