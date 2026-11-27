#pragma once

#include <memory>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/ogen/OptogeneticStimulusSite.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An optogenetic stimulus, i.e., the applied power in watts.
 *
 * The data is 1D [time] or 2D [time, sites]. 2D data is meant to be used in
 * extensions of OptogeneticSeries that define what the second dimension
 * represents.
 */
class OptogeneticSeries : public TimeSeries
{
public:
  // Register the OptogeneticSeries as a subclass of TimeSeries
  REGISTER_SUBCLASS(OptogeneticSeries,
                    TimeSeries,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an OptogeneticSeries.
   *
   * The unit is fixed to "watts" by the schema. The spec sets it by default
   * and does not re-expose withUnit. validate() fails for other units.
   */
  struct Spec : public TimeSeries::Spec
  {
    /**
     * @brief Constructor. Sets the unit to "watts" as required by the schema.
     */
    Spec() { unit = "watts"; }

    /// @brief Link to the OptogeneticStimulusSite of the stimulus. Required.
    DEFINE_SPEC_FIELD(
        Spec, std::shared_ptr<OptogeneticStimulusSite>, site, withSite, nullptr)

    INHERIT_SPEC_SETTER(Spec, TimeSeries::Spec, withData)
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
     * the unit is "watts", that the site is set, and that the data is a 1D or
     * 2D numeric array (unless the data is a link).
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~OptogeneticSeries() override;

  /**
   * @brief Initializes the OptogeneticSeries from a spec.
   *
   * In addition to the TimeSeries datasets and attributes, creates the link to
   * the stimulation site.
   *
   * @param spec The specification of the OptogeneticSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Append samples to the data (and timestamps) of the series.
   *
   * @param numSamples The number of samples (time points) to write.
   * @param dataInput Pointer to the samples, i.e., a contiguous array of
   * shape [numSamples] or [numSamples, sites] with the data type of the data.
   * @param timestampsInput Pointer to the numSamples timestamps (float64). May
   * be nullptr if the series uses startingTime and startingTimeRate.
   * @param controlInput Pointer to the numSamples control values (uint8).
   * Optional.
   * @return The status of the write operation.
   */
  Status writeSamples(const SizeType& numSamples,
                      const void* dataInput,
                      const void* timestampsInput = nullptr,
                      const void* controlInput = nullptr);

  DEFINE_REGISTERED_FIELD(readSite,
                          OptogeneticStimulusSite,
                          "site",
                          The stimulation site of the stimulus)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the OptogeneticSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  OptogeneticSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
