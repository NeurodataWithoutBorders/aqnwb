#pragma once

#include <memory>
#include <optional>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
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
   * @brief Destructor
   */
  ~BaseImage() override;

  /**
   * @brief Initialize the image dataset.
   * @param dataConfig The configuration of the image dataset.
   * @param description Description of the image. Optional.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                    const std::optional<std::string>& description);

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
   * @brief Destructor
   */
  ~Image() override;

  /**
   * @brief Initialize the image dataset.
   *
   * Checks that the shape is valid for the type of the image (see
   * isValidShape) and that the data is numeric.
   *
   * @param dataConfig The configuration of the image dataset.
   * @param description Description of the image. Optional.
   * @param resolution Pixel resolution of the image, in pixels per
   * centimeter. Optional.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                    const std::optional<std::string>& description,
                    const std::optional<float>& resolution = std::nullopt);

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
