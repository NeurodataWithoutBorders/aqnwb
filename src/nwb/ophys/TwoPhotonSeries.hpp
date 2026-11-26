#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/image/ImageSeries.hpp"
#include "nwb/ophys/ImagingPlane.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Image stack recorded over time from 2-photon microscope.
 *
 * Use Ophys::createTwoPhotonSeries to create the series in the acquisition
 * group and ImageSeries::writeFrames to record frames.
 */
class TwoPhotonSeries : public ImageSeries
{
public:
  // Register the TwoPhotonSeries as a subclass of ImageSeries
  REGISTER_SUBCLASS(TwoPhotonSeries,
                    ImageSeries,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a TwoPhotonSeries.
   */
  struct Spec : public ImageSeries::Spec
  {
    REGISTER_SPEC(TwoPhotonSeries, ImageSeries::Spec)

    /// @brief Photomultiplier gain. Optional.
    DEFINE_SPEC_FIELD(
        Spec, std::optional<float>, pmtGain, withPmtGain, std::nullopt)

    /// @brief Lines imaged per second. Optional.
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<float>,
                      scanLineRate,
                      withScanLineRate,
                      std::nullopt)

    /**
     * @brief Physical dimensions of the imaging field of view in width,
     * height, and optionally depth directions, in meters. Optional.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::vector<float>>,
                      fieldOfView,
                      withFieldOfView,
                      std::nullopt)

    /// @brief Link to the ImagingPlane from which the data was generated.
    /// Required.
    DEFINE_SPEC_FIELD(Spec,
                      std::shared_ptr<ImagingPlane>,
                      imagingPlane,
                      withImagingPlane,
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
     * the imagingPlane is set and that fieldOfView has 2 or 3 elements.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~TwoPhotonSeries() override;

  /**
   * @brief Initializes the TwoPhotonSeries from a spec.
   *
   * In addition to the ImageSeries datasets and attributes, creates the
   * optional attributes and the field_of_view dataset, and the link to the
   * imaging plane.
   *
   * @param spec The specification of the TwoPhotonSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_ATTRIBUTE_FIELD(readPmtGain, float, "pmt_gain", Photomultiplier gain)

  DEFINE_ATTRIBUTE_FIELD(readScanLineRate,
                         float,
                         "scan_line_rate",
                         Lines imaged per second)

  DEFINE_DATASET_FIELD(readFieldOfView,
                       recordFieldOfView,
                       float,
                       "field_of_view",
                       Physical dimensions of the imaging field of view)

  DEFINE_REGISTERED_FIELD(readImagingPlane,
                          ImagingPlane,
                          "imaging_plane",
                          The imaging plane from which the data was generated)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the TwoPhotonSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  TwoPhotonSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
