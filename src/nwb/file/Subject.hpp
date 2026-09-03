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
  struct SubjectSpec
  {
    /// @brief Age of subject (e.g., "P90D" for 90 days post-natal).
    std::optional<std::string> age = std::nullopt;
    /// @brief Age is with reference to this event. Can be ‘birth’ or
    /// ‘gestational’. If reference is omitted, ‘birth’ is implied.
    std::optional<std::string> ageReference = std::nullopt;
    /// @brief Date of birth of subject as iso formatted date string
    std::optional<std::string> dateOfBirth = std::nullopt;
    /// @brief Description of subject and where subject came from.
    std::optional<std::string> description = std::nullopt;
    /// @brief Genetic strain. If absent, assume Wild Type (WT).
    std::optional<std::string> genotype = std::nullopt;
    /// @brief Biological sex of subject.
    std::optional<std::string> sex = std::nullopt;
    /// @brief Species of subject.
    std::optional<std::string> species = std::nullopt;
    /// @brief Strain of subject.
    std::optional<std::string> strain = std::nullopt;
    /// @brief ID of animal/person used/participating in experiment (lab
    /// convention).
    std::optional<std::string> subjectId = std::nullopt;
    /// @brief Weight at time of experiment.
    std::optional<std::string> weight = std::nullopt;

    /**
     * @brief Set the age of the subject.
     * @param value Age of subject (e.g., "P90D" for 90 days post-natal).
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withAge(std::string value)
    {
      age = std::move(value);
      return *this;
    }

    /**
     * @brief Set the age reference event of the subject.
     * @param value Age is with reference to this event. Can be ‘birth’ or
     * ‘gestational’. If reference is omitted, ‘birth’ is implied.
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withAgeReference(std::string value)
    {
      ageReference = std::move(value);
      return *this;
    }

    /**
     * @brief Set the date of birth of the subject.
     * @param value Date of birth of subject as iso formatted date string.
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withDateOfBirth(std::string value)
    {
      dateOfBirth = std::move(value);
      return *this;
    }

    /**
     * @brief Set the description of the subject.
     * @param value Description of subject and where subject came from.
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withDescription(std::string value)
    {
      description = std::move(value);
      return *this;
    }

    /**
     * @brief Set the genotype of the subject.
     * @param value Genetic strain. If absent, assume Wild Type (WT).
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withGenotype(std::string value)
    {
      genotype = std::move(value);
      return *this;
    }

    /**
     * @brief Set the biological sex of the subject.
     * @param value Biological sex of subject.
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withSex(std::string value)
    {
      sex = std::move(value);
      return *this;
    }

    /**
     * @brief Set the species of the subject.
     * @param value Species of subject.
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withSpecies(std::string value)
    {
      species = std::move(value);
      return *this;
    }

    /**
     * @brief Set the strain of the subject.
     * @param value Strain of subject.
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withStrain(std::string value)
    {
      strain = std::move(value);
      return *this;
    }

    /**
     * @brief Set the ID of the subject.
     * @param value ID of animal/person used/participating in experiment (lab
     * convention).
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withSubjectId(std::string value)
    {
      subjectId = std::move(value);
      return *this;
    }

    /**
     * @brief Set the weight of the subject.
     * @param value Weight at time of experiment.
     * @return Reference to this SubjectSpec, to allow method chaining.
     */
    SubjectSpec& withWeight(std::string value)
    {
      weight = std::move(value);
      return *this;
    }
  };

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
   * @param subjectSpec The SubjectSpec object with the subject metadata
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const SubjectSpec& subjectSpec);

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
