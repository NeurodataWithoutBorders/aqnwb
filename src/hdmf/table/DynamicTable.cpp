#include "DynamicTable.hpp"

// DynamicTable

/** Constructor */
DynamicTable::DynamicTable(std::string path, std::shared_ptr<BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
DynamicTable::~DynamicTable() {}

/** Initialization function*/
void DynamicTable::initialize()
{
  io->setCommonNWBAttributes(path, "hdmf-common", "DynamicTable", getDescription());
  io->setAttribute(getColNames(), path, "colnames");
}

/** Add column to table */
void DynamicTable::addColumn(std::string name,
                             std::string colDescription,
                             std::unique_ptr<VectorData>& vectorData,
                             std::vector<std::string> values)
{
  if (vectorData->dataset == nullptr) {
    std::cerr << "VectorData dataset is not initialized" << std::endl;
  } else {
    for (size_t i = 0; i < values.size(); i++)
      vectorData->dataset->writeDataBlock(
          1, BaseDataType::STR(values[i].size()), &values[i]);
    io->setCommonNWBAttributes(
        path + name, "hdmf-common", "VectorData", colDescription);
  }
}

void DynamicTable::addColumn(std::string name,
                             std::string colDescription,
                             std::unique_ptr<ElementIdentifiers>& elementIDs,
                             std::vector<int> values)
{
  if (elementIDs->dataset == nullptr) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
  } else {
    elementIDs->dataset->writeDataBlock(
        values.size(), BaseDataType::I32, &values[0]);
    io->setCommonNWBAttributes(
        path + name, "hdmf-common", "ElementIdentifiers", colDescription);
  }
}

void DynamicTable::addColumn(std::string name,
                             std::string colDescription,
                             std::vector<std::string> values)
{
  if (values.empty()) {
    std::cerr << "Data to add to column is empty" << std::endl;
  } else {
    io->createReferenceDataSet(path + name, values);
    io->setCommonNWBAttributes(
        path + name, "hdmf-common", "VectorData", colDescription);
  }
}
