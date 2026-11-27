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
