#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
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
   *
   * The dataset can be extended via append.
   *
   * @param imagePaths The paths of the referenced images, in order.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::vector<std::string>& imagePaths);

  /**
   * @brief Append references to images to the end of the dataset.
   *
   * The existing references keep their position, i.e., indices into the
   * order (e.g., of an IndexSeries) stay valid. Fails for datasets with a
   * fixed size (e.g., in files written by older versions).
   *
   * @param imagePaths The paths of the referenced images. The images must
   * exist.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status append(const std::vector<std::string>& imagePaths);

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
 * The schema requires at least one image, i.e., the images are part of the
 * spec, e.g.:
 *
 * @code
 * auto spec = Images::Spec()
 *                 .withDescription("Stimulus images")
 *                 .withImage<GrayscaleImage>(
 *                     "frame0",
 *                     Image::Spec().withData(IO::ArrayDataSetConfig(
 *                         IO::BaseDataType::U8, {4, 3}, {4, 3})),
 *                     pixels.data())
 *                 .withOrderOfImages({"frame0"});
 * @endcode
 *
 * Further images can be added via addImage.
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
    /// @brief Description of this collection of images. Required.
    DEFINE_SPEC_FIELD(Spec, std::string, description, withDescription, "")

    /**
     * @brief An image of the collection, see withImage.
     */
    struct ImageEntry
    {
      /// @brief The name of the image.
      std::string name;
      /// @brief The specification of the image.
      Image::Spec spec;
      /// @brief Pointer to the pixel values in row-major order with the data
      /// type of spec.data. Must remain valid until initialize is called.
      const void* data = nullptr;
      /// @brief Creates the image object of the requested type.
      std::function<std::shared_ptr<Image>(const std::string&,
                                           std::shared_ptr<IO::BaseIO>)>
          create;
    };

    /// @brief The images of the collection. At least one image is required.
    DEFINE_SPEC_FIELD(Spec,
                      std::vector<ImageEntry>,
                      images,
                      withImages,
                      std::vector<ImageEntry> {})

    /**
     * @brief Add an image of type T (e.g., GrayscaleImage) to the spec.
     * @tparam T The type of the image, i.e., Image or a subtype.
     * @param name The name of the image.
     * @param spec The specification of the image.
     * @param data Pointer to the pixel values in row-major order with the
     * data type of spec.data. Must remain valid until initialize is called.
     * @return Reference to this spec to allow chaining.
     */
    template<typename T>
    Spec& withImage(const std::string& name,
                    const Image::Spec& spec,
                    const void* data)
    {
      static_assert(std::is_base_of_v<Image, T>,
                    "withImage requires Image or a subtype of Image");
      images.push_back(
          {name,
           spec,
           data,
           [](const std::string& path,
              std::shared_ptr<IO::BaseIO> io) -> std::shared_ptr<Image>
           { return T::create(path, io); }});
      return *this;
    }

    /**
     * @brief The names of the images in the order in which they are
     * referenced, e.g., by an IndexSeries. Optional.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<std::vector<std::string>>,
                      orderOfImages,
                      withOrderOfImages,
                      std::nullopt)

    /**
     * @brief Validate the spec.
     *
     * Checks that the description is not empty, that there is at least one
     * image, that the names of the images are unique and not empty, that the
     * specs and data of the images are valid, and that orderOfImages (if set)
     * only refers to images of the spec.
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
   *
   * Checks the spec and the images (e.g., the shapes of the image types)
   * before anything is written, then creates the collection, the images, and
   * the order of the images.
   *
   * @param spec The specification of the collection.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Add an image to an initialized collection.
   *
   * If the collection has an order of the images (order_of_images), the
   * image is appended to the order, since the order must list all images of
   * the collection. The positions of the existing images do not change.
   *
   * @tparam T The type of the image, i.e., Image or a subtype.
   * @param name The name of the image.
   * @param spec The specification of the image.
   * @param data Pointer to the pixel values in row-major order with the data
   * type of spec.data.
   * @return The image or nullptr if the image could not be created.
   */
  template<typename T>
  std::shared_ptr<T> addImage(const std::string& name,
                              const Image::Spec& spec,
                              const void* data)
  {
    static_assert(std::is_base_of_v<Image, T>,
                  "addImage requires Image or a subtype of Image");
    auto image = createImage<T>(name);
    if (image == nullptr || writeImage(*image, spec, data) != Status::Success
        || appendToOrderOfImages(image->getPath()) != Status::Success)
    {
      return nullptr;
    }
    return image;
  }

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
   * @brief Append an image to the order of the images, if the collection has
   * an order.
   * @param imagePath The path of the image.
   * @return Status::Success if the image was appended or the collection has
   * no order, otherwise Status::Failure.
   */
  Status appendToOrderOfImages(const std::string& imagePath);

  /**
   * @brief Initialize an image and write its pixel values.
   * @param image The image to initialize.
   * @param spec The specification of the image.
   * @param data Pointer to the pixel values.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status writeImage(Image& image, const Image::Spec& spec, const void* data);

  /**
   * @brief Constructor.
   * @param path The location of the collection in the file.
   * @param io A shared pointer to the IO object.
   */
  Images(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
