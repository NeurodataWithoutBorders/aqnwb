#pragma once

#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief TimeSeries storing text-based records about the experiment.
 */
class AnnotationSeries : public TimeSeries
{
public:
  // Register the AnnotationSeries
  REGISTER_SUBCLASS(AnnotationSeries,
                    TimeSeries,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an AnnotationSeries.
   *
   * The unit is fixed to "n/a" by the schema and set by default. The
   * conversion, resolution, and offset are fixed to 1.0, -1.0, and 0.0,
   * respectively, and the values set in the spec are ignored. The data type
   * of the data configuration must be a variable-length string.
   */
  struct Spec : public TimeSeries::Spec
  {
    /**
     * @brief Constructor. Sets the unit to "n/a" as required by the schema.
     */
    Spec() { unit = "n/a"; }

    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withData)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withUnit)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withDescription)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withComments)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withConversion)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withResolution)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withOffset)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withContinuity)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withStartingTime)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withStartingTimeRate)
    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withControlDescription)

    /**
     * @brief Validate the spec.
     *
     * In addition to the checks of TimeSeries::Spec::validate(), checks that
     * the unit is "n/a".
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

protected:
  /**
   * @brief Constructor.
   * @param path The location of the AnnotationSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  AnnotationSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Destructor
   */
  ~AnnotationSeries() override;

  /**
   * @brief Initializes the AnnotationSeries
   * @param description The description of the AnnotationSeries.
   * @param comments Comments about the AnnotationSeries.
   * @param dataConfig Configuration for the dataset including shape and
   * chunking. The shape must be a vector with one element specifying the length
   * in time. The data type is fixed to variable-length string according to
   * schema.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  [[deprecated(
      "Use AnnotationSeries::initialize(const AnnotationSeries::Spec&) "
      "instead")]]
  Status initialize(const std::string& description,
                    const std::string& comments,
                    const IO::BaseArrayDataSetConfig& dataConfig);

  /**
   * @brief Initializes the AnnotationSeries from a spec.
   * @param spec The specification of the AnnotationSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Writes a channel to an AnnotationSeries dataset.
   * @param numSamples The number of samples to write (length in time).
   * @param dataInput A vector of strings.
   * @param timestampsInput A pointer to the timestamps block.
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeAnnotation(const SizeType& numSamples,
                         const std::vector<std::string>& dataInput,
                         const void* timestampsInput,
                         const void* controlInput = nullptr);

  DEFINE_DATASET_FIELD(readData,
                       recordData,
                       std::string,
                       "data",
                       Annotations made during an experiment.)

private:
  /**
   * @brief The number of samples already written per channel.
   */
  SizeType m_samplesRecorded = 0;
};
}  // namespace AQNWB::NWB
