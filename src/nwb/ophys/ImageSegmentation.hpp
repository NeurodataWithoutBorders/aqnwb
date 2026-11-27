#pragma once

#include <memory>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBDataInterface.hpp"
#include "nwb/ophys/PlaneSegmentation.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Stores pixels in an image that represent different regions of interest
 * (ROIs) or masks.
 *
 * A container for PlaneSegmentation objects, which are created via
 * createPlaneSegmentation. By convention, the object is named
 * "ImageSegmentation" and stored in the "ophys" ProcessingModule.
 */
class ImageSegmentation : public NWBDataInterface
{
public:
  // Register the ImageSegmentation as a subclass of NWBDataInterface
  REGISTER_SUBCLASS(ImageSegmentation,
                    NWBDataInterface,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a ImageSegmentation.
   *
   * ImageSegmentation does not define any additional fields.
   */
  struct Spec : public NWBDataInterface::Spec
  {
  };

  /**
   * @brief Destructor
   */
  ~ImageSegmentation() override;

  /**
   * @brief Initializes the ImageSegmentation from a spec.
   * @param spec The specification of the ImageSegmentation.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_UNNAMED_REGISTERED_FIELD(readPlaneSegmentation,
                                  createPlaneSegmentation,
                                  PlaneSegmentation,
                                  "",
                                  Results from image segmentation of a specific
                                      imaging plane)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the ImageSegmentation in the file.
   * @param io A shared pointer to the IO object.
   */
  ImageSegmentation(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
