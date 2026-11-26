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

  // Validate the spec and initialize the TimeSeries
  Status status = TimeSeries::initialize(spec);
  if (status != Status::Success) {
    return Status::Failure;
  }

  // Remember the shape of a frame for writeFrames
  SizeArray shape, chunking;
  IO::BaseDataType dataType;
  if (spec.data->getProperties(ioPtr.get(), shape, chunking, dataType)
          == Status::Success
      && !shape.empty())
  {
    m_frameShape = SizeArray(shape.begin() + 1, shape.end());
  }
  m_framesRecorded = 0;

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
  if (m_frameShape.empty()) {
    std::cerr << "ImageSeries::writeFrames: The frame shape is unknown. The "
                 "ImageSeries must be initialized before writing frames."
              << std::endl;
    return Status::Failure;
  }

  SizeArray dataShape = {numFrames};
  dataShape.insert(dataShape.end(), m_frameShape.begin(), m_frameShape.end());
  SizeArray positionOffset(dataShape.size(), 0);
  positionOffset[0] = m_framesRecorded;

  Status status = this->writeData(
      dataShape, positionOffset, dataInput, timestampsInput, controlInput);
  if (status == Status::Success) {
    m_framesRecorded += numFrames;
  }
  return status;
}
