#pragma once

#include <any>
#include <optional>
#include <string>
#include <unordered_map>

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
  using DataSpec = Data::DataSpecBase;
  using DataSpecPtr = std::shared_ptr<DataSpec>;

public:
  using CellValue = AQNWB::NWB::CellValue;
  using RowData = std::unordered_map<std::string, CellValue>;

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
   * optional configured data columns.
   *
   * @param description The description of the table (optional).
   * @param dataSpecs Optional ordered configuration for the data objects to
   *        create. If empty, only the default id column is created.
   * @return Status::Success if successful, otherwise Status::Failure.
   * @throw std::invalid_argument if the provided dataSpecs are invalid.
   *
   */
  Status initialize(const std::string& description,
                    const std::vector<DataSpecPtr>& dataSpecs = {});

  /**
   * @brief Validates the provided data specifications for the table.
   *
   * This method checks whether the provided data specifications are valid
   * for the current table type. It can be called during initialization or
   * separately by the user to validate a proposed schema.
   *
   * @param dataSpecs The data specifications to validate.
   * @return Status::Success if the specifications are valid, otherwise
   * Status::Failure.
   */
  virtual Status validateDataSpecs(
      const std::vector<DataSpecPtr>& dataSpecs) const;

  /**
   * @brief Create the default data specs for a DynamicTable.
   *
   * The default DynamicTable schema consists only of the built-in
   * ElementIdentifiers `id` dataset. Callers may use this as a starting point,
   * modify the returned vector, and pass the result to initialize().
   *
   * @param rowChunkSize Chunk size to use for the default id dataset.
   * @return Ordered default data specs for the table.
   */
  static std::vector<DataSpecPtr> createDefaultDataSpecs(
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
   * @brief Adds a column to the table from a DataSpecPtr.
   *
   * This is the preferred overload for adding columns after table creation
   * (i.e. after initialize() has been called but before startRecording()).
   * It is consistent with the DataSpec-based approach used by initialize() /
   * configureDataObjects(): the spec bundles the column name, dataset
   * configuration, and description, and the table creates and initializes the
   * VectorData internally.  Because DataSpec::initialize() is virtual, this
   * overload works for any VectorData subtype whose DataSpec is registered.
   *
   * @param dataSpec The DataSpec describing the column to add.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status addColumn(const DataSpecPtr& dataSpec);

  /**
   * @brief Adds an already-initialized column to the table without writing
   * data.
   *
   * Kept for backward compatibility. Prefer addColumn(DataSpecPtr) for new
   * code, which is consistent with the DataSpec-based API used by initialize().
   *
   * @param vectorData A shared pointer to the already-initialized `VectorData`.
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

  Status addRow(const RowData& row,
                const std::optional<int>& rowId = std::nullopt);
  Status addRows(const std::vector<RowData>& rows,
                 const std::vector<int>& rowIds = {});

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
   * @exception Throws std::invalid_argument if the newColNames vector does not
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

  struct ConfiguredColumn
  {
    std::string name;
    IO::BaseDataType dataType;
    std::shared_ptr<VectorData> column;
  };

  /**
   * @brief Look up an already-configured column by name.
   *
   * Configured columns are created and registered once during initialize()
   * via the DataSpec mechanism. This accessor allows subclasses (and finalize()
   * logic) to write data into the existing VectorData objects rather than
   * creating new ones, which would register duplicate recording objects.
   *
   * @param name The name of the configured column to retrieve.
   * @return A shared pointer to the configured VectorData column, or nullptr if
   * no column with the given name has been configured.
   */
  std::shared_ptr<VectorData> getConfiguredColumn(
      const std::string& name) const;

  /**
   * @brief Add a column to the list of configured columns.
   * @param column A shared pointer to the `VectorData` column to add.
   * @return The index of the added column in the list of configured columns.
   */
  SizeType addConfiguredColumn(const std::shared_ptr<VectorData>& column);

  /**
   * @brief Configure multiple data objects.
   *
   * @param dataSpecs The specifications for the data objects to configure.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status configureDataObjects(const std::vector<DataSpecPtr>& dataSpecs);

  /**
   * @brief Configure a single data object.
   *
   * @param dataSpec The specification for the data object to configure.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status configureDataObject(const DataSpec& dataSpec);

  /**
   * @brief Ensure that all configured columns are loaded from the file.
   *
   * This function checks whether all configured columns have been loaded into
   * memory. If any configured column is not yet loaded, it attempts to load it
   * from the file.
   *
   * @return Status::Success if all configured columns are loaded successfully,
   * otherwise Status::Failure.
   */
  Status ensureConfiguredColumnsLoaded();

  Status loadConfiguredColumnsFromFile();

  /**
   * @brief Helper method to check if all required column names are present in
   * the data specifications.
   *
   * @param requiredNames The list of required column names.
   * @param dataSpecs The data specifications to check against.
   * @return Status::Success if all required names are present, otherwise
   * Status::Failure.
   */
  Status checkRequiredColumnNames(
      const std::vector<std::string>& requiredNames,
      const std::vector<DataSpecPtr>& dataSpecs) const;
  /**
   * @brief Write a buffer of data to a configured column.
   *
   * This function writes the provided buffer of data to the specified
   * configured column. The column must have been previously configured via
   * the DataSpec mechanism during initialize().
   *
   * @param configuredColumn The configured column to which to write data.
   * @param buffer The buffer of data to write.
   * @param rowCount The number of rows in the buffer.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status writeColumnBuffer(
      const ConfiguredColumn& configuredColumn,
      const IO::BaseDataType::BaseDataVectorVariant& buffer,
      SizeType rowCount);

  /**
   * @brief Generate row IDs for the table.
   *
   * @param rowCount The number of rows for which to generate IDs.
   * @return A vector of generated row IDs.
   */
  std::vector<int> generateRowIDs(SizeType rowCount);

  /**
   * @brief Names of the columns in the table.
   */
  std::vector<std::string> m_colNames;

  /**
   * @brief The row ids data object for write
   */
  std::shared_ptr<ElementIdentifiers> m_rowElementIdentifiers;

  std::vector<ConfiguredColumn> m_configuredColumns;
  std::unordered_map<std::string, SizeType> m_configuredColumnIndices;
};
}  // namespace AQNWB::NWB
