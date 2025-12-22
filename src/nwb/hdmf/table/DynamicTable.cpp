#include "nwb/hdmf/table/DynamicTable.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// DynamicTable
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(DynamicTable)

/** Constructor */
DynamicTable::DynamicTable(const std::string& path,
                           std::shared_ptr<IO::BaseIO> io)
    : Container(path, io)
    , m_colNames({})
    , m_recordingColumns(std::make_unique<IO::RecordingObjects>())
    , m_rowElementIdentifiers(
          ElementIdentifiers::create(AQNWB::mergePaths(path, "id"), io))
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
Status DynamicTable::initialize(const std::string& description)
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
  return containerStatus;
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
    return m_colNames.size() - 1;
  }
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
    Status writeStatus =
        dataset->writeDataBlock(std::vector<SizeType> {values.size()},
                                std::vector<SizeType> {0},
                                IO::BaseDataType::V_STR,
                                values);
    addColumnName(vectorData->getName());
    m_recordingColumns->addRecordingObject(vectorData);
    return writeStatus;
  }
}

Status DynamicTable::setRowIDs(
    const std::shared_ptr<ElementIdentifiers>& elementIDs,
    const std::vector<int>& values)
{
  if (!elementIDs->isInitialized()) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
    return Status::Failure;
  } else {
    auto ioPtr = getIO();
    if (!ioPtr) {
      std::cerr << "DynamicTable::setRowIDs IO object has been deleted."
                << std::endl;
      return Status::Failure;
    }

    Status writeDataStatus = elementIDs->recordData()->writeDataBlock(
        std::vector<SizeType>(1, values.size()),
        IO::BaseDataType::I32,
        &values[0]);
    Status createAttrsStatus =
        ioPtr->createCommonNWBAttributes(AQNWB::mergePaths(m_path, "id"),
                                         elementIDs->getNamespace(),
                                         elementIDs->getTypeName());
    return writeDataStatus && createAttrsStatus;
  }
}

Status DynamicTable::addReferenceColumn(const std::string& name,
                                        const std::string& colDescription,
                                        const std::vector<std::string>& values)
{
  // TODO: Similar to addColumn() we should check if the column already exists
  // and if so append to it rather than creating a new column. This currently
  // prevents append to work for ElectrodesTable.
  if (values.empty()) {
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
        columnPath, ioPtr, colDescription, values);
    if (refColumn == nullptr) {
      std::cerr << "Failed to create reference column" << std::endl;
      return Status::Failure;
    }
    addColumnName(name);
    m_recordingColumns->addRecordingObject(refColumn);
    return Status::Success;
  }
}

Status DynamicTable::finalize()
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "DynamicTable::finalize IO object has been deleted."
              << std::endl;
    return Status::Failure;
  }
  Status colNamesStatus = ioPtr->createAttribute(
      m_colNames,
      m_path,
      "colnames",
      true  // overwrite the attribute if it already exists
  );
  Status parentStatus = Container::finalize();
  return colNamesStatus && parentStatus;
}
