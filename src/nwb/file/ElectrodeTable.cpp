#include "nwb/file/ElectrodeTable.hpp"

#include "Channel.hpp"

using namespace AQNWB::NWB;

// ElectrodeTable

/** Constructor */
ElectrodeTable::ElectrodeTable(std::shared_ptr<BaseIO> io,
                               const std::string& description)
    : DynamicTable(electrodeTablePath,  // use the electrodeTablePath
                   io,
                   description)
{
}

/** Destructor */
ElectrodeTable::~ElectrodeTable() {}

/** Initialization function*/
void ElectrodeTable::initialize()
{
  // create group
  DynamicTable::initialize();

  electrodeDataset->dataset =
      std::unique_ptr<BaseRecordingData>(io->createArrayDataSet(
          BaseDataType::I32, SizeArray {1}, SizeArray {1}, path + "id"));
  groupNamesDataset->dataset = std::unique_ptr<BaseRecordingData>(
      io->createArrayDataSet(BaseDataType::STR(250),
                             SizeArray {0},
                             SizeArray {1},
                             path + "group_name"));
  locationsDataset
      ->dataset = std::unique_ptr<BaseRecordingData>(io->createArrayDataSet(
      BaseDataType::STR(250), SizeArray {0}, SizeArray {1}, path + "location"));
}

void ElectrodeTable::addElectrodes(std::vector<Channel> channels)
{
  // create datasets
  for (const auto& ch : channels) {
    // if the electrode number (row ID) already exists, skip it
    if (std::find(
            electrodeNumbers.begin(), electrodeNumbers.end(), ch.globalIndex)
        == electrodeNumbers.end())
    {
      groupReferences.push_back(groupPathBase + ch.groupName);
      groupNames.push_back(ch.groupName);
      electrodeNumbers.push_back(ch.globalIndex);
      locationNames.push_back("unknown");
    }
  }
}

void ElectrodeTable::finalize()
{
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
  return groupReferences[0];  // all channels in ChannelVector should have the
                              // same groupName
}
