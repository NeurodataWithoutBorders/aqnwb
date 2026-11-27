#pragma once

#include <memory>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/base/NWBDataInterface.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/image/ImageSeries.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Results from motion correction of an image stack.
 *
 * Stores the corrected ImageSeries (`corrected`), the x/y translation of each
 * frame (`xy_translation`), and a link to the original ImageSeries. The
 * `corrected` and `xy_translation` series are created by initialize from the
 * nested specs and can be accessed for recording via readCorrected and
 * readXyTranslation.
 */
class CorrectedImageStack : public NWBDataInterface
{
public:
  // Register the CorrectedImageStack as a subclass of NWBDataInterface
  REGISTER_SUBCLASS(CorrectedImageStack,
                    NWBDataInterface,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a CorrectedImageStack.
   */
  struct Spec : public NWBDataInterface::Spec
  {
    REGISTER_SPEC(CorrectedImageStack, NWBDataInterface::Spec)

    /// @brief Link to the original (uncorrected) ImageSeries. Required.
    DEFINE_SPEC_FIELD(
        Spec, std::shared_ptr<ImageSeries>, original, withOriginal, nullptr)

    /// @brief Spec of the motion-corrected ImageSeries. Required.
    DEFINE_SPEC_FIELD(Spec, ImageSeries::Spec, corrected, withCorrected, {})

    /**
     * @brief Spec of the TimeSeries with the x and y translation (in pixels)
     * of each frame, i.e., data of shape [frames, 2]. Required.
     */
    DEFINE_SPEC_FIELD(
        Spec, TimeSeries::Spec, xyTranslation, withXyTranslation, {})

    /**
     * @brief Validate the spec.
     *
     * Checks that the original is set, that the nested specs are valid, and
     * that the data of xyTranslation is 2D with 2 columns.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~CorrectedImageStack() override;

  /**
   * @brief Initializes the CorrectedImageStack from a spec.
   *
   * Creates the corrected ImageSeries, the xy_translation TimeSeries, and the
   * link to the original ImageSeries.
   *
   * @param spec The specification of the CorrectedImageStack.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_REGISTERED_FIELD(readCorrected,
                          ImageSeries,
                          "corrected",
                          The motion - corrected image stack)

  DEFINE_REGISTERED_FIELD(readXyTranslation,
                          TimeSeries,
                          "xy_translation",
                          The x and y translation of each frame in pixels)

  DEFINE_REGISTERED_FIELD(readOriginal,
                          ImageSeries,
                          "original",
                          The original ImageSeries)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the CorrectedImageStack in the file.
   * @param io A shared pointer to the IO object.
   */
  CorrectedImageStack(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
