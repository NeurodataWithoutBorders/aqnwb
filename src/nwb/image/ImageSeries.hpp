#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/device/Device.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief General image data that is common between acquisition and stimulus
 * time series.
 *
 * The data is stored as a 3D [frame, x, y] or 4D [frame, x, y, z] array. If
 * the images are stored in external files (format "external"), then the data
 * should be an empty 3D array and the paths are stored in external_file.
 */
class ImageSeries : public TimeSeries
{
public:
  // Register the ImageSeries as a subclass of TimeSeries
  REGISTER_SUBCLASS(ImageSeries, TimeSeries, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an ImageSeries.
   *
   * Example:
   * @code
   * IO::ArrayDataSetConfig config(
   *     IO::BaseDataType::U16, {0, 512, 512}, {1, 512, 512});
   * auto spec = ImageSeries::Spec()
   *                 .withData(config)
   *                 .withUnit("n.a.")
   *                 .withDescription("Raw camera frames")
   *                 .withDevice(camera);
   * @endcode
   */
  struct Spec : public TimeSeries::Spec
  {
    REGISTER_SPEC(ImageSeries, TimeSeries::Spec)

    /**
     * @brief Format of the images. "raw" if the data is stored in the data
     * dataset and "external" if the images are stored in external files (see
     * externalFile).
     */
    DEFINE_SPEC_FIELD(Spec, std::string, format, withFormat, "raw")

    /// @brief Number of pixels on the x, y, (and z) axes. Optional.
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::vector<int32_t>>,
                      dimension,
                      withDimension,
                      std::nullopt)

    /**
     * @brief Paths to one or more external files. Only used (and required) if
     * format is "external".
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::vector<std::string>>,
                      externalFile,
                      withExternalFile,
                      std::nullopt)

    /**
     * @brief Index of the first frame of each external file in the full
     * ImageSeries (zero-based). Required if externalFile is set and must have
     * the same length as externalFile.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::vector<int32_t>,
                      externalFileStartingFrame,
                      withExternalFileStartingFrame,
                      {})

    /**
     * @brief Total number of frames across all external files. Only relevant
     * if format is "external" and the timing is described via startingTime
     * and startingTimeRate. Optional.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<uint32_t>, numSamples, withNumSamples, std::nullopt)

    /// @brief The device that was used to capture the images. Optional.
    DEFINE_SPEC_FIELD(
        Spec, std::shared_ptr<Device>, device, withDevice, nullptr)

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
     * the data is a 3D or 4D numeric array (unless the data is a link), that
     * the format is "raw" or "external", and that externalFile and
     * externalFileStartingFrame are consistent with the format.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~ImageSeries() override;

  /**
   * @brief Initializes the ImageSeries from a spec.
   *
   * In addition to the TimeSeries datasets and attributes, creates the format
   * dataset, the optional dimension, external_file, and num_samples datasets,
   * and the optional link to the device.
   *
   * @param spec The specification of the ImageSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Append frames to the data (and timestamps) of the ImageSeries.
   *
   * The frames are appended after the frames that have already been written
   * via this method.
   *
   * @param numFrames The number of frames to write.
   * @param dataInput Pointer to the frames, i.e., a contiguous array of shape
   * [numFrames, x, y(, z)] in row-major order with the data type of the data
   * dataset.
   * @param timestampsInput Pointer to the numFrames timestamps (float64). May
   * be nullptr if the series uses startingTime and startingTimeRate.
   * @param controlInput Pointer to the numFrames control values (uint8).
   * Optional.
   * @return The status of the write operation.
   */
  Status writeFrames(const SizeType& numFrames,
                     const void* dataInput,
                     const void* timestampsInput = nullptr,
                     const void* controlInput = nullptr);

  /**
   * @brief Get the shape of a single frame, i.e., the shape of the data
   * without the first (time) dimension.
   * @return The frame shape. Empty if the ImageSeries has not been initialized
   * in this session.
   */
  inline const SizeArray& getFrameShape() const { return m_frameShape; }

  /**
   * @brief Get the number of frames written via writeFrames.
   * @return The number of frames recorded.
   */
  inline SizeType getNumFramesRecorded() const { return m_framesRecorded; }

  DEFINE_DATASET_FIELD(readFormat,
                       recordFormat,
                       std::string,
                       "format",
                       Format of image.raw or external)

  DEFINE_DATASET_FIELD(readDimension,
                       recordDimension,
                       int32_t,
                       "dimension",
                       Number of pixels on x y and z axes)

  DEFINE_DATASET_FIELD(readExternalFile,
                       recordExternalFile,
                       std::string,
                       "external_file",
                       Paths to one or more external files)

  DEFINE_ATTRIBUTE_FIELD(readExternalFileStartingFrame,
                         int32_t,
                         "external_file/starting_frame",
                         Index of the first frame of each external file)

  DEFINE_DATASET_FIELD(readNumSamples,
                       recordNumSamples,
                       uint32_t,
                       "num_samples",
                       Total number of frames across all external files)

  DEFINE_REGISTERED_FIELD(readDevice,
                          Device,
                          "device",
                          The device used to capture the images)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the ImageSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  ImageSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief The shape of a single frame (data shape without the first
   * dimension).
   */
  SizeArray m_frameShape;

  /**
   * @brief The number of frames written via writeFrames.
   */
  SizeType m_framesRecorded = 0;
};
}  // namespace AQNWB::NWB
