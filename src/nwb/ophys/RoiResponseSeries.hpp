#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/hdmf/table/DynamicTableRegion.hpp"
#include "nwb/ophys/PlaneSegmentation.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief ROI responses over an imaging plane, e.g., the fluorescence or dF/F
 * of each ROI over time.
 *
 * The data is 1D [time] for a single ROI or 2D [time, rois]. The ROIs are
 * rows of a PlaneSegmentation, referenced by the `rois` DynamicTableRegion.
 */
class RoiResponseSeries : public TimeSeries
{
public:
  // Register the RoiResponseSeries as a subclass of TimeSeries
  REGISTER_SUBCLASS(RoiResponseSeries,
                    TimeSeries,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a RoiResponseSeries.
   */
  struct Spec : public TimeSeries::Spec
  {
    REGISTER_SPEC(RoiResponseSeries, TimeSeries::Spec)

    /// @brief The PlaneSegmentation with the ROIs of the series. Required.
    DEFINE_SPEC_FIELD(Spec,
                      std::shared_ptr<PlaneSegmentation>,
                      planeSegmentation,
                      withPlaneSegmentation,
                      nullptr)

    /**
     * @brief The (zero-based) row indices of the ROIs in the
     * planeSegmentation, one per column of the data. Required.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::vector<int32_t>, roiIndices, withRoiIndices, {})

    /// @brief Description of the ROIs of the series.
    DEFINE_SPEC_FIELD(Spec,
                      std::string,
                      roisDescription,
                      withRoisDescription,
                      "the ROIs of the RoiResponseSeries")

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
     * the planeSegmentation and roiIndices are set and that the data is a
     * numeric 1D array (for a single ROI) or 2D array with one column per ROI
     * (unless the data is a link).
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~RoiResponseSeries() override;

  /**
   * @brief Initializes the RoiResponseSeries from a spec.
   *
   * In addition to the TimeSeries datasets and attributes, creates the rois
   * DynamicTableRegion that references the ROIs in the planeSegmentation.
   *
   * @param spec The specification of the RoiResponseSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Append samples to the data (and timestamps) of the series.
   *
   * @param numSamples The number of samples (time points) to write.
   * @param dataInput Pointer to the samples, i.e., a contiguous array of
   * shape [numSamples] or [numSamples, rois] with the data type of the data.
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

  DEFINE_REGISTERED_FIELD(readRois,
                          DynamicTableRegion,
                          "rois",
                          The ROIs of the series)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the RoiResponseSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  RoiResponseSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief The shape of a single sample (data shape without the first
   * dimension).
   */
  SizeArray m_sampleShape;

  /**
   * @brief The number of samples written via writeSamples.
   */
  SizeType m_samplesRecorded = 0;
};
}  // namespace AQNWB::NWB
