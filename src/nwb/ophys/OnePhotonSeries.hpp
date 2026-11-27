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
 * @brief Image stack recorded over time from 1-photon microscope.
 *
 * Use Ophys::createOnePhotonSeries to create the series in the acquisition
 * group and ImageSeries::writeFrames to record frames.
 */
class OnePhotonSeries : public ImageSeries
{
public:
  // Register the OnePhotonSeries as a subclass of ImageSeries
  REGISTER_SUBCLASS(OnePhotonSeries,
                    ImageSeries,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a OnePhotonSeries.
   */
  struct Spec : public ImageSeries::Spec
  {
    /// @brief Photomultiplier gain. Optional.
    DEFINE_SPEC_FIELD(
        Spec, std::optional<float>, pmtGain, withPmtGain, std::nullopt)

    /// @brief Lines imaged per second. Optional.
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<float>,
                      scanLineRate,
                      withScanLineRate,
                      std::nullopt)

    /// @brief Exposure time of the sample, in seconds. Optional.
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<float>,
                      exposureTime,
                      withExposureTime,
                      std::nullopt)

    /// @brief Amount of pixels combined into bins. Optional.
    DEFINE_SPEC_FIELD(
        Spec, std::optional<uint8_t>, binning, withBinning, std::nullopt)

    /// @brief Power of the excitation, in mW. Optional.
    DEFINE_SPEC_FIELD(
        Spec, std::optional<float>, power, withPower, std::nullopt)

    /// @brief Intensity of the excitation, in mW/mm^2. Optional.
    DEFINE_SPEC_FIELD(
        Spec, std::optional<float>, intensity, withIntensity, std::nullopt)

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
     * the imagingPlane is set.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~OnePhotonSeries() override;

  /**
   * @brief Initializes the OnePhotonSeries from a spec.
   *
   * In addition to the ImageSeries datasets and attributes, creates the
   * optional attributes, and the link to the imaging
   * plane.
   *
   * @param spec The specification of the OnePhotonSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_ATTRIBUTE_FIELD(readPmtGain, float, "pmt_gain", Photomultiplier gain)

  DEFINE_ATTRIBUTE_FIELD(readScanLineRate,
                         float,
                         "scan_line_rate",
                         Lines imaged per second)

  DEFINE_ATTRIBUTE_FIELD(readExposureTime,
                         float,
                         "exposure_time",
                         Exposure time of the sample in seconds)

  DEFINE_ATTRIBUTE_FIELD(readBinning,
                         uint8_t,
                         "binning",
                         Amount of pixels combined into bins)

  DEFINE_ATTRIBUTE_FIELD(readPower,
                         float,
                         "power",
                         Power of the excitation in mW)

  DEFINE_ATTRIBUTE_FIELD(readIntensity,
                         float,
                         "intensity",
                         Intensity of the excitation in mW / mm ^ 2)

  DEFINE_REGISTERED_FIELD(readImagingPlane,
                          ImagingPlane,
                          "imaging_plane",
                          The imaging plane from which the data was generated)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the OnePhotonSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  OnePhotonSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
