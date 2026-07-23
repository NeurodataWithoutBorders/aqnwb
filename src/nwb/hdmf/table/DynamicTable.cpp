#include <algorithm>
#include <type_traits>

#include "nwb/hdmf/table/DynamicTable.hpp"

#include "Utils.hpp"
#include "nwb/hdmf/table/MeaningsTable.hpp"

using namespace AQNWB::NWB;

namespace
{
using BufferVariant = AQNWB::IO::BaseDataType::BaseDataVectorVariant;
using CellValue = AQNWB::IO::BaseDataType::BaseDataVariant;

template<typename T>
bool appendTypedCell(BufferVariant& buffer, const CellValue& cellValue)
{
  auto* typedValue = std::get_if<T>(&cellValue);
  if (!typedValue) {
    return false;
  }
  auto* typedBuffer = std::get_if<std::vector<T>>(&buffer);
  if (!typedBuffer) {
    return false;
  }
  typedBuffer->push_back(*typedValue);
  return true;
}

bool appendCellToBuffer(BufferVariant& buffer,
                        const AQNWB::IO::BaseDataType& dataType,
                        const CellValue& cellValue)
{
  switch (dataType.type) {
    case AQNWB::IO::BaseDataType::T_U8:
      return appendTypedCell<uint8_t>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_U16:
      return appendTypedCell<uint16_t>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_U32:
      return appendTypedCell<uint32_t>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_U64:
      return appendTypedCell<uint64_t>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_I8:
      return appendTypedCell<int8_t>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_I16:
      return appendTypedCell<int16_t>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_I32:
      return appendTypedCell<int32_t>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_I64:
      return appendTypedCell<int64_t>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_F32:
      return appendTypedCell<float>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_F64:
      return appendTypedCell<double>(buffer, cellValue);
    case AQNWB::IO::BaseDataType::T_STR:
    case AQNWB::IO::BaseDataType::V_STR:
      return appendTypedCell<std::string>(buffer, cellValue);
  }
  return false;
}
}  // namespace

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

  Status configureStatus = configureDataObjects(effectiveSpecs);
  return containerStatus && configureStatus;
}

Status DynamicTable::validateDataSpecs(
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  return checkRequiredColumnNames({"id"}, dataSpecs);
}

Status DynamicTable::checkRequiredColumnNames(
    const std::vector<std::string>& requiredNames,
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  if (dataSpecs.empty()) {
    std::cerr
        << "DynamicTable::checkRequiredColumnNames: dataSpecs vector is empty."
        << std::endl;
    return Status::Failure;
  }

  for (const auto& reqName : requiredNames) {
    bool found = false;
    for (const auto& spec : dataSpecs) {
      if (spec && spec->name == reqName) {
        found = true;
        break;
      }
    }

    if (!found) {
      std::cerr << "DynamicTable::checkRequiredColumnNames: required column '"
                << reqName << "' not found." << std::endl;
      return Status::Failure;
    }
  }

  return Status::Success;
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
    addColumnName(vectorData->getName());
    // If the column is not already in the list of configured columns, add it
    if (m_colNames.size() > m_configuredColumns.size()) {
      addConfiguredColumn(vectorData);
    }
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
  } else {
    addColumnName(vectorData->getName());
    // If the column is not already in the list of configured columns, add it
    if (m_colNames.size() > m_configuredColumns.size()) {
      addConfiguredColumn(vectorData);
    }
    return Status::Success;
  }
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

Status DynamicTable::addRow(const RowData& row, const std::optional<int>& rowId)
{
  if (rowId.has_value()) {
    return addRows(std::vector<RowData> {row}, std::vector<int> {*rowId});
  }
  return addRows(std::vector<RowData> {row});
}

Status DynamicTable::addRows(const std::vector<RowData>& rows,
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

  std::vector<BufferVariant> columnBuffers;
  columnBuffers.reserve(m_configuredColumns.size());
  for (const auto& configuredColumn : m_configuredColumns) {
    columnBuffers.push_back(
        IO::BaseDataType::createEmptyVectorVariant(configuredColumn.dataType));
  }

  for (const auto& row : rows) {
    if (row.size() != m_configuredColumns.size()) {
      std::cerr << "DynamicTable::addRows row size does not match configured "
                   "column count."
                << std::endl;
      return Status::Failure;
    }
    for (SizeType i = 0; i < m_configuredColumns.size(); ++i) {
      const auto& configuredColumn = m_configuredColumns[i];
      auto rowValueIt = row.find(configuredColumn.name);
      if (rowValueIt == row.end()) {
        std::cerr << "DynamicTable::addRows missing value for column '"
                  << configuredColumn.name << "'." << std::endl;
        return Status::Failure;
      }
      if (!appendCellToBuffer(
              columnBuffers[i], configuredColumn.dataType, rowValueIt->second))
      {
        std::cerr << "DynamicTable::addRows value type mismatch for column '"
                  << configuredColumn.name << "'." << std::endl;
        return Status::Failure;
      }
    }
  }

  Status status = Status::Success;
  for (SizeType i = 0; i < m_configuredColumns.size(); ++i) {
    status = status
        && writeColumnBuffer(
                 m_configuredColumns[i], columnBuffers[i], rows.size());
  }
  if (status != Status::Success) {
    return status;
  }

  std::vector<int> idsToWrite =
      rowIds.empty() ? generateRowIDs(rows.size()) : rowIds;
  return setRowIDs(idsToWrite);
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
    addColumnName(name);
    // If the column is not already in the list of configured columns, add it
    if (m_colNames.size() > m_configuredColumns.size()) {
      addConfiguredColumn(refColumn);
    }
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

Status DynamicTable::configureDataObjects(
    const std::vector<DataSpecPtr>& dataSpecs)
{
  m_configuredColumns.clear();
  m_configuredColumnIndices.clear();

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

  ConfiguredColumn col;
  col.name = dataSpec.name;
  col.dataType = dataSpec.getType();
  col.column = vectorData;

  m_configuredColumns.push_back(col);
  m_configuredColumnIndices[col.name] = m_configuredColumns.size() - 1;

  addColumnName(col.name);

  return Status::Success;
}

SizeType DynamicTable::addConfiguredColumn(
    const std::shared_ptr<VectorData>& column)
{
  if (!column) {
    std::cerr << "DynamicTable::addConfiguredColumn received null column."
              << std::endl;
    return static_cast<SizeType>(-1);
  }
  ConfiguredColumn config;
  config.name = column->getName();
  config.dataType = column->readData()->getDataType();
  config.column = column;

  m_configuredColumns.push_back(config);
  return m_configuredColumns.size() - 1;
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
      ConfiguredColumn confCol;
      confCol.name = colName;
      confCol.dataType = col->readData()->getDataType();
      confCol.column = col;
      m_configuredColumns.push_back(confCol);
      m_configuredColumnIndices[colName] = m_configuredColumns.size() - 1;
    }
  }
  return Status::Success;
}

Status DynamicTable::writeColumnBuffer(
    const ConfiguredColumn& configuredColumn,
    const IO::BaseDataType::BaseDataVectorVariant& buffer,
    SizeType rowCount)
{
  auto dataset = configuredColumn.column->recordData();
  SizeArray positionOffset = {0};
  auto currentShape = dataset->getShape();
  if (!currentShape.empty()) {
    positionOffset[0] = currentShape[0];
  }

  return std::visit(
      [&](auto&& arg) -> Status
      {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          return Status::Failure;
        } else {
          if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            return dataset->writeDataBlock(SizeArray {rowCount},
                                           positionOffset,
                                           configuredColumn.dataType,
                                           arg);
          } else {
            return dataset->writeDataBlock(SizeArray {rowCount},
                                           positionOffset,
                                           configuredColumn.dataType,
                                           arg.data());
          }
        }
      },
      buffer);
}

std::vector<int> DynamicTable::generateRowIDs(SizeType rowCount)
{
  std::vector<int> ids(rowCount);
  int startId = 0;
  if (m_rowElementIdentifiers) {
    auto idData = m_rowElementIdentifiers->recordData();
    auto currentShape = idData->getShape();
    if (!currentShape.empty()) {
      startId = static_cast<int>(currentShape[0]);
    }
  }
  for (SizeType i = 0; i < rowCount; ++i) {
    ids[i] = startId + static_cast<int>(i);
  }
  return ids;
}
