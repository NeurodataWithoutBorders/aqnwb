#include "nwb/hdmf/table/DynamicTable.hpp"

using namespace AQNWB::NWB;

// DynamicTable

/** Constructor */
DynamicTable::DynamicTable(const std::string& path,
                           std::shared_ptr<BaseIO> io,
                           const std::string& description)
    : Container(path, io)
    , m_description(description)
{
}

/** Destructor */
DynamicTable::~DynamicTable() {}

/** Initialization function*/
void DynamicTable::initialize()
{
  Container::initialize();

  m_io->createCommonNWBAttributes(
      m_path, "hdmf-common", "DynamicTable", getDescription());
  m_io->createAttribute(getColNames(), m_path, "colnames");
}

/** Add column to table */
void DynamicTable::addColumn(const std::string& name,
                             const std::string& colDescription,
                             std::unique_ptr<VectorData>& vectorData,
                             const std::vector<std::string>& values)
{
  if (!vectorData->isInitialized()) {
    std::cerr << "VectorData dataset is not initialized" << std::endl;
  } else {
    // write in loop because variable length string
    for (SizeType i = 0; i < values.size(); i++)
      vectorData->m_dataset->writeDataBlock(
          std::vector<SizeType>(1, 1),
          BaseDataType::STR(values[i].size() + 1),
          values[i].c_str());  // TODO - add tests for this
    m_io->createCommonNWBAttributes(
        m_path + name, "hdmf-common", "VectorData", colDescription);
  }
}

void DynamicTable::setRowIDs(std::unique_ptr<ElementIdentifiers>& elementIDs,
                             const std::vector<int>& values)
{
  if (!elementIDs->isInitialized()) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
  } else {
    elementIDs->m_dataset->writeDataBlock(
        std::vector<SizeType>(1, values.size()), BaseDataType::I32, &values[0]);
    m_io->createCommonNWBAttributes(
        m_path + "id", "hdmf-common", "ElementIdentifiers");
  }
}

void DynamicTable::addColumn(const std::string& name,
                             const std::string& colDescription,
                             const std::vector<std::string>& values)
{
  if (values.empty()) {
    std::cerr << "Data to add to column is empty" << std::endl;
  } else {
    m_io->createReferenceDataSet(m_path + name, values);
    m_io->createCommonNWBAttributes(
        m_path + name, "hdmf-common", "VectorData", colDescription);
  }
}
