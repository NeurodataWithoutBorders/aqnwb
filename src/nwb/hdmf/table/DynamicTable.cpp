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
}

/** Destructor */
DynamicTable::~DynamicTable() {}

/** Initialization function*/
void DynamicTable::initialize(const std::string& description,
                              const std::vector<std::string>& colNames)
{
  Container::initialize();
  m_colNames = colNames;
  if (description != "")
    m_io->createAttribute(description, m_path, "description");
  m_io->createAttribute(m_colNames, m_path, "colnames");
}

/** Add column to table */
void DynamicTable::addColumn(std::unique_ptr<VectorData>& vectorData,
                             const std::vector<std::string>& values)
{
  if (!vectorData->isInitialized()) {
    std::cerr << "VectorData dataset is not initialized" << std::endl;
  } else {
    // write in loop because variable length string
    for (SizeType i = 0; i < values.size(); i++)
      vectorData->m_dataset->writeDataBlock(
          std::vector<SizeType> {1},
          std::vector<SizeType> {i},
          IO::BaseDataType::STR(values[i].size() + 1),
          values);  // TODO - add tests for this
  }
}

void DynamicTable::setRowIDs(std::unique_ptr<ElementIdentifiers>& elementIDs,
                             const std::vector<int>& values)
{
  if (!elementIDs->isInitialized()) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
  } else {
    elementIDs->m_dataset->writeDataBlock(
        std::vector<SizeType>(1, values.size()),
        IO::BaseDataType::I32,
        &values[0]);
    m_io->createCommonNWBAttributes(AQNWB::mergePaths(m_path, "id"),
                                    elementIDs->getNamespace(),
                                    elementIDs->getTypeName());
  }
}

void DynamicTable::addReferenceColumn(const std::string& name,
                                      const std::string& colDescription,
                                      const std::vector<std::string>& values)
{
  if (values.empty()) {
    std::cerr << "Data to add to column is empty" << std::endl;
  } else {
    std::string columnPath = AQNWB::mergePaths(m_path, name);
    m_io->createReferenceDataSet(columnPath, values);
    auto refColumn = AQNWB::NWB::VectorData(columnPath, m_io);
    refColumn.initialize(nullptr,  // Use nullptr because we only want to create
                                   // the attributes but not modify the data
                         colDescription);
  }
}
