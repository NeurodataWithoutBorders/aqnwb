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

Status BaseImage::Spec::validate() const
{
  Status status = RegisteredTypeSpec::validate();
  if (data == nullptr) {
    std::cerr << "BaseImage::Spec::validate: data must be set." << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status BaseImage::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "BaseImage::initialize: IO object is not valid." << std::endl;
    return Status::Failure;
  }
  // Data types are not initialized via Container::initialize, i.e., validate
  // the spec here
  if (spec.validate() != Status::Success) {
    return Status::Failure;
  }
  Status status = NWBData::initialize(*spec.data);
  if (status == Status::Success && spec.description.has_value()) {
    status =
        ioPtr->createAttribute(spec.description.value(), m_path, "description");
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

Status Image::Spec::validate() const
{
  Status status = BaseImage::Spec::validate();
  if (data != nullptr && !data->isLink()) {
    SizeArray shape, chunking;
    IO::BaseDataType dataType;
    if (data->getProperties(nullptr, shape, chunking, dataType)
            == Status::Success
        && (dataType.type == IO::BaseDataType::Type::T_STR
            || dataType.type == IO::BaseDataType::Type::V_STR
            || dataType.isCompound()))
    {
      std::cerr << "Image::Spec::validate: The data of an image must be "
                   "numeric."
                << std::endl;
      status = Status::Failure;
    }
  }
  if (resolution.has_value() && !(resolution.value() > 0.0f)) {
    std::cerr << "Image::Spec::validate: resolution must be > 0." << std::endl;
    status = Status::Failure;
  }
  return status;
}

Status Image::checkSpec(const Spec& spec) const
{
  if (spec.validate() != Status::Success) {
    return Status::Failure;
  }
  auto ioPtr = getIO();
  SizeArray shape, chunking;
  IO::BaseDataType dataType;
  if (ioPtr == nullptr
      || spec.data->getProperties(ioPtr.get(), shape, chunking, dataType)
          != Status::Success)
  {
    return Status::Failure;
  }
  if (!isValidShape(shape)) {
    std::cerr << "Image::initialize: Invalid shape of the " << getTypeName()
              << " " << m_path << std::endl;
    return Status::Failure;
  }
  return Status::Success;
}

Status Image::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Image::initialize: IO object is not valid." << std::endl;
    return Status::Failure;
  }
  // Check the spec and the shape of the data before anything is written
  if (checkSpec(spec) != Status::Success) {
    return Status::Failure;
  }
  Status status = BaseImage::initialize(spec);
  if (status == Status::Success && spec.resolution.has_value()) {
    const float resolutionValue = spec.resolution.value();
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
