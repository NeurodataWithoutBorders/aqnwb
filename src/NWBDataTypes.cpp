#include "NWBDataTypes.hpp"

#include "NWBFile.hpp"

// Data

/** Constructor */
Data::Data() {}

/** Destructor */
Data::~Data() {}

// Container

/** Constructor */
Container::Container(std::string path, std::shared_ptr<BaseIO> io)
    : path(path)
    , io(io)
{
  // create group if it does not exist
  io->createGroup(path);
}

/** Destructor */
Container::~Container() {}

// Device
/** Constructor */
Device::Device(std::string path, std::shared_ptr<BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
Device::~Device() {}

void Device::initialize()
{
  io->setGroupAttributes(path, "core", "Device", description);
  io->setAttribute(manufacturer, path, "manufacturer");
}

// ElectrodeGroup
// /** Constructor */
ElectrodeGroup::ElectrodeGroup(std::string path, std::shared_ptr<BaseIO> io)
    : Container(path, io)
{
}

/** Destructor */
ElectrodeGroup::~ElectrodeGroup() {}

void ElectrodeGroup::initialize()
{
  io->setGroupAttributes(path, "core", "ElectrodeGroup", description);
  io->setAttribute(location, path, "location");
}

void ElectrodeGroup::linkDevice()
{
  io->createLink("/" + path + "/device", "/" + device);
}
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
  io->setGroupAttributes(path, "hdmf-common", "DynamicTable", getDescription());
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
    io->setGroupAttributes(
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
        static_cast<int>(values.size()), BaseDataType::I32, &values[0]);
    io->setGroupAttributes(
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
    io->setGroupAttributes(
        path + name, "hdmf-common", "VectorData", colDescription);
  }
}

// ElectrodeTable

/** Constructor */
ElectrodeTable::ElectrodeTable(std::string path, std::shared_ptr<BaseIO> io)
    : DynamicTable(path, io)
{
}

/** Destructor */
ElectrodeTable::~ElectrodeTable() {}

/** Initialization function*/
void ElectrodeTable::initialize()
{
  // create group
  DynamicTable::initialize();

  // create datasets
  for (auto ch : channels) {
    groupReferences.push_back(
        groupPath);  // TODO - would get this info from channel input
    groupNames.push_back(
        "array1");  // TODO - would get this info from channel input
    electrodeNumbers.push_back(ch);
    locationNames.push_back("unknown");
  }

  // add columns
  addColumn("id",
            "a reference to the ElectrodeGroup this electrode is a part of",
            electrodeDataset,
            electrodeNumbers);
  addColumn("group_name",
            "the name of the ElectrodeGroup this electrode is a part of",
            groupNamesDataset,
            groupNames);
  addColumn("location",
            "the location of channel within the subject e.g. brain region",
            locationsDataset,
            locationNames);
  addColumn("group",
            "a reference to the ElectrodeGroup this electrode is a part of",
            groupReferences);
}

std::vector<std::string> ElectrodeTable::getColNames() const
{
  return colnames;
}

std::string ElectrodeTable::getDescription() const
{
  return description;
}