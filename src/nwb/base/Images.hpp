#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/BaseImage.hpp"
#include "nwb/base/NWBData.hpp"
#include "nwb/base/NWBDataInterface.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Ordered dataset of references to BaseImage objects.
 */
class ImageReferences : public NWBData
{
public:
  // Register the ImageReferences as a subclass of NWBData
  REGISTER_SUBCLASS(ImageReferences, NWBData, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Destructor
   */
  ~ImageReferences() override;

  /**
   * @brief Initialize the dataset with references to the given images.
   * @param imagePaths The paths of the referenced images, in order.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::vector<std::string>& imagePaths);

protected:
  /**
   * @brief Constructor.
   * @param path The location of the dataset in the file.
   * @param io A shared pointer to the IO object.
   */
  ImageReferences(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};

/**
 * @brief A collection of images with an optional way to specify the order of
 * the images.
 *
 * Images are created via createImage, e.g.,
 * `images->createImage<GrayscaleImage>("frame0")`, and then initialized and
 * written via Image::initialize and Image::writeImage.
 */
class Images : public NWBDataInterface
{
public:
  // Register the Images as a subclass of NWBDataInterface
  REGISTER_SUBCLASS(Images, NWBDataInterface, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an Images collection.
   */
  struct Spec : public NWBDataInterface::Spec
  {
    REGISTER_SPEC(Images, NWBDataInterface::Spec)

    /// @brief Description of this collection of images. Required.
    DEFINE_SPEC_FIELD(Spec, std::string, description, withDescription, "")

    /**
     * @brief Validate the spec.
     *
     * Checks that the description is not empty.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~Images() override;

  /**
   * @brief Initialize the collection from a spec.
   * @param spec The specification of the collection.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Specify the order of the images via the order_of_images dataset.
   * @param imageNames The names of the images of this collection, in order.
   * The images must exist.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status setOrderOfImages(const std::vector<std::string>& imageNames);

  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         Description of this collection of images)

  DEFINE_UNNAMED_REGISTERED_FIELD(
      readImage, createImage, BaseImage, "", Images stored in this collection)

  DEFINE_REGISTERED_FIELD(readOrderOfImages,
                          ImageReferences,
                          "order_of_images",
                          Ordered references to the images)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the collection in the file.
   * @param io A shared pointer to the IO object.
   */
  Images(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
