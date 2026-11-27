#pragma once

#include <memory>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/base/BaseImage.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief A color image with shape [x, y, 3] (r, g, b).
 */
class RGBImage : public Image
{
public:
  // Register the RGBImage as a subclass of Image
  REGISTER_SUBCLASS(RGBImage, Image, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Destructor
   */
  ~RGBImage() override;

protected:
  /**
   * @brief Check whether the shape is valid for a RGBImage.
   * @param shape The shape of the image.
   * @return True if the shape is valid.
   */
  bool isValidShape(const SizeArray& shape) const override;

  /**
   * @brief Constructor.
   * @param path The location of the image in the file.
   * @param io A shared pointer to the IO object.
   */
  RGBImage(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
