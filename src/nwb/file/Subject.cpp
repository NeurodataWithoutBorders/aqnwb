#include "nwb/file/Subject.hpp"

#include "Utils.hpp"
#include "nwb/NWBFile.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(Subject)

Subject::Subject(std::shared_ptr<IO::BaseIO> io)
    : NWBContainer(mergePaths(NWBFile::GENERAL_PATH, "subject"), io)
{
}

// Constructor
Subject::Subject(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
    : NWBContainer(mergePaths(NWBFile::GENERAL_PATH, "subject"), io)
{
  if (path != mergePaths(NWBFile::GENERAL_PATH, "subject")) {
    std::cerr << "WARNING: Subject object path must be /general/subject. "
                 "Ignoring provided path."
              << std::endl;
  }
}

std::shared_ptr<Subject> Subject::create(std::shared_ptr<IO::BaseIO> io)
{
  return RegisteredType::create<Subject>(
      mergePaths(NWBFile::GENERAL_PATH, "subject"), io);
}

// Initialize the object
Status Subject::initialize(const SubjectSpec& subjectSpec)
{
  // Get the IO object
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "Subject::initialize IO object has been deleted." << std::endl;
    return Status::Failure;
  }
  if (!ioPtr->canModifyObjects()) {
    return Status::Failure;
  }

  // Call parent initialize method.
  Status initStatus = Status::Success;
  Status parentInitStatus = NWBContainer::initialize();
  initStatus = initStatus && parentInitStatus;

  // Initialize attributes, datasets, and groups
  // Initialize age dataset and age/reference attribute if age is provided
  if (subjectSpec.age.has_value()) {
    Status ageStatus = ioPtr->createStringDataSet(
        mergePaths(this->m_path, "age"), subjectSpec.age.value());
    initStatus = initStatus && ageStatus;
    if (ageStatus != Status::Success) {
      std::cerr << "Failed to create age dataset." << std::endl;
    } else {
      std::string ageReference = subjectSpec.ageReference.value_or("birth");
      Status ageRefStatus = ioPtr->createAttribute(
          ageReference, mergePaths(this->m_path, "age"), "reference");
      initStatus = initStatus && ageRefStatus;
      if (ageRefStatus != Status::Success) {
        std::cerr << "Failed to create age reference attribute." << std::endl;
      }
    }
  }

  // Initialize date_of_birth dataset if date_of_birth is provided
  if (subjectSpec.dateOfBirth.has_value()) {
    if (isISO8601Date(subjectSpec.dateOfBirth.value()) == false) {
      std::cerr << "Warning: date_of_birth is not in ISO8601 format: "
                << subjectSpec.dateOfBirth.value() << std::endl;
      initStatus = Status::Failure;
    } else {
      Status dobStatus =
          ioPtr->createStringDataSet(mergePaths(this->m_path, "date_of_birth"),
                                     subjectSpec.dateOfBirth.value());
      initStatus = initStatus && dobStatus;
      if (dobStatus != Status::Success) {
        std::cerr << "Failed to create date_of_birth dataset." << std::endl;
      }
    }
  }
  // Initialize description dataset if description is provided
  if (subjectSpec.description.has_value()) {
    Status descStatus =
        ioPtr->createStringDataSet(mergePaths(this->m_path, "description"),
                                   subjectSpec.description.value());
    initStatus = initStatus && descStatus;
    if (descStatus != Status::Success) {
      std::cerr << "Failed to create description dataset." << std::endl;
    }
  }

  // Initialize genotype dataset if genotype is provided
  if (subjectSpec.genotype.has_value()) {
    Status genotypeStatus = ioPtr->createStringDataSet(
        mergePaths(this->m_path, "genotype"), subjectSpec.genotype.value());
    initStatus = initStatus && genotypeStatus;
    if (genotypeStatus != Status::Success) {
      std::cerr << "Failed to create genotype dataset." << std::endl;
    }
  }

  // Initialize sex dataset if sex is provided
  if (subjectSpec.sex.has_value()) {
    Status sexStatus = ioPtr->createStringDataSet(
        mergePaths(this->m_path, "sex"), subjectSpec.sex.value());
    initStatus = initStatus && sexStatus;
    if (sexStatus != Status::Success) {
      std::cerr << "Failed to create sex dataset." << std::endl;
    }
  }

  // Initialize species dataset if species is provided
  if (subjectSpec.species.has_value()) {
    Status speciesStatus = ioPtr->createStringDataSet(
        mergePaths(this->m_path, "species"), subjectSpec.species.value());
    initStatus = initStatus && speciesStatus;
    if (speciesStatus != Status::Success) {
      std::cerr << "Failed to create species dataset." << std::endl;
    }
  }

  // Initialize strain dataset if strain is provided
  if (subjectSpec.strain.has_value()) {
    Status strainStatus = ioPtr->createStringDataSet(
        mergePaths(this->m_path, "strain"), subjectSpec.strain.value());
    initStatus = initStatus && strainStatus;
    if (strainStatus != Status::Success) {
      std::cerr << "Failed to create strain dataset." << std::endl;
    }
  }

  // Initialize subject_id dataset if subject_id is provided
  if (subjectSpec.subjectId.has_value()) {
    Status subjectIdStatus = ioPtr->createStringDataSet(
        mergePaths(this->m_path, "subject_id"), subjectSpec.subjectId.value());
    initStatus = initStatus && subjectIdStatus;
    if (subjectIdStatus != Status::Success) {
      std::cerr << "Failed to create subject_id dataset." << std::endl;
    }
  }

  // Initialize weight dataset if weight is provided
  if (subjectSpec.weight.has_value()) {
    Status weightStatus = ioPtr->createStringDataSet(
        mergePaths(this->m_path, "weight"), subjectSpec.weight.value());
    initStatus = initStatus && weightStatus;
    if (weightStatus != Status::Success) {
      std::cerr << "Failed to create weight dataset." << std::endl;
    }
  }

  // Return the overall status of the initialization
  return initStatus;
}
