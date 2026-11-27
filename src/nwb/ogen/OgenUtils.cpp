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
  // Validate the spec before creating the parent group
  if (spec.validate() != Status::Success) {
    std::cerr << "Ogen::createOptogeneticStimulusSite: Invalid spec for "
              << name << std::endl;
    return nullptr;
  }
  if (!ioPtr->objectExists(OPTOGENETICS_PATH)
      && ioPtr->createGroup(OPTOGENETICS_PATH) != Status::Success)
  {
    std::cerr << "Ogen::createOptogeneticStimulusSite: Failed to create "
              << OPTOGENETICS_PATH << std::endl;
    return nullptr;
  }
  auto obj = OptogeneticStimulusSite::create(
      AQNWB::mergePaths(OPTOGENETICS_PATH, name), ioPtr);
  if (obj == nullptr || obj->initialize(spec) != Status::Success) {
    std::cerr << "Ogen::createOptogeneticStimulusSite: Failed to create "
              << name << std::endl;
    return nullptr;
  }
  return obj;
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
