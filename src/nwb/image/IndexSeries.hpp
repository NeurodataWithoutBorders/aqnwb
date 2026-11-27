#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/base/Images.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/image/ImageSeries.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Stores indices to image frames stored in an Images collection, e.g.,
 * the order in which images of a stimulus set were presented.
 *
 * The data are uint32 indices [time] into the images referenced by the
 * order_of_images of the indexed Images.
 */
class IndexSeries : public TimeSeries
{
public:
  // Register the IndexSeries as a subclass of TimeSeries
  REGISTER_SUBCLASS(IndexSeries, TimeSeries, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an IndexSeries.
   *
   * The unit is fixed to "N/A" by the schema and set by default.
   */
  struct Spec : public TimeSeries::Spec
  {
    REGISTER_SPEC(IndexSeries, TimeSeries::Spec)

    /**
     * @brief Constructor. Sets the unit to "N/A" as required by the schema.
     */
    Spec() { unit = "N/A"; }

    /// @brief Link to the Images collection with the indexed images.
    DEFINE_SPEC_FIELD(Spec,
                      std::shared_ptr<Images>,
                      indexedImages,
                      withIndexedImages,
                      nullptr)

    /**
     * @brief Link to the ImageSeries with the indexed frames. DEPRECATED in
     * favor of indexedImages.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::shared_ptr<ImageSeries>,
                      indexedTimeseries,
                      withIndexedTimeseries,
                      nullptr)

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
     * the unit is "N/A", that the data is a 1D uint32 array (unless the data
     * is a link), and that indexedImages or indexedTimeseries is set.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~IndexSeries() override;

  /**
   * @brief Initializes the IndexSeries from a spec.
   * @param spec The specification of the IndexSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Append indices to the data (and timestamps) of the series.
   * @param numSamples The number of indices to write.
   * @param indices Pointer to the numSamples indices.
   * @param timestampsInput Pointer to the numSamples timestamps (float64). May
   * be nullptr if the series uses startingTime and startingTimeRate.
   * @return The status of the write operation.
   */
  Status writeIndices(const SizeType& numSamples,
                      const uint32_t* indices,
                      const void* timestampsInput = nullptr);

  DEFINE_REGISTERED_FIELD(readIndexedImages,
                          Images,
                          "indexed_images",
                          The Images collection with the indexed images)

  DEFINE_REGISTERED_FIELD(readIndexedTimeseries,
                          ImageSeries,
                          "indexed_timeseries",
                          The ImageSeries with the indexed frames)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the IndexSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  IndexSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief The number of indices written via writeIndices.
   */
  SizeType m_samplesRecorded = 0;
};
}  // namespace AQNWB::NWB
