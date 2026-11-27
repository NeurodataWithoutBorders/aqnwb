#pragma once

#include <memory>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An optical channel used to record from an imaging plane.
 *
 * OpticalChannel objects are created as part of an ImagingPlane via
 * ImagingPlane::Spec::withOpticalChannel.
 */
class OpticalChannel : public NWBContainer
{
public:
  // Register the OpticalChannel as a subclass of NWBContainer
  REGISTER_SUBCLASS(OpticalChannel,
                    NWBContainer,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an OpticalChannel.
   */
  struct Spec : public NWBContainer::Spec
  {
    /// @brief Description or other notes about the channel. Required.
    DEFINE_SPEC_FIELD(Spec, std::string, description, withDescription, "")

    /// @brief Emission wavelength for the channel, in nm. Required.
    DEFINE_SPEC_FIELD(Spec, float, emissionLambda, withEmissionLambda, -1.0f)

    /**
     * @brief Validate the spec.
     *
     * Checks that the description is not empty and that the emission
     * wavelength is positive.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~OpticalChannel() override;

  /**
   * @brief Initializes the OpticalChannel from a spec.
   *
   * Creates the description and emission_lambda datasets.
   *
   * @param spec The specification of the OpticalChannel.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_DATASET_FIELD(readDescription,
                       recordDescription,
                       std::string,
                       "description",
                       Description or other notes about the channel)

  DEFINE_DATASET_FIELD(readEmissionLambda,
                       recordEmissionLambda,
                       float,
                       "emission_lambda",
                       Emission wavelength for channel in nm)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the OpticalChannel in the file.
   * @param io A shared pointer to the IO object.
   */
  OpticalChannel(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
