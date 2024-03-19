#include "ElectrodeTable.hpp" 

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
