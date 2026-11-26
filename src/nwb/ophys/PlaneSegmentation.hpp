#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "nwb/ophys/ImagingPlane.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Results from image segmentation of a specific imaging plane.
 *
 * Each row of the table is a region of interest (ROI). The ROIs are described
 * by the `image_mask` column, i.e., a weight for each pixel of the imaging
 * plane (shape [num_rois, x, y] or [num_rois, x, y, z]).
 *
 * @note The `pixel_mask` and `voxel_mask` columns, which use compound data
 * types, are not supported yet (see issue #381).
 */
class PlaneSegmentation : public DynamicTable
{
public:
  // Register the PlaneSegmentation as a subclass of DynamicTable
  REGISTER_SUBCLASS(PlaneSegmentation,
                    DynamicTable,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a PlaneSegmentation.
   *
   * Example:
   * @code
   * auto columns = PlaneSegmentation::createDefaultDataSpecs();
   * columns.push_back(PlaneSegmentation::createImageMaskDataSpec({512, 512}));
   * auto spec = PlaneSegmentation::Spec()
   *                 .withDescription("ROIs detected online")
   *                 .withColumns(columns)
   *                 .withImagingPlane(imagingPlane);
   * @endcode
   */
  struct Spec : public DynamicTable::Spec
  {
    REGISTER_SPEC(PlaneSegmentation, DynamicTable::Spec)

    /// @brief Link to the ImagingPlane of the ROIs. Required.
    DEFINE_SPEC_FIELD(Spec,
                      std::shared_ptr<ImagingPlane>,
                      imagingPlane,
                      withImagingPlane,
                      nullptr)

    INHERIT_SPEC_SETTER(Spec, DynamicTable::Spec, withDescription)
    INHERIT_SPEC_SETTER(Spec, DynamicTable::Spec, withColumns)

    /**
     * @brief Validate the spec.
     *
     * Checks that the description is not empty and that the imagingPlane is
     * set. The columns are validated by validateDataSpecs.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~PlaneSegmentation() override;

  /**
   * @brief Initializes the PlaneSegmentation from a spec.
   *
   * Uses createDefaultDataSpecs() if no columns are specified and creates the
   * (empty) reference_images group and the link to the imaging plane.
   *
   * @param spec The specification of the PlaneSegmentation.
   * @return Status::Success if successful, otherwise Status::Failure.
   * @throw std::invalid_argument if the column specs are invalid.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Create the default column specs of the table, i.e., only the id
   * column. Add the image_mask column via createImageMaskDataSpec.
   * @param rowChunkSize Chunk size for the id column.
   * @return The default column specs.
   */
  static std::vector<DataSpecPtr> createDefaultDataSpecs(
      const SizeType rowChunkSize = 100);

  /**
   * @brief Create the spec of the image_mask column.
   *
   * The column stores one float32 mask per ROI with one weight per pixel
   * (shape [num_rois, x, y] or [num_rois, x, y, z]) and one mask per chunk.
   *
   * @param maskShape The shape of a mask, i.e., [x, y] or [x, y, z].
   * @return The DataSpec of the column.
   */
  static DataSpecPtr createImageMaskDataSpec(const SizeArray& maskShape);

  /**
   * @brief Validate the column specs of the table.
   *
   * In addition to the checks of DynamicTable::validateDataSpecs, checks that
   * the image_mask column (if present) is a 3D or 4D float VectorData.
   *
   * @param dataSpecs The column specs to validate.
   * @return Status::Success if the specs are valid, otherwise Status::Failure.
   */
  Status validateDataSpecs(
      const std::vector<DataSpecPtr>& dataSpecs) const override;

  /**
   * @brief Append ROIs defined by image masks.
   *
   * Appends numRois rows to the table, i.e., the image masks and the ids of
   * the ROIs. The table must have been initialized with an image_mask column.
   *
   * @param numRois The number of ROIs to append.
   * @param masks Pointer to the masks, i.e., a contiguous float32 array of
   * shape [numRois, x, y(, z)] in row-major order.
   * @param ids The ids of the ROIs. If empty, the ids are the row indices.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status addImageMasks(const SizeType& numRois,
                       const float* masks,
                       const std::vector<int>& ids = {});

  DEFINE_REGISTERED_FIELD(readImageMask,
                          VectorData,
                          "image_mask",
                          ROI masks for each ROI)

  DEFINE_REGISTERED_FIELD(readImagingPlane,
                          ImagingPlane,
                          "imaging_plane",
                          The imaging plane of the ROIs)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the PlaneSegmentation in the file.
   * @param io A shared pointer to the IO object.
   */
  PlaneSegmentation(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
