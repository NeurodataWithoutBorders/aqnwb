#pragma once

#include <memory>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/base/BaseImage.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief A grayscale image with shape [x, y].
 */
class GrayscaleImage : public Image
{
public:
  // Register the GrayscaleImage as a subclass of Image
  REGISTER_SUBCLASS(GrayscaleImage, Image, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Destructor
   */
  ~GrayscaleImage() override;

protected:
  /**
   * @brief Check whether the shape is valid for a GrayscaleImage.
   * @param shape The shape of the image.
   * @return True if the shape is valid.
   */
  bool isValidShape(const SizeArray& shape) const override;

  /**
   * @brief Constructor.
   * @param path The location of the image in the file.
   * @param io A shared pointer to the IO object.
   */
  GrayscaleImage(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
