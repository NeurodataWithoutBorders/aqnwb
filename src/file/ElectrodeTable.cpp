#include "ElectrodeTable.hpp"

using namespace AQNWBIO;

// ElectrodeTable

/** Constructor */
ElectrodeTable::ElectrodeTable(const std::string& path,
                               std::shared_ptr<BaseIO> io,
                               const std::vector<int>& channels,
                               const std::string& description)
    : DynamicTable(path, io, description)
    , channels(channels)
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

// Getter for colNames
const std::vector<std::string>& ElectrodeTable::getColNames()
{
  return colNames;
}

// Setter for colNames
void ElectrodeTable::setColNames(const std::vector<std::string>& newColNames)
{
  colNames = newColNames;
}

// Getter for groupPath
std::string ElectrodeTable::getGroupPath() const
{
  return groupPath;
}

// Setter for colNames
void ElectrodeTable::setGroupPath(const std::string& newgroupPath)
{
  groupPath = newgroupPath;
}
