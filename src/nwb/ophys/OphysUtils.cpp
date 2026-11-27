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
  // Validate the spec before creating the parent group
  if (spec.validate() != Status::Success) {
    std::cerr << "Ophys::createImagingPlane: Invalid spec for " << name
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
  auto obj =
      ImagingPlane::create(AQNWB::mergePaths(OPTOPHYSIOLOGY_PATH, name), ioPtr);
  if (obj == nullptr || obj->initialize(spec) != Status::Success) {
    std::cerr << "Ophys::createImagingPlane: Failed to create " << name
              << std::endl;
    return nullptr;
  }
  return obj;
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
  // Validate the spec before creating any parent containers
  if (spec.validate() != Status::Success) {
    return nullptr;
  }
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
  // Validate the spec before creating any parent containers
  if (spec.validate() != Status::Success) {
    return nullptr;
  }
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
