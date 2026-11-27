#pragma once

#include <memory>
#include <optional>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/RegisteredTypeSpec.hpp"
#include "nwb/base/NWBData.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An abstract base type for image data. Parent type for Image.
 */
class BaseImage : public NWBData
{
public:
  // Register the BaseImage as a subclass of NWBData
  REGISTER_SUBCLASS(BaseImage, NWBData, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a BaseImage.
   */
  struct Spec : public RegisteredTypeSpec
  {
    /// @brief Shared pointer to a dataset configuration (const)
    using DataConfigPtr = std::shared_ptr<const IO::BaseArrayDataSetConfig>;

    /// @brief Configuration of the image dataset. Required.
    DEFINE_SPEC_FIELD(Spec, DataConfigPtr, data, withData, nullptr)

    /**
     * @brief Set the configuration of the image dataset.
     *
     * Stores a copy of the configuration via
     * IO::BaseArrayDataSetConfig::clone (see TimeSeries::Spec::withData).
     *
     * @param config The configuration of the image dataset.
     * @return Reference to this spec to allow chaining.
     */
    Spec& withData(const IO::BaseArrayDataSetConfig& config)
    {
      data = config.clone();
      return *this;
    }

    /// @brief Description of the image. Optional.
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::string>,
                      description,
                      withDescription,
                      std::nullopt)

    /**
     * @brief Validate the spec, i.e., check that the data is set.
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~BaseImage() override;

  /**
   * @brief Initialize the image dataset from a spec.
   *
   * Validates the spec before anything is written.
   *
   * @param spec The specification of the image.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         Description of the image)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the image in the file.
   * @param io A shared pointer to the IO object.
   */
  BaseImage(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};

/**
 * @brief A type for storing image data directly. The shape can be 2D [x, y]
 * for grayscale images, or 3D [x, y, 3] / [x, y, 4] for RGB / RGBA images.
 */
class Image : public BaseImage
{
public:
  // Register the Image as a subclass of BaseImage
  REGISTER_SUBCLASS(Image, BaseImage, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an Image (and its subtypes, e.g.,
   * GrayscaleImage).
   */
  struct Spec : public BaseImage::Spec
  {
    /**
     * @brief Pixel resolution of the image, in pixels per centimeter.
     * Optional.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::optional<float>, resolution, withResolution, std::nullopt)

    INHERIT_SPEC_SETTER(Spec, BaseImage::Spec, withData)
    INHERIT_SPEC_SETTER(Spec, BaseImage::Spec, withDescription)

    /**
     * @brief Validate the spec.
     *
     * Checks that the data is set and numeric and that the resolution (if
     * set) is > 0. The shape is checked by the image type (see checkSpec).
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~Image() override;

  /**
   * @brief Check a spec against the type of this image and the file.
   *
   * Validates the spec and checks that the shape of the data is valid for the
   * type of the image (e.g., [x, y, 3] for an RGBImage). Used by initialize
   * and Images::initialize before anything is written.
   *
   * @param spec The specification of the image.
   * @return Status::Success if the spec can be used for this image.
   */
  Status checkSpec(const Spec& spec) const;

  /**
   * @brief Initialize the image dataset from a spec.
   *
   * Checks the spec (see checkSpec) before anything is written.
   *
   * @param spec The specification of the image.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Write the pixel values of the image.
   * @param shape The shape of the image (must match the dataset shape).
   * @param dataType The data type of the values.
   * @param data Pointer to the values in row-major order.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status writeImage(const SizeArray& shape,
                    const IO::BaseDataType& dataType,
                    const void* data);

  DEFINE_ATTRIBUTE_FIELD(readResolution,
                         float,
                         "resolution",
                         Pixel resolution of the image in pixels per cm)

protected:
  /**
   * @brief Check whether the shape is valid for the type of the image.
   * @param shape The shape of the image.
   * @return True for [x, y], [x, y, 3], and [x, y, 4].
   */
  virtual bool isValidShape(const SizeArray& shape) const;

  /**
   * @brief Constructor.
   * @param path The location of the image in the file.
   * @param io A shared pointer to the IO object.
   */
  Image(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
