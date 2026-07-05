#pragma once

#include <any>
#include <set>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "io/RecordingObjects.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{
class MeaningsTable;
}  // namespace AQNWB::NWB

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
  REGISTER_SUBCLASS(DynamicTable,
                    Container,
                    AQNWB::SPEC::HDMF_COMMON::namespaceName)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the table in the file.
   * @param io A shared pointer to the IO object.
   */
  DynamicTable(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Destructor
   */
  ~DynamicTable() override;

  /**
   * @brief Initializes the `DynamicTable` object by creating NWB attributes and
   * column names.
   *
   * @param description The description of the table (optional).
   * @param rowChunkSize The chunk size for the rows in the table (optional,
   * default: 100).
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::string& description,
                    const SizeType rowChunkSize = 100);

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
   *
   * If the VectorData has already been added to the table, it will not be
   * added again, but the values will still be appended to the existing dataset.
   *
   * @param vectorData A unique pointer to the `VectorData` dataset.
   * @param values The vector of string values.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status addColumn(const std::shared_ptr<VectorData>& vectorData,
                   const std::vector<std::string>& values);

  /**
   * @brief Adds a column to the table without writing data.
   *
   * This is useful for adding fully initialized columns where the
   * data is written separately. This includes columns that may be
   * subclasses of VectorData that have their own initialization
   * and data writing methods (e.g,. TimestampVectorData or
   * DurationVectorData, etc.).
   *
   * @param vectorData A shared pointer to the `VectorData` dataset.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status addColumn(const std::shared_ptr<VectorData>& vectorData);

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
   * @brief Sets the values of the element identifiers on the table.
   * @param values The vector of id values.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status setRowIDs(const std::vector<int>& values);

  /**
   * @brief Sets the column names of the DynamicTable
   *
   * All changes to the column names will be flushed to the file.
   * If the newColNames is identical to the existing column names,
   * then no changes will be made.
   *
   * @raises std::invalid_argument if the newColNames vector does not
   * contain all columns of the table. I.e., the newColNames vector
   * must be a permutation of the existing column names.
   * @param newColNames The vector of new column names.
   */
  virtual void setColNames(const std::vector<std::string>& newColNames);

  /**
   * @brief Create a MeaningsTable for a specific VectorData column in this
   * DynamicTable.
   *
   * The function creates and fully initializes a MeaningsTable for the
   * specified VectorData column in this DynamicTable. The MeaningsTable will
   * be created at the path "meanings_tables/<columnName>_meanings" within this
   * DynamicTable.
   *
   * @param columnName The name of the VectorData column for which to create the
   * MeaningsTable.
   * @param rowChunkSize The chunk size for the rows in the MeaningsTable
   * (optional, default: 100).
   * @return A shared pointer to the created MeaningsTable, or nullptr if
   * creation failed.
   */
  std::shared_ptr<MeaningsTable> createMeaningsTable(
      const std::string& columnName, const SizeType rowChunkSize = 100);

  /**
   * @brief Type trait to determine the return type of readColumn.
   *
   * If T is a subclass of VectorData, the return type is T.
   * Otherwise, the return type is VectorDataTyped<T>.
   */
  template<typename T>
  struct ColumnReturnType
  {
    using type =
        typename std::conditional<std::is_base_of<VectorData, T>::value,
                                  T,
                                  VectorDataTyped<T>>::type;
  };

  /**
   * @brief Read an arbitrary column of the DynamicTable
   *
   * For columns defined in the schema the corresponding DEFINE_REGISTERED_FIELD
   * read functions are preferred because they help avoid the need for
   * specifying the specific name of the column and data type to use.
   *
   * If the template parameter T is a subclass of VectorData (e.g.,
   * TimestampVectorData), the function returns a shared pointer to T.
   * Otherwise, it returns a shared pointer to VectorDataTyped<T>.
   *
   * @tparam T The data type of the column or the specific VectorData subclass.
   * @param colName The name of the column to read.
   * @return The VectorData object representing the column or a nullptr if the
   * column doesn't exist.
   */
  template<typename T = std::any>
  std::shared_ptr<typename ColumnReturnType<T>::type> readColumn(
      const std::string& colName)
  {
    using ReturnType = typename ColumnReturnType<T>::type;
    std::string columnPath = AQNWB::mergePaths(m_path, colName);
    auto ioPtr = getIO();
    if (ioPtr != nullptr) {
      if (ioPtr->objectExists(columnPath)) {
        if (ioPtr->getStorageObjectType(columnPath)
            == StorageObjectType::Dataset)
        {
          return ReturnType::create(columnPath, ioPtr);
        }
      }
    } else {
      std::cerr << "IO object has been deleted. Can't read column: " << colName
                << " in DynamicTable: " << m_path << std::endl;
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

  /**
   * @brief Returns the instance of the class representing the named
   * MeaningsTable field.
   *
   * This overload is declared here and defined in DynamicTable.cpp so callers
   * that include only DynamicTable.hpp do not need MeaningsTable.hpp to use the
   * default MeaningsTable accessor.
   *
   * @param objectName The name of the object to retrieve.
   * @return A shared pointer to a MeaningsTable representing the object. May
   * return nullptr if the path does not exist.
   */
  std::shared_ptr<MeaningsTable> readMeaningsTable(
      const std::string& objectName) const;

  /**
   * @brief Returns the instance of the class representing the named
   * MeaningsTable field for write.
   *
   * This overload is declared here and defined in DynamicTable.cpp so callers
   * that include only DynamicTable.hpp do not need MeaningsTable.hpp to use the
   * default MeaningsTable accessor.
   *
   * **NOTE:** Use createMeaningsTable() to create and initialize a new
   * MeaningsTable for a specific column.
   *
   * @param objectName The name of the object to retrieve.
   * @return A shared pointer to a MeaningsTable representing the object.
   */
  std::shared_ptr<MeaningsTable> createMeaningsTableInstance(
      const std::string& objectName) const;

protected:
  /**
   *  @brief Add a column name to m_colNames while preventing duplicates
   *
   *  If the column name already exists in m_colNames, it will not be added
   * again. The function returns the index of the column name in m_colNames,
   * whether it was newly added or already existed. If the column name is new,
   * it will be appended to the end of m_colNames. Any changes to m_colNames
   * will be flushed to the file.
   *  @param colName The name of the column to add
   *  @return Index of the column name in m_colNames
   */
  SizeType addColumnName(const std::string& colName);

  /**
   * @brief Flush the column names to the file.
   *
   * This function writes the current column names in m_colNames to the file as
   * an attribute.
   *
   * @return Status::Success if successful, otherwise Status::Failure.
   */

  Status flushColNames();

  /**
   * @brief Names of the columns in the table.
   */
  std::vector<std::string> m_colNames;

  /**
   * @brief The columns added for recording
   */
  std::unique_ptr<IO::RecordingObjects> m_recordingColumns;

  /**
   * @brief The row ids data object for write
   */
  std::shared_ptr<ElementIdentifiers> m_rowElementIdentifiers;
};
}  // namespace AQNWB::NWB
