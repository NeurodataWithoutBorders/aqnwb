#include "DynamicTable.hpp"

// DynamicTable

/** Constructor */
DynamicTable::DynamicTable(const std::string& path, std::shared_ptr<BaseIO> io, const std::string& description)
    : Container(path, io), description(description)
{
}

/** Destructor */
DynamicTable::~DynamicTable() {}

/** Initialization function*/
void DynamicTable::initialize()
{
  io->createCommonNWBAttributes(path, "hdmf-common", "DynamicTable", getDescription());
  io->createAttribute(getColNames(), path, "colnames");
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
    for (SizeType i = 0; i < values.size(); i++)
      vectorData->dataset->writeDataBlock(
          1, BaseDataType::STR(values[i].size()), &values[i]);
    io->createCommonNWBAttributes(
        path + name, "hdmf-common", "VectorData", colDescription);
  }
}

void DynamicTable::addColumn(const std::string& name,
                             const std::string& colDescription,
                             std::unique_ptr<ElementIdentifiers>& elementIDs,
                             const std::vector<int>& values)
{
  if (elementIDs->dataset == nullptr) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
  } else {
    elementIDs->dataset->writeDataBlock(
        values.size(), BaseDataType::I32, &values[0]);
    io->createCommonNWBAttributes(
        path + name, "hdmf-common", "ElementIdentifiers", colDescription);
  }
}

void DynamicTable::addColumn(const std::string& name,
                             const std::string& colDescription,
                             const std::vector<std::string>& values)
{
  if (values.empty()) {
    std::cerr << "Data to add to column is empty" << std::endl;
  } else {
    io->createDataSetOfReferences(path + name, values);
    io->createCommonNWBAttributes(
        path + name, "hdmf-common", "VectorData", colDescription);
  }
}

std::string DynamicTable::getDescription() const {
    return description;
}