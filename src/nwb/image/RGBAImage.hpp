#pragma once

#include <memory>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/base/BaseImage.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief A color image with transparency with shape [x, y, 4] (r, g, b, a).
 */
class RGBAImage : public Image
{
public:
  // Register the RGBAImage as a subclass of Image
  REGISTER_SUBCLASS(RGBAImage, Image, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Destructor
   */
  ~RGBAImage() override;

protected:
  /**
   * @brief Check whether the shape is valid for a RGBAImage.
   * @param shape The shape of the image.
   * @return True if the shape is valid.
   */
  bool isValidShape(const SizeArray& shape) const override;

  /**
   * @brief Constructor.
   * @param path The location of the image in the file.
   * @param io A shared pointer to the IO object.
   */
  RGBAImage(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
