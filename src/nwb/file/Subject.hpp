#pragma once

// Common STL includes
#include <memory>
#include <optional>
#include <string>
#include <vector>
// Base AqNWB includes for IO and RegisteredType
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/RegisteredType.hpp"
// Include for parent type
#include "nwb/base/NWBContainer.hpp"
// Include for the namespace schema header
#include "spec/core.hpp"

namespace AQNWB::NWB
{

/**
 * @brief Information about the animal or person from which the data was
 * measured.
 */
class Subject : public AQNWB::NWB::NWBContainer
{
public:
  REGISTER_SUBCLASS(Subject, NWBContainer, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Metadata about the experimental subject.
   *
   * All fields are optional. Pass an instance of this struct to
   * Subject::initialize() to create a Subject group in the NWB file.
   */
  struct Spec : public NWBContainer::Spec
  {
    /**
     * @brief Age of subject (e.g., "P90D" for 90 days post-natal).
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<std::string>, age, withAge, std::nullopt)
    /**
     * @brief Age is with reference to this event. Can be "birth" or
     * "gestational". If reference is omitted then "birth" is implied.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::string>,
                      ageReference,
                      withAgeReference,
                      std::nullopt)
    /**
     * @brief Date of birth of subject as iso formatted date string
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::string>,
                      dateOfBirth,
                      withDateOfBirth,
                      std::nullopt)
    /**
     * @brief Description of subject and where subject came from.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::string>,
                      description,
                      withDescription,
                      std::nullopt)
    /**
     * @brief Genetic strain. If absent, assume Wild Type (WT).
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<std::string>, genotype, withGenotype, std::nullopt)
    /**
     * @brief Biological sex of subject.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<std::string>, sex, withSex, std::nullopt)
    /**
     * @brief Species of subject.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<std::string>, species, withSpecies, std::nullopt)
    /**
     * @brief Strain of subject.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<std::string>, strain, withStrain, std::nullopt)
    /**
     * @brief ID of animal/person used/participating in experiment (lab
     * convention).
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::string>,
                      subjectId,
                      withSubjectId,
                      std::nullopt)
    /**
     * @brief Weight at time of experiment.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<std::string>, weight, withWeight, std::nullopt)
  };

  /**
   * @brief Alias for Subject::Spec for backward compatibility.
   */
  using SubjectSpec [[deprecated("Use Subject::Spec instead")]] = Spec;

  /** \brief Convenience factor method since the path is fixed to
   * '/general/subject'
   * @param io A shared pointer to the IO object.
   * @return A shared pointer to the created NWBFile object, or nullptr if
   * creation failed.
   */
  static std::shared_ptr<Subject> create(std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~Subject() override {}

  /**
   * @brief Initialize the object
   * @param subjectSpec The Subject::Spec object with the subject metadata
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& subjectSpec);

  // Define read methods
  DEFINE_DATASET_FIELD(
      readAge,
      recordAge,
      std::string,
      "age",
      "Age of subject. Can be supplied instead of date_of_birth.")

  DEFINE_DATASET_FIELD(
      readDateOfBirth,
      recordDateOfBirth,
      std::string,
      "date_of_birth",
      "Date of birth of subject. Can be supplied instead of age.")

  DEFINE_DATASET_FIELD(readDescription,
                       recordDescription,
                       std::string,
                       "description",
                       "Description of subject and where subject came from "
                       "(e.g. - breeder - if animal).")

  DEFINE_DATASET_FIELD(readGenotype,
                       recordGenotype,
                       std::string,
                       "genotype",
                       "Genetic strain. If absent - assume Wild Type (WT).")

  DEFINE_DATASET_FIELD(
      readSex, recordSex, std::string, "sex", "Gender of subject.")

  DEFINE_DATASET_FIELD(
      readSpecies, recordSpecies, std::string, "species", "Species of subject.")

  DEFINE_DATASET_FIELD(
      readStrain, recordStrain, std::string, "strain", "Strain of subject.")

  DEFINE_DATASET_FIELD(
      readSubjectId,
      recordSubjectId,
      std::string,
      "subject_id",
      "ID of animal/person used/participating in experiment (lab convention).")

  DEFINE_DATASET_FIELD(readWeight,
                       recordWeight,
                       std::string,
                       "weight",
                       "Weight at time of experiment - at time of surgery and "
                       "at other important times.")

  DEFINE_ATTRIBUTE_FIELD(
      readAgeReference,
      std::string,
      "age/reference",
      "Age is with reference to this event. Can be birth or gestational. If "
      "reference is omitted - birth is implied.")

protected:
  /**
   * @brief Constructor for Subject.
   * @param io The shared pointer to the IO object.
   */
  explicit Subject(std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  Subject(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io);
};

}  // namespace AQNWB::NWB
