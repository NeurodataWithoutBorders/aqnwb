#pragma once

#include <memory>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "nwb/device/Device.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief A site of optogenetic stimulation.
 *
 * Stimulation sites are stored in `/general/optogenetics/<name>`, e.g., via
 * Ogen::createOptogeneticStimulusSite.
 */
class OptogeneticStimulusSite : public NWBContainer
{
public:
  // Register the OptogeneticStimulusSite as a subclass of NWBContainer
  REGISTER_SUBCLASS(OptogeneticStimulusSite,
                    NWBContainer,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an OptogeneticStimulusSite.
   */
  struct Spec : public NWBContainer::Spec
  {
    REGISTER_SPEC(OptogeneticStimulusSite, NWBContainer::Spec)

    /// @brief Description of the stimulation site. Required.
    DEFINE_SPEC_FIELD(Spec, std::string, description, withDescription, "")

    /// @brief Excitation wavelength, in nm. Required.
    DEFINE_SPEC_FIELD(
        Spec, float, excitationLambda, withExcitationLambda, -1.0f)

    /**
     * @brief Location of the stimulation site, e.g., the brain area or layer.
     * Use standard atlas names for anatomical regions when possible.
     * Required.
     */
    DEFINE_SPEC_FIELD(Spec, std::string, location, withLocation, "")

    /// @brief Link to the Device that generated the stimulus. Required.
    DEFINE_SPEC_FIELD(
        Spec, std::shared_ptr<Device>, device, withDevice, nullptr)

    /**
     * @brief Validate the spec.
     *
     * Checks that the description and location are not empty, that the
     * excitation wavelength is positive, and that the device is set.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~OptogeneticStimulusSite() override;

  /**
   * @brief Initializes the OptogeneticStimulusSite from a spec.
   *
   * Creates the description, excitation_lambda, and location datasets and the
   * link to the device.
   *
   * @param spec The specification of the OptogeneticStimulusSite.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_DATASET_FIELD(readDescription,
                       recordDescription,
                       std::string,
                       "description",
                       Description of the stimulation site)

  DEFINE_DATASET_FIELD(readExcitationLambda,
                       recordExcitationLambda,
                       float,
                       "excitation_lambda",
                       Excitation wavelength in nm)

  DEFINE_DATASET_FIELD(readLocation,
                       recordLocation,
                       std::string,
                       "location",
                       Location of the stimulation site)

  DEFINE_REGISTERED_FIELD(readDevice,
                          Device,
                          "device",
                          The device that generated the stimulus)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the OptogeneticStimulusSite in the file.
   * @param io A shared pointer to the IO object.
   */
  OptogeneticStimulusSite(const std::string& path,
                          std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
