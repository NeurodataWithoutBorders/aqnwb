#include <algorithm>

#include "nwb/image/ImageSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// ImageSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ImageSeries)

/** Constructor */
ImageSeries::ImageSeries(const std::string& path,
                         std::shared_ptr<IO::BaseIO> io)
    : TimeSeries(path, io)
{
}

/** Destructor */
ImageSeries::~ImageSeries() {}

Status ImageSeries::Spec::validate() const
{
  Status status = TimeSeries::Spec::validate();
  if (format != "raw" && format != "external") {
    std::cerr << "ImageSeries::Spec::validate: format must be 'raw' or "
                 "'external' but is '"
              << format << "'." << std::endl;
    status = Status::Failure;
  }
  if (format == "external" && !externalFile.has_value()) {
    std::cerr << "ImageSeries::Spec::validate: externalFile is required if "
                 "format is 'external'."
              << std::endl;
    status = Status::Failure;
  }
  if (externalFile.has_value()) {
    if (format != "external") {
      std::cerr << "ImageSeries::Spec::validate: externalFile requires the "
                   "format 'external'."
                << std::endl;
      status = Status::Failure;
    }
    if (externalFileStartingFrame.size() != externalFile.value().size()) {
      std::cerr << "ImageSeries::Spec::validate: externalFileStartingFrame "
                   "must have one element per external file."
                << std::endl;
      status = Status::Failure;
    }
    if (!externalFileStartingFrame.empty()
        && (externalFileStartingFrame[0] != 0
            || !std::is_sorted(externalFileStartingFrame.begin(),
                               externalFileStartingFrame.end())))
    {
      std::cerr << "ImageSeries::Spec::validate: externalFileStartingFrame "
                   "must start at 0 and be non-decreasing."
                << std::endl;
      status = Status::Failure;
    }
  }
  // Check the shape and data type of the data (unless the data is a link,
  // which is validated when the link is created)
  if (data != nullptr && !data->isLink()) {
    SizeArray shape, chunking;
    IO::BaseDataType dataType;
    if (data->getProperties(nullptr, shape, chunking, dataType)
        == Status::Success)
    {
      if (shape.size() != 3 && shape.size() != 4) {
        std::cerr << "ImageSeries::Spec::validate: data must be a 3D [frame, "
                     "x, y] or 4D [frame, x, y, z] array but has "
                  << shape.size() << " dimensions." << std::endl;
        status = Status::Failure;
      }
      if (dataType.type == IO::BaseDataType::Type::T_STR
          || dataType.type == IO::BaseDataType::Type::V_STR)
      {
        std::cerr << "ImageSeries::Spec::validate: data must be numeric."
                  << std::endl;
        status = Status::Failure;
      }
    }
  }
  return status;
}

Status ImageSeries::initialize(const Spec& spec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "ImageSeries::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  // Check the link targets before anything is written
  if (checkLinkTarget(spec.device, "device") != Status::Success) {
    return Status::Failure;
  }
  // Validate the spec and initialize the TimeSeries
  Status status = TimeSeries::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }

  m_isExternal = spec.format == "external";

  status = status
      && ioPtr->createStringDataSet(AQNWB::mergePaths(m_path, "format"),
                                    spec.format);

  if (spec.dimension.has_value()) {
    const std::vector<int32_t>& dimension = spec.dimension.value();
    IO::ArrayDataSetConfig dimensionConfig(IO::BaseDataType::I32,
                                           SizeArray {dimension.size()},
                                           SizeArray {dimension.size()});
    auto dimensionRecorder = ioPtr->createArrayDataSet(
        dimensionConfig, AQNWB::mergePaths(m_path, "dimension"));
    if (dimensionRecorder == nullptr) {
      status = Status::Failure;
    } else {
      status = status
          && dimensionRecorder->writeDataBlock(SizeArray {dimension.size()},
                                               IO::BaseDataType::I32,
                                               dimension.data());
    }
  }

  if (spec.externalFile.has_value()) {
    const std::string externalFilePath =
        AQNWB::mergePaths(m_path, "external_file");
    status = status
        && ioPtr->createStringDataSet(externalFilePath,
                                      spec.externalFile.value());
    status = status
        && ioPtr->createAttribute(IO::BaseDataType::I32,
                                  spec.externalFileStartingFrame.data(),
                                  externalFilePath,
                                  "starting_frame",
                                  spec.externalFileStartingFrame.size());
  }

  if (spec.numSamples.has_value()) {
    const uint32_t numSamples = spec.numSamples.value();
    status = status
        && ioPtr->createScalarDataSet(IO::BaseDataType::U32,
                                      &numSamples,
                                      AQNWB::mergePaths(m_path, "num_samples"));
  }

  if (spec.device != nullptr) {
    status = status
        && ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "device"),
                             AQNWB::mergePaths("/", spec.device->getPath()));
  }

  return status;
}

Status ImageSeries::writeFrames(const SizeType& numFrames,
                                const void* dataInput,
                                const void* timestampsInput,
                                const void* controlInput)
{
  return appendSamples(numFrames, dataInput, timestampsInput, controlInput);
}

Status ImageSeries::appendSamples(const SizeType& numSamples,
                                  const void* dataInput,
                                  const void* timestampsInput,
                                  const void* controlInput)
{
  if (m_isExternal) {
    std::cerr << "ImageSeries::appendSamples: " << m_path
              << " stores the images in external files." << std::endl;
    return Status::Failure;
  }
  return TimeSeries::appendSamples(
      numSamples, dataInput, timestampsInput, controlInput);
}

SizeArray ImageSeries::getFrameShape() const
{
  auto dataRecorder = const_cast<ImageSeries*>(this)->recordData();
  if (dataRecorder == nullptr) {
    return {};
  }
  SizeArray shape = dataRecorder->getShape();
  return shape.empty() ? SizeArray {}
                       : SizeArray(shape.begin() + 1, shape.end());
}

SizeType ImageSeries::getNumFramesRecorded() const
{
  auto dataRecorder = const_cast<ImageSeries*>(this)->recordData();
  if (dataRecorder == nullptr) {
    return 0;
  }
  SizeArray shape = dataRecorder->getShape();
  return shape.empty() ? 0 : shape[0];
}
