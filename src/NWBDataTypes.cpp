#include "NWBDataTypes.hpp"
#include "NWBFile.hpp"

// Container

/** Constructor */
Container::Container(std::string path, std::shared_ptr<BaseIO> io): path(path), io(io)
{
  // create group
  io->createGroup(path);
}

/** Destructor */
Container::~Container(){}

// Device

void Device::initialize()
{
  // create group
    io->setGroupAttributes(path, "core", "Device", "description");
    io->setAttribute("unknown", path, "manufacturer");
}

// DynamicTable

/** Initialization function*/
void DynamicTable::initialize(){}


/** Add column to table */
void DynamicTable::addColumn(std::string name, std::string description)
{
  // TODO - add check for existing columns
  io->setGroupAttributes(path + name, "hdmf-common", "VectorData", description);
}

void DynamicTable::addIdentifiers(std::string name, std::string description)
{
  io->setGroupAttributes(path + name, "hdmf-common", "ElementIdentifiers", description);
}

// ElectrodeTable

/** Initialization function*/
void ElectrodeTable::initialize(std::vector<int> channels){
  // create group
  DynamicTable::initialize();
  io->setGroupAttributes(path, "hdmf-common", "DynamicTable", "metadata about extracellular electrodes");

  // add required columns
  const std::vector<std::string> colnames = {"group", "group_name", "location"};
  io->setAttribute(colnames, path, "colnames");

  // create datasets
  for (auto ch : channels)
  {
    electrodeNumbers.push_back(ch); 
    groupNames.push_back("electrode" + std::to_string(ch)); 
    groupReferences.push_back("/general/extracellular_ephys/electrode" + std::to_string(ch));
  }
}

// Data

/** Constructor */
Data::Data(){}

/** Destructor */
Data::~Data(){}


// VectorData

/** Constructor */
VectorData::VectorData(){}

/** Destructor */
VectorData::~VectorData(){}
