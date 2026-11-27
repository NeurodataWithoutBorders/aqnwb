#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "io/BaseIO.hpp"
#include "nwb/image/ImageSeries.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Image data that is presented or recorded, e.g., a visual stimulus.
 *
 * The data is 3D [frame, x, y] or 4D [frame, x, y, 3] for RGB color images.
 */
class OpticalSeries : public ImageSeries
{
public:
  // Register the OpticalSeries as a subclass of ImageSeries
  REGISTER_SUBCLASS(OpticalSeries,
                    ImageSeries,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an OpticalSeries.
   */
  struct Spec : public ImageSeries::Spec
  {
    /// @brief Distance from the camera or monitor to the target, in meters.
    DEFINE_SPEC_FIELD(
        Spec, std::optional<float>, distance, withDistance, std::nullopt)

    /**
     * @brief Width, height, and optionally depth of the image (or the field
     * of view), in meters. Optional.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::vector<float>>,
                      fieldOfView,
                      withFieldOfView,
                      std::nullopt)

    /**
     * @brief Description of the image relative to some reference frame (e.g.,
     * which way is up). Optional.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::string>,
                      orientation,
                      withOrientation,
                      std::nullopt)

    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withData)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withUnit)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withDescription)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withComments)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withConversion)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withResolution)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withOffset)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withContinuity)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withStartingTime)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withStartingTimeRate)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withControlDescription)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withFormat)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withDimension)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withExternalFile)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withExternalFileStartingFrame)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withNumSamples)
    INHERIT_SPEC_SETTER(Spec, ImageSeries::Spec, withDevice)

    /**
     * @brief Validate the spec.
     *
     * In addition to the checks of ImageSeries::Spec::validate(), checks that
     * fieldOfView has 2 or 3 elements and that 4D data has 3 color channels.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~OpticalSeries() override;

  /**
   * @brief Initializes the OpticalSeries from a spec.
   * @param spec The specification of the OpticalSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_DATASET_FIELD(readDistance,
                       recordDistance,
                       float,
                       "distance",
                       Distance from camera or monitor to target in meters)

  DEFINE_DATASET_FIELD(readFieldOfView,
                       recordFieldOfView,
                       float,
                       "field_of_view",
                       Width height and depth of the image in meters)

  DEFINE_DATASET_FIELD(readOrientation,
                       recordOrientation,
                       std::string,
                       "orientation",
                       Description of the image relative to a reference frame)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the OpticalSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  OpticalSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
