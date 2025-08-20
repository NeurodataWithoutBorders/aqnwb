#pragma once

#include <any>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Represents a group containing multiple datasets that are aligned on
 * the first dimension
 *
 * This class inherits from the `Container` class and provides methods to add
 * columns of different types of data to the table.
 */
class DynamicTable : public Container
{
public:
  // Register the TimeSeries as a subclass of Container
  REGISTER_SUBCLASS(DynamicTable, AQNWB::SPEC::HDMF_COMMON::namespaceName)

  // Bring base class initialize method into scope
  using Container::initialize;

  /**
   * @brief Constructor.
   * @param path The location of the table in the file.
   * @param io A shared pointer to the IO object.
   */
  DynamicTable(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor
   */
  virtual ~DynamicTable();

  /**
   * @brief Initializes the `DynamicTable` object by creating NWB attributes and
   * column names.
   *
   * @param description The description of the table (optional).
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::string& description);

  /**
   * @brief Finalizes writing the DynamicTable.
   *
   * Finalizes the DynamicTable by writing the column names
   * as a single write once the table has been set up
   *
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status finalize() override;

  /**
   * @brief Adds a column of vector string data to the table.
   * @param vectorData A unique pointer to the `VectorData` dataset.
   * @param values The vector of string values.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status addColumn(std::unique_ptr<VectorData>& vectorData,
                   const std::vector<std::string>& values);

  /**
   * @brief Adds a column of references to the table.
   * @param name The name of the column.
   * @param colDescription The description of the column.
   * @param dataset The vector of string values representing the references.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status addReferenceColumn(const std::string& name,
                            const std::string& colDescription,
                            const std::vector<std::string>& dataset);

  /**
   * @brief Adds a column of element identifiers to the table.
   * @param elementIDs A unique pointer to the `ElementIdentifiers` dataset.
   * @param values The vector of id values.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status setRowIDs(std::unique_ptr<ElementIdentifiers>& elementIDs,
                   const std::vector<int>& values);

  /**
   * @brief Sets the column names of the DynamicTable
   *
   * ..note::
   * For this change to take affect in the file we need to call
   * finalize() after setting the column names to write the data to the file.
   *
   * .. warning::
   * This will overwrite any existing column names. It is up to
   * the caller to ensure that all existing columns are included in the new
   * list.
   *
   * @param newColNames The vector of new column names.
   */
  virtual void setColNames(const std::vector<std::string>& newColNames)
  {
    m_colNames = newColNames;
  }

  /**
   * @brief Read an arbitrary column of the DyanmicTable
   *
   * For columns defined in the schema the corresponding DEFINE_REGISTERED_FIELD
   * read functions are preferred because they help avoid the need for
   * specifying the specific name of the column and data type to use.
   *
   * @return The VectorData object representing the column or a nullptr if the
   * column doesn't exists
   */
  template<typename DTYPE = std::any>
  std::shared_ptr<VectorDataTyped<DTYPE>> readColumn(const std::string& colName)
  {
    std::string columnPath = AQNWB::mergePaths(m_path, colName);
    if (m_io->objectExists(columnPath)) {
      if (m_io->getStorageObjectType(columnPath) == StorageObjectType::Dataset)
      {
        return std::make_shared<VectorDataTyped<DTYPE>>(columnPath, m_io);
      }
    }
    return nullptr;
  }

  DEFINE_ATTRIBUTE_FIELD(readColNames,
                         std::string,
                         "colnames",
                         The names of the columns in the table)

  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         Description of what is in this dynamic table)

  DEFINE_REGISTERED_FIELD(
      readIdColumn,
      ElementIdentifiers,
      "id",
      "unique identifiers for the rows of this dynamic table")

protected:
  /**
   * @brief Names of the columns in the table.
   */
  std::vector<std::string> m_colNames;
};
}  // namespace AQNWB::NWB
