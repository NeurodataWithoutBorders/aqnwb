#include "nwb/hdmf/table/DynamicTable.hpp"

using namespace AQNWB::NWB;

// DynamicTable

/** Constructor */
DynamicTable::DynamicTable(const std::string& path,
                           std::shared_ptr<BaseIO> io,
                           const std::string& description)
    : Container(path, io)
    , description(description)
{
}

/** Destructor */
DynamicTable::~DynamicTable() {}

/** Initialization function*/
void DynamicTable::initialize()
{
  Container::initialize();

  this->m_io->createCommonNWBAttributes(
      this->m_path, "hdmf-common", "DynamicTable", getDescription());
  this->m_io->createAttribute(getColNames(), this->m_path, "colnames");
}

/** Add column to table */
void DynamicTable::addColumn(const std::string& name,
                             const std::string& colDescription,
                             std::unique_ptr<VectorData>& vectorData,
                             const std::vector<std::string>& values)
{
  if (vectorData->dataset == nullptr) {
    std::cerr << "VectorData dataset is not initialized" << std::endl;
  } else {
    // write in loop because variable length string
    for (SizeType i = 0; i < values.size(); i++)
      vectorData->dataset->writeDataBlock(
          std::vector<SizeType>(1, 1),
          BaseDataType::STR(values[i].size() + 1),
          values[i].c_str());  // TODO - add tests for this
    this->m_io->createCommonNWBAttributes(
        this->m_path + name, "hdmf-common", "VectorData", colDescription);
  }
}

void DynamicTable::setRowIDs(std::unique_ptr<ElementIdentifiers>& elementIDs,
                             const std::vector<int>& values)
{
  if (elementIDs->dataset == nullptr) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
  } else {
    elementIDs->dataset->writeDataBlock(
        std::vector<SizeType>(1, values.size()), BaseDataType::I32, &values[0]);
    this->m_io->createCommonNWBAttributes(
        this->m_path + "id", "hdmf-common", "ElementIdentifiers");
  }
}

void DynamicTable::addColumn(const std::string& name,
                             const std::string& colDescription,
                             const std::vector<std::string>& values)
{
  if (values.empty()) {
    std::cerr << "Data to add to column is empty" << std::endl;
  } else {
    this->m_io->createReferenceDataSet(this->m_path + name, values);
    this->m_io->createCommonNWBAttributes(
        this->m_path + name, "hdmf-common", "VectorData", colDescription);
  }
}

// Getter for description
std::string DynamicTable::getDescription() const
{
  return description;
}

// Getter for colNames
const std::vector<std::string>& DynamicTable::getColNames()
{
  return colNames;
}
