#include "nwb/hdmf/table/DynamicTable.hpp"

using namespace AQNWB::NWB;

// DynamicTable
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(DynamicTable)

/** Constructor */
DynamicTable::DynamicTable(const std::string& path,
                           std::shared_ptr<IO::BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
DynamicTable::~DynamicTable() {}

/** Initialization function*/
void DynamicTable::initialize(const std::string& description)
{
  Container::initialize();

  io->createCommonNWBAttributes(
      this->path, this->getNamespace(), this->getTypeName(), description);
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
    // write in loop because variable length string
    for (SizeType i = 0; i < values.size(); i++)
      vectorData->dataset->writeDataBlock(
          std::vector<SizeType>(1, 1),
          IO::BaseDataType::STR(values[i].size() + 1),
          values[i].c_str());  // TODO - add tests for this
    io->createCommonNWBAttributes(
        this->path + name,
        vectorData->getNamespace(),
        vectorData->getTypeName(),
        colDescription);  // TODO should this be path + "/" + name
  }
}

void DynamicTable::setRowIDs(std::unique_ptr<ElementIdentifiers>& elementIDs,
                             const std::vector<int>& values)
{
  if (elementIDs->dataset == nullptr) {
    std::cerr << "ElementIdentifiers dataset is not initialized" << std::endl;
  } else {
    elementIDs->dataset->writeDataBlock(std::vector<SizeType>(1, values.size()),
                                        IO::BaseDataType::I32,
                                        &values[0]);
    io->createCommonNWBAttributes(
        this->path + "id",
        elementIDs->getNamespace(),
        elementIDs->getTypeName());  // TODO should this be path + "/id"
  }
}

void DynamicTable::addColumn(const std::string& name,
                             const std::string& colDescription,
                             const std::vector<std::string>& values)
{
  if (values.empty()) {
    std::cerr << "Data to add to column is empty" << std::endl;
  } else {
    io->createReferenceDataSet(path + name, values);
    io->createCommonNWBAttributes(
        path + name,
        "hdmf-common",
        "VectorData",
        colDescription);  // TODO this should use VectorData properly
  }
}

// Getter for colNames
const std::vector<std::string>& DynamicTable::getColNames()
{
  return colNames;
}
