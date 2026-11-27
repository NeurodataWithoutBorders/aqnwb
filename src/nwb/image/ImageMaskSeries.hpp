#pragma once

#include <memory>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/image/ImageSeries.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An alpha mask that is applied to a presented visual stimulus.
 *
 * The data are the masks (0 means transparent, 1 means opaque) for the frames
 * of the masked ImageSeries.
 */
class ImageMaskSeries : public ImageSeries
{
public:
  // Register the ImageMaskSeries as a subclass of ImageSeries
  REGISTER_SUBCLASS(ImageMaskSeries,
                    ImageSeries,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an ImageMaskSeries.
   */
  struct Spec : public ImageSeries::Spec
  {
    REGISTER_SPEC(ImageMaskSeries, ImageSeries::Spec)

    /// @brief Link to the ImageSeries that the mask is applied to. Required.
    DEFINE_SPEC_FIELD(Spec,
                      std::shared_ptr<ImageSeries>,
                      maskedImageSeries,
                      withMaskedImageSeries,
                      nullptr)

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
     * the maskedImageSeries is set.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~ImageMaskSeries() override;

  /**
   * @brief Initializes the ImageMaskSeries from a spec.
   * @param spec The specification of the ImageMaskSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_REGISTERED_FIELD(readMaskedImageSeries,
                          ImageSeries,
                          "masked_imageseries",
                          The ImageSeries that the mask is applied to)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the ImageMaskSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  ImageMaskSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
