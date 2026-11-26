#pragma once

#include <memory>
#include <string>

#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/hdmf/base/Data.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{
/**
 * @brief DynamicTableRegion provides a link from one table to an index or
 * region of another.
 *
 * The data are the (zero-based) indices of the referenced rows of the target
 * table, which is referenced by the `table` attribute. DynamicTableRegion can
 * be used as a column of a DynamicTable (via DataSpec) or as a dataset of
 * another type, e.g., the `electrodes` of an ElectricalSeries or the `rois` of
 * a RoiResponseSeries.
 */
class DynamicTableRegion : public VectorData
{
public:
  /**
   * @brief Runtime configuration for creating and initializing a
   * DynamicTableRegion column.
   */
  struct DataSpec : public Data::DataSpec<DynamicTableRegion>
  {
    /**
     * @brief Construct the DataSpec.
     * @param datasetName The name of the column.
     * @param dataConfig The configuration of the dataset. Must use an
     * integer data type and a 1D shape.
     * @param columnDescription The description of the column.
     * @param columnTablePath The path of the referenced DynamicTable.
     */
    DataSpec(const std::string& datasetName,
             const IO::ArrayDataSetConfig& dataConfig,
             const std::string& columnDescription,
             const std::string& columnTablePath)
        : Data::DataSpec<DynamicTableRegion>(datasetName, dataConfig)
        , description(columnDescription)
        , tablePath(columnTablePath)
    {
    }

    virtual ~DataSpec() override = default;

    /// @brief Description of what this table region points to.
    std::string description;

    /// @brief The path of the referenced DynamicTable.
    std::string tablePath;

    /**
     * @brief Initialize the DynamicTableRegion from this spec.
     * @param data The DynamicTableRegion to initialize.
     * @return Status::Success if successful, otherwise Status::Failure.
     */
    Status initialize(Data& data) const override;
  };

  /**
   * @brief Create the DataSpec for a DynamicTableRegion column.
   * @param name The name of the column.
   * @param dataConfig The configuration of the dataset.
   * @param description The description of the column.
   * @param tablePath The path of the referenced DynamicTable.
   * @return The DataSpec.
   */
  static std::shared_ptr<DataSpec> createDataSpec(
      const std::string& name,
      const IO::ArrayDataSetConfig& dataConfig,
      const std::string& description,
      const std::string& tablePath);

  // Register the DynamicTableRegion as a subclass of VectorData
  REGISTER_SUBCLASS(DynamicTableRegion,
                    VectorData,
                    AQNWB::SPEC::HDMF_COMMON::namespaceName)

  /**
   * @brief Destructor
   */
  ~DynamicTableRegion() override;

  /**
   * @brief Initialize the DynamicTableRegion.
   *
   * Creates the dataset with the description attribute and the reference to
   * the target table.
   *
   * @param dataConfig The configuration of the dataset. Must use an integer
   * data type and a 1D shape.
   * @param description Description of what this table region points to.
   * @param tablePath The path of the referenced DynamicTable. The table must
   * exist.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                    const std::string& description,
                    const std::string& tablePath);

  DEFINE_REFERENCED_REGISTERED_FIELD(
      readTable,
      DynamicTable,
      "table",
      Reference to the DynamicTable object that this region applies to.)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the DynamicTableRegion in the file.
   * @param io A shared pointer to the IO object.
   */
  DynamicTableRegion(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
