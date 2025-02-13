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
{
  // Read the colNames attribute if it exists such that any columns
  // we may add append to the existing list of columns rather than
  // replacing it. This is important for the finalize function
  // to ensure that all columns are correctly listed.
  if (m_io->isOpen()) {
    auto colNamesFromFile = readColNames();
    if (colNamesFromFile->exists()) {
      m_colNames = colNamesFromFile->values().data;
    }
  }
}

/** Destructor */
DynamicTable::~DynamicTable() {}

/** Initialization function*/
Status DynamicTable::initialize(const std::string& description)
{
  Status containerStatus = Container::initialize();
  if (description != "") {
    m_io->createAttribute(description, m_path, "description");
  }
  return containerStatus;
}

/** Add column to table */
Status DynamicTable::addColumn(
    std::unique_ptr<VectorData<std::string>>& vectorData,
    const std::vector<std::string>& values)
{
  if (!vectorData->isInitialized()) {
    std::cerr << "VectorData dataset is not initialized" << std::endl;
    return Status::Failure;
  } else {
    // Write all strings in a single block
    Status writeStatus = vectorData->m_dataset->writeDataBlock(
        std::vector<SizeType> {values.size()},
        std::vector<SizeType> {0},
        IO::BaseDataType::V_STR,
        values);
    m_colNames.push_back(vectorData->getName());
    return writeStatus;
  }
}

Status DynamicTable::setRowIDs(std::unique_ptr<ElementIdentifiers>& elementIDs,
                               const std::vector<int>& values)
{
  if (!elementIDs->isInitialized()) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
    return Status::Failure;
  } else {
    Status writeDataStatus = elementIDs->m_dataset->writeDataBlock(
        std::vector<SizeType>(1, values.size()),
        IO::BaseDataType::I32,
        &values[0]);
    Status createAttrsStatus =
        m_io->createCommonNWBAttributes(AQNWB::mergePaths(m_path, "id"),
                                        elementIDs->getNamespace(),
                                        elementIDs->getTypeName());
    return writeDataStatus && createAttrsStatus;
  }
}

Status DynamicTable::addReferenceColumn(const std::string& name,
                                        const std::string& colDescription,
                                        const std::vector<std::string>& values)
{
  if (values.empty()) {
    std::cerr << "Data to add to column is empty" << std::endl;
    return Status::Failure;
  } else {
    std::string columnPath = AQNWB::mergePaths(m_path, name);
    Status dataStatus = m_io->createReferenceDataSet(columnPath, values);
    auto refColumn = AQNWB::NWB::VectorData<std::string>(columnPath, m_io);
    Status vectorDataStatus = refColumn.initialize(
        nullptr,  // Use nullptr because we only want to create
                  // the attributes but not modify the data
        colDescription);
    m_colNames.push_back(name);
    return dataStatus && vectorDataStatus;
  }
}

Status DynamicTable::finalize()
{
  Status colNamesStatus = m_io->createAttribute(
      m_colNames,
      m_path,
      "colnames",
      true  // overwrite the attribute if it already exists
  );
  return colNamesStatus;
}
