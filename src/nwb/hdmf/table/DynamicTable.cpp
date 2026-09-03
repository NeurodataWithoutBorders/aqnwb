#include <algorithm>
#include <type_traits>

#include "nwb/hdmf/table/DynamicTable.hpp"

#include "Utils.hpp"
#include "nwb/hdmf/table/MeaningsTable.hpp"
#include "nwb/hdmf/table/VectorIndex.hpp"

using namespace AQNWB::NWB;

#include <unordered_set>

// DynamicTable
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(DynamicTable)

/** Constructor */
DynamicTable::DynamicTable(const std::string& path,
                           std::shared_ptr<IO::BaseIO> io)
    : Container(path, io)
    , m_colNames({})
    , m_rowElementIdentifiers(nullptr)
{
  // Read the colNames attribute if it exists such that any columns
  // we may add append to the existing list of columns rather than
  // replacing it. This is important for the finalize function
  // to ensure that all columns are correctly listed.
  auto ioPtr = getIO();
  if (ioPtr) {
    if (ioPtr->isOpen()) {
      auto colNamesFromFile = readColNames();
      if (colNamesFromFile->exists()) {
        m_colNames = colNamesFromFile->values().data;
      }
    }
  }
}

/** Destructor */
DynamicTable::~DynamicTable() {}

/** Initialization function*/
Status DynamicTable::initialize(const std::string& description,
                                const std::vector<DataSpecPtr>& dataSpecs)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "DynamicTable::initialize IO object has been deleted."
              << std::endl;
    return Status::Failure;
  }

  Status containerStatus = Container::initialize();
  if (description != "") {
    ioPtr->createAttribute(description, m_path, "description");
  }

  const auto effectiveSpecs =
      dataSpecs.empty() ? createDefaultDataSpecs() : dataSpecs;

  Status validationStatus = validateDataSpecs(effectiveSpecs);
  if (validationStatus != Status::Success) {
    throw std::invalid_argument(
        "DynamicTable::initialize: provided dataSpecs are invalid.");
  }

  clearColumns();
  Status configureStatus = configureDataObjects(effectiveSpecs);
  return containerStatus && configureStatus;
}

Status DynamicTable::validateDataSpecs(
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  return checkRequiredColumnSpec<ElementIdentifiers::DataSpec>(
      "id", dataSpecs, IO::BaseDataType::I32);
}

std::vector<DynamicTable::DataSpecPtr> DynamicTable::createDefaultDataSpecs(
    const SizeType rowChunkSize)
{
  return {ElementIdentifiers::createDataSpec(
      "id",
      IO::ArrayDataSetConfig(
          IO::BaseDataType::I32, SizeArray {0}, SizeArray {rowChunkSize}))};
}

void DynamicTable::setColNames(const std::vector<std::string>& newColNames)
{
  if (newColNames == m_colNames) {
    return;
  }

  // Ensure that the new column names are a permutation of the existing column
  // names. This check ensures that all existing columns are present in the new
  // list, but allows for reordering.
  if (newColNames.size() != m_colNames.size()
      || !std::is_permutation(
          newColNames.begin(), newColNames.end(), m_colNames.begin()))
  {
    std::cerr << "New column names do not match existing column names. "
              << "All columns must be present in the newColNames vector."
              << std::endl;
    throw std::invalid_argument(
        "New column names do not match existing column names.");
  }

  m_colNames = newColNames;
  flushColNames();
}

SizeType DynamicTable::addColumnName(const std::string& colName)
{
  auto it = std::find(m_colNames.begin(), m_colNames.end(), colName);
  if (it != m_colNames.end()) {
    // Column name already exists, return its index
    return static_cast<SizeType>(std::distance(m_colNames.begin(), it));
  } else {
    // Column name does not exist, add it and return new index
    m_colNames.push_back(colName);
    flushColNames();
    return m_colNames.size() - 1;
  }
}

Status DynamicTable::addColumn(const DataSpecPtr& dataSpec)
{
  if (!dataSpec) {
    std::cerr << "DynamicTable::addColumn received null DataSpec." << std::endl;
    return Status::Failure;
  }
  return configureDataObject(*dataSpec);
}

/** Add column to table */
Status DynamicTable::addColumn(const std::shared_ptr<VectorData>& vectorData,
                               const std::vector<std::string>& values)
{
  if (!vectorData->isInitialized()) {
    std::cerr << "VectorData dataset is not initialized "
              << vectorData->getPath() << std::endl;
    return Status::Failure;
  } else {
    // Write all strings in a single block
    auto dataset = vectorData->recordData();
    Status writeStatus = dataset->writeDataBlock(SizeArray {values.size()},
                                                 SizeArray {0},
                                                 IO::BaseDataType::V_STR,
                                                 values);
    registerColumn(vectorData);
    return writeStatus;
  }
}

Status DynamicTable::addColumn(const std::shared_ptr<VectorData>& vectorData)
{
  if (!vectorData) {
    std::cerr << "VectorData column is null" << std::endl;
    return Status::Failure;
  }
  if (!vectorData->isInitialized()) {
    std::cerr << "VectorData dataset is not initialized "
              << vectorData->getPath() << std::endl;
    return Status::Failure;
  }

  registerColumn(vectorData);
  return Status::Success;
}

Status DynamicTable::setRowIDs(const std::vector<int>& values)
{
  if (!m_rowElementIdentifiers) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
    return Status::Failure;
  } else {
    auto ioPtr = getIO();
    if (!ioPtr) {
      std::cerr << "DynamicTable::setRowIDs IO object has been deleted."
                << std::endl;
      return Status::Failure;
    }

    if (values.empty()) {
      return Status::Success;
    }
    auto idData = m_rowElementIdentifiers->recordData();
    SizeArray positionOffset = {0};
    auto currentShape = idData->getShape();
    if (!currentShape.empty()) {
      positionOffset[0] = currentShape[0];
    }
    Status writeDataStatus = idData->writeDataBlock(SizeArray {values.size()},
                                                    positionOffset,
                                                    IO::BaseDataType::I32,
                                                    &values[0]);
    return writeDataStatus;
  }
}

Status DynamicTable::addRow(const AQNWB::Types::RowData& row,
                            const std::optional<int>& rowId)
{
  if (rowId.has_value()) {
    return addRows(std::vector<AQNWB::Types::RowData> {row},
                   std::vector<int> {*rowId});
  }
  return addRows(std::vector<AQNWB::Types::RowData> {row});
}

SizeType DynamicTable::getNumberOfRows() const
{
  auto idCol = readIdColumn();
  if (!idCol) {
    return 0;
  }
  auto shape = idCol->readData()->getShape();
  if (shape.empty()) {
    return 0;
  }
  return shape[0];
}

std::vector<AQNWB::Types::RowData> DynamicTable::readRows(
    SizeType start,
    SizeType count,
    const std::vector<std::string>& colNames,
    bool includeId)
{
  std::vector<AQNWB::Types::RowData> rows;

  if (m_colNames.empty()) {
    std::cerr << "DynamicTable::readRows no columns available." << std::endl;
    return rows;
  }

  // Determine which columns to read
  std::vector<std::string> columnsToRead =
      colNames.empty() ? m_colNames : colNames;

  // Add "id" to columnsToRead if it's not already there and includeId is true
  if (includeId) {
    auto it = std::find(columnsToRead.begin(), columnsToRead.end(), "id");
    if (it == columnsToRead.end()) {
      columnsToRead.insert(columnsToRead.begin(), "id");
    }
  }

  // 1. Determine the actual number of rows to read
  SizeType totalRows = getNumberOfRows();
  if (totalRows == 0) {
    std::cerr << "DynamicTable::readRows: Could not determine row count."
              << std::endl;
    return rows;
  }

  // Raise an error if start is out of bounds
  if (start >= totalRows) {
    throw std::invalid_argument(
        "DynamicTable::readRows: start index out of bounds.");
  }

  // Adjust count if it exceeds the available rows
  SizeType actualCount = count;
  if (count == AQNWB::Types::SizeTypeNotSet || start + count > totalRows) {
    actualCount = totalRows - start;
  }

  // 2. Read data for each column
  std::unordered_map<std::string, std::vector<AQNWB::Types::CellValue>>
      columnData;

  for (const auto& colName : columnsToRead) {
    if (colName == "id") {
      auto idCol = readIdColumn();
      if (idCol) {
        SizeArray startArray = {start};
        SizeArray countArray = {actualCount};
        columnData["id"] = idCol->readCellValues(startArray, countArray);
      }
      continue;
    }

    // Check if there is a VectorIndex for this column
    auto vectorIndex = readColumn<VectorIndex>(colName + "_index");

    if (vectorIndex) {
      // Read ragged array data using the VectorIndex
      columnData[colName] =
          vectorIndex->readIndexedCellValues(start, actualCount);
    } else {
      // Read regular column data
      auto col = readColumn<VectorData>(colName);
      if (col) {
        SizeArray startArray = {start};
        SizeArray countArray = {actualCount};
        columnData[colName] = col->readCellValues(startArray, countArray);
      } else {
        std::cerr << "DynamicTable::readRows: Could not read column '"
                  << colName << "'." << std::endl;
      }
    }
  }

  // 3. Assemble the rows
  rows.resize(actualCount);
  for (SizeType i = 0; i < actualCount; ++i) {
    for (const auto& [colName, data] : columnData) {
      if (i < data.size()) {
        rows[i][colName] = data[i];
      }
    }
  }

  return rows;
}

std::string DynamicTable::toString(SizeType start,
                                   SizeType count,
                                   const std::vector<std::string>& colNames,
                                   bool includeId)
{
  std::vector<AQNWB::Types::RowData> rows =
      readRows(start, count, colNames, includeId);
  if (rows.empty()) {
    return "Empty Table";
  }

  std::string result;

  // Determine columns to print
  std::vector<std::string> columnsToPrint =
      colNames.empty() ? m_colNames : colNames;
  if (includeId) {
    auto it = std::find(columnsToPrint.begin(), columnsToPrint.end(), "id");
    if (it == columnsToPrint.end()) {
      columnsToPrint.insert(columnsToPrint.begin(), "id");
    }
  }

  // Print header
  for (size_t i = 0; i < columnsToPrint.size(); ++i) {
    result += columnsToPrint[i];
    if (i < columnsToPrint.size() - 1) {
      result += ",";
    }
  }
  result += "\n";

  // Helper to escape fields for CSV (RFC 4180)
  auto csvEscape = [](std::string_view field) -> std::string
  {
    const bool needsQuotes =
        field.find_first_of(",\"\n\r") != std::string_view::npos
        || (!field.empty() && field.front() == '[' && field.back() == ']');
    if (!needsQuotes)
      return std::string(field);

    std::string out;
    out.reserve(field.size() + 2);
    out.push_back('"');
    for (char c : field) {
      if (c == '"')
        out.push_back('"');  // double it
      out.push_back(c);
    }
    out.push_back('"');
    return out;
  };

  // Print rows
  for (const auto& row : rows) {
    for (size_t i = 0; i < columnsToPrint.size(); ++i) {
      const auto& colName = columnsToPrint[i];
      auto it = row.find(colName);
      if (it != row.end()) {
        result += csvEscape(it->second.toString());
      } else {
        result += "NULL";
      }

      if (i < columnsToPrint.size() - 1) {
        result += ",";
      }
    }
    result += "\n";
  }

  return result;
}

Status DynamicTable::addRows(const std::vector<AQNWB::Types::RowData>& rows,
                             const std::vector<int>& rowIds)
{
  if (rows.empty()) {
    return Status::Success;
  }
  Status loadStatus = ensureConfiguredColumnsLoaded();
  if (loadStatus != Status::Success) {
    return Status::Failure;
  }
  if (m_configuredColumns.empty()) {
    std::cerr << "DynamicTable::addRows no configured columns available."
              << std::endl;
    return Status::Failure;
  }
  if (!rowIds.empty() && rowIds.size() != rows.size()) {
    std::cerr << "DynamicTable::addRows rowIds size must match rows size."
              << std::endl;
    return Status::Failure;
  }

  SizeType rowCount = rows.size();

  // 1. Identify target columns and cache them in VectorIndices
  std::unordered_set<std::string> targetColumns;
  for (const auto& col : m_configuredColumns) {
    auto vectorIndex = std::dynamic_pointer_cast<VectorIndex>(col.column);
    if (vectorIndex) {
      auto target = vectorIndex->readTarget();
      if (target) {
        targetColumns.insert(target->getName());
      }
    }
  }

  // 2. Validate that all rows have the required columns
  for (const auto& row : rows) {
    // 2.1 Validate the keys in the row, e.g., no unknown columns
    for (const auto& [key, value] : row) {
      if (key == "id") {
        continue;
      }
      if (m_configuredColumnIndices.find(key) == m_configuredColumnIndices.end()
          && targetColumns.find(key) == targetColumns.end())
      {
        std::cerr << "DynamicTable::addRows: Unknown column '" << key
                  << "' provided." << std::endl;
        return Status::Failure;
      }
    }
    // 2.2 Validate against the configured columns
    for (const auto& col : m_configuredColumns) {
      if (targetColumns.find(col.name) != targetColumns.end()) {
        // Target columns must be present in the row
        if (row.find(col.name) == row.end()) {
          std::cerr
              << "DynamicTable::addRows: Missing value for target column '"
              << col.name << "'." << std::endl;
          return Status::Failure;
        }
      } else {
        auto vectorIndex = std::dynamic_pointer_cast<VectorIndex>(col.column);
        if (!vectorIndex) {
          // Regular columns must be present in the row
          auto cell = row.find(col.name);
          if (cell == row.end()) {
            std::cerr << "DynamicTable::addRows: Missing value for column '"
                      << col.name << "'." << std::endl;
            return Status::Failure;
          }
          if (std::holds_alternative<AQNWB::Types::VectorDataVariant>(
                  cell->second.value))
          {
            std::cerr << "DynamicTable::addRows: Regular column '" << col.name
                      << "' does not accept vector-valued cells." << std::endl;
            return Status::Failure;
          }
        }
      }
    }
  }

  // 3. Buffer values for each column
  std::unordered_map<std::string, IO::BaseDataType::BaseDataVectorVariant>
      columnBuffers;
  std::unordered_map<std::string, std::vector<uint32_t>> indexBuffers;

  for (const auto& col : m_configuredColumns) {
    auto vectorIndex = std::dynamic_pointer_cast<VectorIndex>(col.column);
    if (vectorIndex) {
      auto target = vectorIndex->readTarget();
      if (!target)
        return Status::Failure;
      std::string targetName = target->getName();
      columnBuffers[targetName] = IO::BaseDataType::createEmptyVectorVariant(
          target->readData()->getDataType());
      indexBuffers[col.name] = std::vector<uint32_t>();
    } else if (targetColumns.find(col.name) == targetColumns.end()) {
      columnBuffers[col.name] =
          IO::BaseDataType::createEmptyVectorVariant(col.dataType);
    }
  }

  for (const auto& row : rows) {
    for (const auto& col : m_configuredColumns) {
      auto vectorIndex = std::dynamic_pointer_cast<VectorIndex>(col.column);
      if (vectorIndex) {
        auto target = vectorIndex->readTarget();
        if (!target)
          return Status::Failure;
        std::string targetName = target->getName();
        auto it = row.find(targetName);
        if (it != row.end()) {
          // Also need to account for existing elements in the target column
          size_t existingTargetElements = 0;
          auto targetData = target->readData();
          if (targetData) {
            auto shape = targetData->getShape();
            if (!shape.empty()) {
              existingTargetElements = shape[0];
            }
          }

          Status appendStatus =
              appendCellValueToBuffer(columnBuffers[targetName], it->second);
          if (appendStatus != Status::Success) {
            std::cerr << "DynamicTable::addRows: Failed to append row to "
                         "VectorIndex '"
                      << col.name << "'." << std::endl;
            return Status::Failure;
          }

          size_t newTargetSize = std::visit(
              [](const auto& vec) -> size_t
              {
                using VecType = std::decay_t<decltype(vec)>;
                if constexpr (std::is_same_v<VecType, std::monostate>)
                  return 0;
                else
                  return vec.size();
              },
              columnBuffers[targetName]);

          uint32_t newIndex =
              static_cast<uint32_t>(existingTargetElements + newTargetSize);
          indexBuffers[col.name].push_back(newIndex);
        }
      } else if (targetColumns.find(col.name) == targetColumns.end()) {
        // Regular column (not a target of a VectorIndex)
        auto it = row.find(col.name);
        if (it != row.end()) {
          Status appendStatus =
              appendCellValueToBuffer(columnBuffers[col.name], it->second);
          if (appendStatus != Status::Success) {
            std::cerr
                << "DynamicTable::addRows: Failed to append value to column '"
                << col.name << "'." << std::endl;
            return Status::Failure;
          }
        }
      }
    }
  }

  // 4. Write buffers to columns
  for (const auto& col : m_configuredColumns) {
    auto vectorIndex = std::dynamic_pointer_cast<VectorIndex>(col.column);
    if (vectorIndex) {
      auto target = vectorIndex->readTarget();
      if (!target)
        return Status::Failure;
      std::string targetName = target->getName();

      // Write target buffer
      if (columnBuffers.find(targetName) != columnBuffers.end()) {
        Status writeStatus = target->appendBuffer(columnBuffers[targetName]);
        if (writeStatus != Status::Success) {
          std::cerr << "DynamicTable::addRows: Failed to write buffer for "
                       "target column '"
                    << targetName << "'." << std::endl;
          return Status::Failure;
        }
        // Clear buffer so we don't write it again if multiple indices point to
        // it
        columnBuffers.erase(targetName);
      }

      // Write index buffer
      IO::BaseDataType::BaseDataVectorVariant indexVariant =
          indexBuffers[col.name];
      Status writeIndexStatus = vectorIndex->appendBuffer(indexVariant);
      if (writeIndexStatus != Status::Success) {
        std::cerr << "DynamicTable::addRows: Failed to write buffer for index "
                     "column '"
                  << col.name << "'." << std::endl;
        return Status::Failure;
      }
    } else if (targetColumns.find(col.name) == targetColumns.end()) {
      // Write regular column buffer
      Status writeStatus = col.column->appendBuffer(columnBuffers[col.name]);
      if (writeStatus != Status::Success) {
        std::cerr
            << "DynamicTable::addRows: Failed to write buffer for column '"
            << col.name << "'." << std::endl;
        return Status::Failure;
      }
    }
  }

  // 5. Handle IDs
  std::vector<int> finalRowIds = rowIds;
  if (finalRowIds.empty()) {
    finalRowIds = generateRowIDs(rowCount);
  } else if (finalRowIds.size() != rowCount) {
    std::cerr << "DynamicTable::addRows: Number of provided row IDs does not "
                 "match number of rows."
              << std::endl;
    return Status::Failure;
  }

  if (m_rowElementIdentifiers) {
    auto idData = m_rowElementIdentifiers->recordData();
    SizeArray positionOffset = {0};
    auto currentShape = idData->getShape();
    if (!currentShape.empty()) {
      positionOffset[0] = currentShape[0];
    }
    Status idWriteStatus = idData->writeDataBlock(SizeArray {rowCount},
                                                  positionOffset,
                                                  IO::BaseDataType::I32,
                                                  finalRowIds.data());
    if (idWriteStatus != Status::Success) {
      std::cerr << "DynamicTable::addRows: Failed to write row IDs."
                << std::endl;
      return Status::Failure;
    }
  }

  return Status::Success;
}

Status DynamicTable::addReferenceColumn(const std::string& name,
                                        const std::string& colDescription,
                                        const std::vector<std::string>& dataset)
{
  // TODO: Similar to addColumn() we should check if the column already exists
  // and if so append to it rather than creating a new column. This currently
  // prevents append to work for ElectrodesTable.
  if (dataset.empty()) {
    std::cerr << "Data to add to column is empty" << std::endl;
    return Status::Failure;
  } else {
    auto ioPtr = getIO();
    if (!ioPtr) {
      std::cerr
          << "DynamicTable::addReferenceColumn IO object has been deleted."
          << std::endl;
      return Status::Failure;
    }

    std::string columnPath = AQNWB::mergePaths(m_path, name);

    auto refColumn = AQNWB::NWB::VectorData::createReferenceVectorData(
        columnPath, ioPtr, colDescription, dataset);
    if (refColumn == nullptr) {
      std::cerr << "Failed to create reference column" << std::endl;
      return Status::Failure;
    }
    registerColumn(refColumn);
    return Status::Success;
  }
}

Status DynamicTable::flushColNames()
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "DynamicTable::flushColNames IO object has been deleted."
              << std::endl;
    return Status::Failure;
  }
  Status colNamesStatus = ioPtr->createAttribute(
      m_colNames,
      m_path,
      "colnames",
      true  // overwrite the attribute if it already exists
  );
  return colNamesStatus;
}

Status DynamicTable::finalize()
{
  Status parentStatus = Container::finalize();
  return parentStatus;
}

std::shared_ptr<MeaningsTable> DynamicTable::createMeaningsTable(
    const std::string& columnName, const SizeType rowChunkSize)
{
  // Get the I/O object an ensure it is valid
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "DynamicTable::createMeaningsTable IO object has been deleted."
              << std::endl;
    return nullptr;
  }

  // Check that the column VectorData exists in the DynamicTable
  auto columnVectorData = readColumn<VectorData>(columnName);
  if (!columnVectorData) {
    std::cerr << "Column VectorData '" << columnName
              << "' does not exist in DynamicTable '" << m_path
              << "'. Cannot create MeaningsTable." << std::endl;
    return nullptr;
  }
  auto valueDataType = columnVectorData->readData()->getDataType();

  // Check the meanings_tables group exists, if not create it
  std::string meaningsTablesGroupPath =
      AQNWB::mergePaths(m_path, "meanings_tables");
  if (!ioPtr->objectExists(meaningsTablesGroupPath)) {
    Status createGroupStatus = ioPtr->createGroup(meaningsTablesGroupPath);
    if (createGroupStatus != Status::Success) {
      std::cerr << "Failed to create meanings_tables group at '"
                << meaningsTablesGroupPath << "'." << std::endl;
      return nullptr;
    }
  }

  // Create the MeaningsTable for the specified column
  std::string meaningsTablePath =
      AQNWB::mergePaths(meaningsTablesGroupPath, columnName + "_meanings");
  auto meaningsTable = MeaningsTable::create(meaningsTablePath, ioPtr);
  if (!meaningsTable) {
    std::cerr << "Failed to create MeaningsTable at '" << meaningsTablePath
              << "'." << std::endl;
    return nullptr;
  }

  // Initialize the MeaningsTable with the target VectorData and value data type
  auto specs =
      MeaningsTable::createDefaultDataSpecs(valueDataType, rowChunkSize);
  Status initStatus =
      meaningsTable->initialize(*columnVectorData,
                                valueDataType,
                                "Meanings table for column: " + columnName,
                                specs);

  // Report error if initialization failed
  if (initStatus != Status::Success) {
    std::cerr << "Failed to initialize MeaningsTable at '" << meaningsTablePath
              << "'." << std::endl;
    return nullptr;
  }

  // Return the created MeaningsTable
  return meaningsTable;
}

std::shared_ptr<MeaningsTable> DynamicTable::readMeaningsTable(
    const std::string& objectName) const
{
  std::string prefixPath = AQNWB::mergePaths(m_path, "meanings_tables");
  std::string objectPath = AQNWB::mergePaths(prefixPath, objectName);
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "IO object has been deleted. Can't read field: " << objectPath
              << std::endl;
    return nullptr;
  }
  if (ioPtr->objectExists(objectPath)) {
    return MeaningsTable::create(objectPath, ioPtr);
  }
  return nullptr;
}

std::shared_ptr<MeaningsTable> DynamicTable::createMeaningsTableInstance(
    const std::string& objectName) const
{
  std::string prefixPath = AQNWB::mergePaths(m_path, "meanings_tables");
  std::string objectPath = AQNWB::mergePaths(prefixPath, objectName);
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "IO object has been deleted. Can't create field: "
              << objectPath << std::endl;
    return nullptr;
  }
  return MeaningsTable::create(objectPath, ioPtr);
}

std::shared_ptr<VectorData> DynamicTable::getConfiguredColumn(
    const std::string& name) const
{
  auto it = m_configuredColumnIndices.find(name);
  if (it != m_configuredColumnIndices.end()) {
    return m_configuredColumns[it->second].column;
  }
  return nullptr;
}

void DynamicTable::clearColumns()
{
  m_configuredColumns.clear();
  m_configuredColumnIndices.clear();
  m_colNames.clear();
  m_rowElementIdentifiers.reset();
}

Status DynamicTable::configureDataObjects(
    const std::vector<DataSpecPtr>& dataSpecs)
{
  for (const auto& spec : dataSpecs) {
    if (!spec) {
      std::cerr << "DynamicTable::configureDataObjects received null spec."
                << std::endl;
      return Status::Failure;
    }
    Status status = configureDataObject(*spec);
    if (status != Status::Success) {
      return status;
    }
  }
  return Status::Success;
}

Status DynamicTable::configureDataObject(const DataSpec& dataSpec)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    return Status::Failure;
  }
  std::string columnPath = AQNWB::mergePaths(m_path, dataSpec.name);
  auto dataObj = dataSpec.create(columnPath, ioPtr);
  if (!dataObj) {
    return Status::Failure;
  }
  Status initStatus = dataSpec.initialize(*dataObj);
  if (initStatus != Status::Success) {
    return Status::Failure;
  }

  if (dataSpec.name == "id") {
    m_rowElementIdentifiers =
        std::dynamic_pointer_cast<ElementIdentifiers>(dataObj);
    return Status::Success;
  }

  auto vectorData = std::dynamic_pointer_cast<VectorData>(dataObj);
  if (!vectorData) {
    return Status::Failure;
  }

  registerColumn(vectorData);

  return Status::Success;
}

SizeType DynamicTable::registerColumn(const std::shared_ptr<VectorData>& column)
{
  if (!column) {
    std::cerr << "DynamicTable::registerColumn received null column."
              << std::endl;
    return static_cast<SizeType>(-1);
  }

  std::string name = column->getName();

  // If already registered, return its index.
  auto it = m_configuredColumnIndices.find(name);
  if (it != m_configuredColumnIndices.end()) {
    return it->second;
  }

  // Register column
  ConfiguredColumn config;
  config.name = name;
  config.dataType = column->readData()->getDataType();
  config.column = column;

  m_configuredColumns.push_back(config);
  SizeType index = m_configuredColumns.size() - 1;
  m_configuredColumnIndices[name] = index;

  // If the column is not a VectorIndex, add its name to the list of column
  // names Since we are here writing string values to the columns, our
  // vectorData should not be able to be a VectorIndex, but we check just in
  // case.
  bool isVectorIndex =
      (std::dynamic_pointer_cast<VectorIndex>(column) != nullptr);
  if (!isVectorIndex) {
    addColumnName(name);
  }
  // Return the index of the registered column.
  return index;
}

Status DynamicTable::ensureConfiguredColumnsLoaded()
{
  if (!m_configuredColumns.empty()) {
    return Status::Success;
  }
  return loadConfiguredColumnsFromFile();
}

Status DynamicTable::loadConfiguredColumnsFromFile()
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    return Status::Failure;
  }

  for (const auto& colName : m_colNames) {
    auto col = readColumn<VectorData>(colName);
    if (col) {
      registerColumn(col);
    }
  }

  m_rowElementIdentifiers = readIdColumn();
  return Status::Success;
}

std::vector<int> DynamicTable::generateRowIDs(SizeType rowCount)
{
  std::vector<int> ids(rowCount);
  int startId = 0;
  if (m_rowElementIdentifiers) {
    auto idData = m_rowElementIdentifiers->recordData();
    if (idData) {
      const auto& currentShape = idData->getShape();
      if (!currentShape.empty()) {
        startId = static_cast<int>(currentShape.front());
      }
    }
  }
  for (SizeType i = 0; i < rowCount; ++i) {
    ids[i] = startId + static_cast<int>(i);
  }
  return ids;
}
