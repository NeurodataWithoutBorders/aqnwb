#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "nwb/hdmf/table/VectorIndex.hpp"
#include "nwb/ophys/ImagingPlane.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Results from image segmentation of a specific imaging plane.
 *
 * Each row of the table is a region of interest (ROI). The ROIs are described
 * by one of the following columns:
 * - `image_mask`: a weight for each pixel of the imaging plane (shape
 *   [num_rois, x, y] or [num_rois, x, y, z]), see addImageMasks
 * - `pixel_mask` (with `pixel_mask_index`): a list of (x, y, weight) entries
 *   per ROI, see addPixelMasks
 * - `voxel_mask` (with `voxel_mask_index`): a list of (x, y, z, weight)
 *   entries per ROI, see addVoxelMasks
 */
class PlaneSegmentation : public DynamicTable
{
public:
  // Register the PlaneSegmentation as a subclass of DynamicTable
  REGISTER_SUBCLASS(PlaneSegmentation,
                    DynamicTable,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief An entry of a pixel mask, i.e., the pixel coordinates and weight.
   */
  struct PixelMaskEntry
  {
    uint32_t x;  ///< Pixel x-coordinate.
    uint32_t y;  ///< Pixel y-coordinate.
    float weight;  ///< Weight of the pixel.
  };

  /**
   * @brief An entry of a voxel mask, i.e., the voxel coordinates and weight.
   */
  struct VoxelMaskEntry
  {
    uint32_t x;  ///< Voxel x-coordinate.
    uint32_t y;  ///< Voxel y-coordinate.
    uint32_t z;  ///< Voxel z-coordinate.
    float weight;  ///< Weight of the voxel.
  };

  /**
   * @brief Get the compound data type of the pixel_mask column.
   * @return The data type matching PixelMaskEntry.
   */
  static IO::BaseDataType getPixelMaskType();

  /**
   * @brief Get the compound data type of the voxel_mask column.
   * @return The data type matching VoxelMaskEntry.
   */
  static IO::BaseDataType getVoxelMaskType();

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
    /**
     * @brief If true, add the pixel_mask and pixel_mask_index columns (if not
     * already part of the columns).
     */
    DEFINE_SPEC_FIELD(Spec, bool, pixelMask, withPixelMask, false)

    /**
     * @brief If true, add the voxel_mask and voxel_mask_index columns (if not
     * already part of the columns).
     */
    DEFINE_SPEC_FIELD(Spec, bool, voxelMask, withVoxelMask, false)

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
     * Checks that the description is not empty, that the imagingPlane is set,
     * and that at least one mask column (image_mask, pixel_mask, or
     * voxel_mask) is configured. The columns are validated by
     * validateDataSpecs.
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
   * @brief Create the specs of the pixel_mask and pixel_mask_index columns.
   * @param tablePath The path of the PlaneSegmentation.
   * @param chunkSize Chunk size of the pixel_mask column (in entries).
   * @return The DataSpecs of the pixel_mask and pixel_mask_index columns.
   */
  static std::vector<DataSpecPtr> createPixelMaskDataSpecs(
      const std::string& tablePath, const SizeType chunkSize = 1024);

  /**
   * @brief Create the specs of the voxel_mask and voxel_mask_index columns.
   * @param tablePath The path of the PlaneSegmentation.
   * @param chunkSize Chunk size of the voxel_mask column (in entries).
   * @return The DataSpecs of the voxel_mask and voxel_mask_index columns.
   */
  static std::vector<DataSpecPtr> createVoxelMaskDataSpecs(
      const std::string& tablePath, const SizeType chunkSize = 1024);

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

  /**
   * @brief Append ROIs defined by pixel masks.
   *
   * Appends one row per mask, i.e., the entries of the masks and the ids of
   * the ROIs. The table must have the pixel_mask column (see
   * Spec::withPixelMask).
   *
   * @param masks The pixel masks, one list of entries per ROI.
   * @param ids The ids of the ROIs. If empty, the ids are the row indices.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status addPixelMasks(const std::vector<std::vector<PixelMaskEntry>>& masks,
                       const std::vector<int>& ids = {});

  /**
   * @brief Append ROIs defined by voxel masks.
   *
   * Appends one row per mask, i.e., the entries of the masks and the ids of
   * the ROIs. The table must have the voxel_mask column (see
   * Spec::withVoxelMask).
   *
   * @param masks The voxel masks, one list of entries per ROI.
   * @param ids The ids of the ROIs. If empty, the ids are the row indices.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status addVoxelMasks(const std::vector<std::vector<VoxelMaskEntry>>& masks,
                       const std::vector<int>& ids = {});

  /**
   * @brief Read the pixel masks of all ROIs.
   * @return One list of (x, y, weight) entries per ROI. Empty if the table
   * has no pixel_mask column.
   * @throws std::invalid_argument if the pixel_mask column has an unexpected
   * data type.
   */
  std::vector<std::vector<PixelMaskEntry>> readPixelMasks() const;

  /**
   * @brief Read the voxel masks of all ROIs.
   * @return One list of (x, y, z, weight) entries per ROI. Empty if the table
   * has no voxel_mask column.
   * @throws std::invalid_argument if the voxel_mask column has an unexpected
   * data type.
   */
  std::vector<std::vector<VoxelMaskEntry>> readVoxelMasks() const;

  DEFINE_REGISTERED_FIELD(readPixelMask,
                          VectorData,
                          "pixel_mask",
                          Pixel masks for each ROI)

  DEFINE_REGISTERED_FIELD(readPixelMaskIndex,
                          VectorIndex,
                          "pixel_mask_index",
                          Index into pixel_mask)

  DEFINE_REGISTERED_FIELD(readVoxelMask,
                          VectorData,
                          "voxel_mask",
                          Voxel masks for each ROI)

  DEFINE_REGISTERED_FIELD(readVoxelMaskIndex,
                          VectorIndex,
                          "voxel_mask_index",
                          Index into voxel_mask)

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
   * @brief Append ragged masks to a mask column and its index column.
   * @param maskName The name of the mask column.
   * @param type The compound data type of the mask entries.
   * @param entries Pointer to all entries of all masks (concatenated).
   * @param maskSizes The number of entries of each mask.
   * @param ids The ids of the ROIs. If empty, the ids are the row indices.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  /**
   * @brief Check that the table has no mask column other than maskName.
   *
   * The add*Masks methods append rows to a single mask column (and the id
   * column), i.e., tables with several mask columns are not supported.
   *
   * @param maskName The name of the mask column to add rows to.
   * @return Status::Success if no other mask column exists.
   */
  Status checkSingleMaskType(const std::string& maskName) const;

  Status addRaggedMasks(const std::string& maskName,
                        const IO::BaseDataType& type,
                        const void* entries,
                        const std::vector<SizeType>& maskSizes,
                        const std::vector<int>& ids);

  /**
   * @brief Constructor.
   * @param path The location of the PlaneSegmentation in the file.
   * @param io A shared pointer to the IO object.
   */
  PlaneSegmentation(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
