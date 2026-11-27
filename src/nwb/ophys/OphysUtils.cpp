#include "nwb/ophys/OphysUtils.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

std::shared_ptr<ImagingPlane> Ophys::createImagingPlane(
    const NWBFile& nwbfile,
    const std::string& name,
    const ImagingPlane::Spec& spec)
{
  auto ioPtr = nwbfile.getIO();
  if (!ioPtr) {
    std::cerr << "Ophys::createImagingPlane: IO object is not valid."
              << std::endl;
    return nullptr;
  }
  if (!ioPtr->objectExists(OPTOPHYSIOLOGY_PATH)
      && ioPtr->createGroup(OPTOPHYSIOLOGY_PATH) != Status::Success)
  {
    std::cerr << "Ophys::createImagingPlane: Failed to create "
              << OPTOPHYSIOLOGY_PATH << std::endl;
    return nullptr;
  }
  return spec.createAndInitialize(AQNWB::mergePaths(OPTOPHYSIOLOGY_PATH, name),
                                  ioPtr);
}

std::shared_ptr<TwoPhotonSeries> Ophys::createTwoPhotonSeries(
    const NWBFile& nwbfile,
    const std::string& name,
    const TwoPhotonSeries::Spec& spec)
{
  auto series = nwbfile.createAquisitionSeries<TwoPhotonSeries>(name);
  if (series == nullptr || series->initialize(spec) != Status::Success) {
    std::cerr << "Ophys::createTwoPhotonSeries: Failed to create " << name
              << std::endl;
    return nullptr;
  }
  return series;
}

std::shared_ptr<OnePhotonSeries> Ophys::createOnePhotonSeries(
    const NWBFile& nwbfile,
    const std::string& name,
    const OnePhotonSeries::Spec& spec)
{
  auto series = nwbfile.createAquisitionSeries<OnePhotonSeries>(name);
  if (series == nullptr || series->initialize(spec) != Status::Success) {
    std::cerr << "Ophys::createOnePhotonSeries: Failed to create " << name
              << std::endl;
    return nullptr;
  }
  return series;
}

AQNWB::IO::ArrayDataSetConfig Ophys::getDefaultImageSeriesDataConfig(
    const IO::BaseDataType& dataType, const SizeArray& frameShape)
{
  SizeArray shape = {0};
  shape.insert(shape.end(), frameShape.begin(), frameShape.end());
  SizeArray chunking = {1};
  chunking.insert(chunking.end(), frameShape.begin(), frameShape.end());
  return IO::ArrayDataSetConfig(dataType, shape, chunking);
}

AQNWB::Types::Status Ophys::writeImageSeriesFrames(
    const std::shared_ptr<IO::RecordingObjects>& recordingObjects,
    const SizeType& containerInd,
    const SizeType& numFrames,
    const void* data,
    const void* timestamps)
{
  if (recordingObjects == nullptr) {
    return Status::Failure;
  }
  auto imageSeries = std::dynamic_pointer_cast<ImageSeries>(
      recordingObjects->getRecordingObject(containerInd));
  if (imageSeries == nullptr) {
    std::cerr << "Ophys::writeImageSeriesFrames: The recording object "
              << containerInd << " is not an ImageSeries." << std::endl;
    return Status::Failure;
  }
  return imageSeries->writeFrames(numFrames, data, timestamps);
}

std::shared_ptr<ProcessingModule> Ophys::getOrCreateOphysModule(
    const NWBFile& nwbfile)
{
  auto module = nwbfile.readProcessingModule(OPHYS_MODULE_NAME);
  if (module != nullptr) {
    return module;
  }
  module = nwbfile.createProcessingModule(OPHYS_MODULE_NAME);
  if (module == nullptr
      || module->initialize(ProcessingModule::Spec().withDescription(
             "optical physiology processed data"))
          != Status::Success)
  {
    std::cerr << "Ophys::getOrCreateOphysModule: Failed to create the ophys "
                 "processing module."
              << std::endl;
    return nullptr;
  }
  return module;
}

namespace
{
/**
 * @brief Get the container of type T with the given name in the ophys module
 * and create it if it does not exist.
 */
template<typename T>
std::shared_ptr<T> getOrCreateOphysContainer(const NWBFile& nwbfile,
                                             const std::string& name)
{
  auto module = Ophys::getOrCreateOphysModule(nwbfile);
  if (module == nullptr) {
    return nullptr;
  }
  auto container = module->readNWBDataInterface<T>(name);
  if (container != nullptr) {
    return container;
  }
  container = module->createNWBDataInterface<T>(name);
  if (container == nullptr
      || container->initialize(typename T::Spec()) != Status::Success)
  {
    std::cerr << "Ophys: Failed to create " << name << std::endl;
    return nullptr;
  }
  return container;
}
}  // namespace

std::shared_ptr<PlaneSegmentation> Ophys::createPlaneSegmentation(
    const NWBFile& nwbfile,
    const std::string& name,
    const PlaneSegmentation::Spec& spec)
{
  auto imageSegmentation = getOrCreateOphysContainer<ImageSegmentation>(
      nwbfile, "ImageSegmentation");
  if (imageSegmentation == nullptr) {
    return nullptr;
  }
  auto planeSegmentation = imageSegmentation->createPlaneSegmentation(name);
  if (planeSegmentation == nullptr
      || planeSegmentation->initialize(spec) != Status::Success)
  {
    std::cerr << "Ophys::createPlaneSegmentation: Failed to create " << name
              << std::endl;
    return nullptr;
  }
  return planeSegmentation;
}

std::shared_ptr<RoiResponseSeries> Ophys::createRoiResponseSeries(
    const NWBFile& nwbfile,
    const RoiResponseContainer& container,
    const std::string& name,
    const RoiResponseSeries::Spec& spec)
{
  std::shared_ptr<RoiResponseSeries> series = nullptr;
  if (container == RoiResponseContainer::Fluorescence) {
    auto fluorescence =
        getOrCreateOphysContainer<Fluorescence>(nwbfile, "Fluorescence");
    series =
        fluorescence ? fluorescence->createRoiResponseSeries(name) : nullptr;
  } else {
    auto dfOverF = getOrCreateOphysContainer<DfOverF>(nwbfile, "DfOverF");
    series = dfOverF ? dfOverF->createRoiResponseSeries(name) : nullptr;
  }
  if (series == nullptr || series->initialize(spec) != Status::Success) {
    std::cerr << "Ophys::createRoiResponseSeries: Failed to create " << name
              << std::endl;
    return nullptr;
  }
  return series;
}

AQNWB::Types::Status Ophys::writeRoiResponseSeriesData(
    const std::shared_ptr<IO::RecordingObjects>& recordingObjects,
    const SizeType& containerInd,
    const SizeType& numSamples,
    const void* data,
    const void* timestamps)
{
  if (recordingObjects == nullptr) {
    return Status::Failure;
  }
  auto series = std::dynamic_pointer_cast<RoiResponseSeries>(
      recordingObjects->getRecordingObject(containerInd));
  if (series == nullptr) {
    std::cerr << "Ophys::writeRoiResponseSeriesData: The recording object "
              << containerInd << " is not a RoiResponseSeries." << std::endl;
    return Status::Failure;
  }
  return series->writeSamples(numSamples, data, timestamps);
}

std::shared_ptr<CorrectedImageStack> Ophys::createCorrectedImageStack(
    const NWBFile& nwbfile,
    const std::string& name,
    const CorrectedImageStack::Spec& spec)
{
  auto motionCorrection =
      getOrCreateOphysContainer<MotionCorrection>(nwbfile, "MotionCorrection");
  if (motionCorrection == nullptr) {
    return nullptr;
  }
  auto stack = motionCorrection->createCorrectedImageStack(name);
  if (stack == nullptr || stack->initialize(spec) != Status::Success) {
    std::cerr << "Ophys::createCorrectedImageStack: Failed to create " << name
              << std::endl;
    return nullptr;
  }
  return stack;
}
