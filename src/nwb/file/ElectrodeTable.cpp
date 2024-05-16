#include "nwb/file/ElectrodeTable.hpp"

using namespace AQNWB::NWB;

// ElectrodeTable

/** Constructor */
ElectrodeTable::ElectrodeTable(const std::string& path,
                               std::shared_ptr<BaseIO> io,
                               std::vector<Channel> channels,
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
}

void ElectrodeTable::addElectrodes()
{
  // create datasets
  for (const auto& ch : channels) {
    groupReferences.push_back(
        groupPathBase
        + ch.groupName);  // TODO - initialize these with the size of channels
    groupNames.push_back(ch.groupName);
    electrodeNumbers.push_back(ch.globalIndex);
    locationNames.push_back("unknown");
  }

  // add columns
  setRowIDs(electrodeDataset, electrodeNumbers);
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
  return groupReferences[0];  // all channel in channelGroup should have the
                              // same groupName
}
