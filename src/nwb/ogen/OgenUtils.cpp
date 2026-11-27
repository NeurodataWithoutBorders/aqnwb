#include "nwb/ogen/OgenUtils.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

std::shared_ptr<OptogeneticStimulusSite> Ogen::createOptogeneticStimulusSite(
    const NWBFile& nwbfile,
    const std::string& name,
    const OptogeneticStimulusSite::Spec& spec)
{
  auto ioPtr = nwbfile.getIO();
  if (!ioPtr) {
    std::cerr << "Ogen::createOptogeneticStimulusSite: IO object is not valid."
              << std::endl;
    return nullptr;
  }
  if (!ioPtr->objectExists(OPTOGENETICS_PATH)
      && ioPtr->createGroup(OPTOGENETICS_PATH) != Status::Success)
  {
    std::cerr << "Ogen::createOptogeneticStimulusSite: Failed to create "
              << OPTOGENETICS_PATH << std::endl;
    return nullptr;
  }
  return spec.createAndInitialize(AQNWB::mergePaths(OPTOGENETICS_PATH, name),
                                  ioPtr);
}

std::shared_ptr<OptogeneticSeries> Ogen::createOptogeneticSeries(
    const NWBFile& nwbfile,
    const std::string& name,
    const OptogeneticSeries::Spec& spec)
{
  auto series = nwbfile.createAquisitionSeries<OptogeneticSeries>(name);
  if (series == nullptr || series->initialize(spec) != Status::Success) {
    std::cerr << "Ogen::createOptogeneticSeries: Failed to create " << name
              << std::endl;
    return nullptr;
  }
  return series;
}

AQNWB::Types::Status Ogen::writeOptogeneticSeriesData(
    const std::shared_ptr<IO::RecordingObjects>& recordingObjects,
    const SizeType& containerInd,
    const SizeType& numSamples,
    const void* data,
    const void* timestamps)
{
  if (recordingObjects == nullptr) {
    return Status::Failure;
  }
  auto series = std::dynamic_pointer_cast<OptogeneticSeries>(
      recordingObjects->getRecordingObject(containerInd));
  if (series == nullptr) {
    std::cerr << "Ogen::writeOptogeneticSeriesData: The recording object "
              << containerInd << " is not an OptogeneticSeries." << std::endl;
    return Status::Failure;
  }
  return series->writeSamples(numSamples, data, timestamps);
}
