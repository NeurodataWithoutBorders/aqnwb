#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ophys/OpticalChannel.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An imaging plane and its metadata.
 *
 * Imaging planes are stored in `/general/optophysiology/<name>`, e.g., via
 * Ophys::createImagingPlane. They own one or more OpticalChannel objects and
 * link to the Device used for imaging.
 *
 * @note The deprecated `manifold` dataset is not supported. Use originCoords,
 * gridSpacing, and referenceFrame instead.
 */
class ImagingPlane : public NWBContainer
{
public:
  // Register the ImagingPlane as a subclass of NWBContainer
  REGISTER_SUBCLASS(ImagingPlane,
                    NWBContainer,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an ImagingPlane.
   *
   * Example:
   * @code
   * auto spec = ImagingPlane::Spec()
   *                 .withDescription("Imaging plane in V1")
   *                 .withExcitationLambda(920.0f)
   *                 .withIndicator("GCaMP6s")
   *                 .withLocation("V1")
   *                 .withImagingRate(30.0f)
   *                 .withDevice(microscope)
   *                 .withOpticalChannel("green",
   *                                     OpticalChannel::Spec()
   *                                         .withDescription("Green channel")
   *                                         .withEmissionLambda(510.0f));
   * @endcode
   */
  struct Spec : public NWBContainer::Spec
  {
    /// @brief List of (name, spec) pairs of the optical channels
    using NamedOpticalChannels =
        std::vector<std::pair<std::string, OpticalChannel::Spec>>;

    /// @brief Description of the imaging plane. Optional.
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::string>,
                      description,
                      withDescription,
                      std::nullopt)

    /**
     * @brief Excitation wavelength, in nm. Required. Use NaN (e.g.,
     * `std::numeric_limits<float>::quiet_NaN()`) if the wavelength is unknown.
     */
    DEFINE_SPEC_FIELD(
        Spec, float, excitationLambda, withExcitationLambda, -1.0f)

    /**
     * @brief Rate that images are acquired, in Hz. If the corresponding
     * TimeSeries is present, the rate should be stored there instead.
     * Optional.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<float>, imagingRate, withImagingRate, std::nullopt)

    /// @brief Calcium indicator, e.g., "GCaMP6s". Required.
    DEFINE_SPEC_FIELD(Spec, std::string, indicator, withIndicator, "")

    /**
     * @brief Location of the imaging plane, e.g., the brain area or layer.
     * Use standard atlas names for anatomical regions when possible.
     * Required.
     */
    DEFINE_SPEC_FIELD(Spec, std::string, location, withLocation, "")

    /**
     * @brief Physical location of the first element of the imaging plane,
     * (0, 0) for 2-D data or (0, 0, 0) for 3-D data. Optional.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::vector<float>>,
                      originCoords,
                      withOriginCoords,
                      std::nullopt)

    /// @brief Measurement units for originCoords.
    DEFINE_SPEC_FIELD(
        Spec, std::string, originCoordsUnit, withOriginCoordsUnit, "meters")

    /**
     * @brief Space between pixels in (x, y) or voxels in (x, y, z) directions,
     * in the unit gridSpacingUnit. Optional.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::vector<float>>,
                      gridSpacing,
                      withGridSpacing,
                      std::nullopt)

    /// @brief Measurement units for gridSpacing.
    DEFINE_SPEC_FIELD(
        Spec, std::string, gridSpacingUnit, withGridSpacingUnit, "meters")

    /**
     * @brief Describes the reference frame of originCoords and gridSpacing,
     * e.g., "Origin coordinates are relative to bregma". Optional.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::string>,
                      referenceFrame,
                      withReferenceFrame,
                      std::nullopt)

    /**
     * @brief The optical channels of the imaging plane as (name, spec) pairs.
     * At least one optical channel is required.
     */
    DEFINE_SPEC_FIELD(
        Spec, NamedOpticalChannels, opticalChannels, withOpticalChannels, {})

    /// @brief Link to the Device used for imaging. Required.
    DEFINE_SPEC_FIELD(
        Spec, std::shared_ptr<Device>, device, withDevice, nullptr)

    /**
     * @brief Add an optical channel to the imaging plane.
     * @param name The name of the OpticalChannel group to create.
     * @param channelSpec The specification of the optical channel.
     * @return Reference to this spec to allow chaining.
     */
    Spec& withOpticalChannel(const std::string& name,
                             const OpticalChannel::Spec& channelSpec)
    {
      opticalChannels.emplace_back(name, channelSpec);
      return *this;
    }

    /**
     * @brief Validate the spec.
     *
     * Checks the required fields, the length of originCoords and gridSpacing
     * (2 or 3), and the specs of the optical channels.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~ImagingPlane() override;

  /**
   * @brief Initializes the ImagingPlane from a spec.
   *
   * Creates the datasets of the imaging plane, the OpticalChannel groups, and
   * the link to the device.
   *
   * @param spec The specification of the ImagingPlane.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_DATASET_FIELD(readDescription,
                       recordDescription,
                       std::string,
                       "description",
                       Description of the imaging plane)

  DEFINE_DATASET_FIELD(readExcitationLambda,
                       recordExcitationLambda,
                       float,
                       "excitation_lambda",
                       Excitation wavelength in nm)

  DEFINE_DATASET_FIELD(readImagingRate,
                       recordImagingRate,
                       float,
                       "imaging_rate",
                       Rate that images are acquired in Hz)

  DEFINE_DATASET_FIELD(readIndicator,
                       recordIndicator,
                       std::string,
                       "indicator",
                       Calcium indicator)

  DEFINE_DATASET_FIELD(readLocation,
                       recordLocation,
                       std::string,
                       "location",
                       Location of the imaging plane)

  DEFINE_DATASET_FIELD(readOriginCoords,
                       recordOriginCoords,
                       float,
                       "origin_coords",
                       Physical location of the first element of the plane)

  DEFINE_ATTRIBUTE_FIELD(readOriginCoordsUnit,
                         std::string,
                         "origin_coords/unit",
                         Measurement units for origin_coords)

  DEFINE_DATASET_FIELD(readGridSpacing,
                       recordGridSpacing,
                       float,
                       "grid_spacing",
                       Space between pixels or voxels)

  DEFINE_ATTRIBUTE_FIELD(readGridSpacingUnit,
                         std::string,
                         "grid_spacing/unit",
                         Measurement units for grid_spacing)

  DEFINE_DATASET_FIELD(readReferenceFrame,
                       recordReferenceFrame,
                       std::string,
                       "reference_frame",
                       Reference frame of origin_coords and grid_spacing)

  DEFINE_UNNAMED_REGISTERED_FIELD(readOpticalChannel,
                                  createOpticalChannel,
                                  OpticalChannel,
                                  "",
                                  An optical channel of the imaging plane)

  DEFINE_REGISTERED_FIELD(readDevice,
                          Device,
                          "device",
                          The device used for imaging)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the ImagingPlane in the file.
   * @param io A shared pointer to the IO object.
   */
  ImagingPlane(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
