#include "nwb/file/ElectrodeTable.hpp"

#include "Channel.hpp"

using namespace AQNWB::NWB;

// ElectrodeTable

/** Constructor */
ElectrodeTable::ElectrodeTable(std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodeTablePath,  // use the electrodeTablePath
                   io)
{
}

ElectrodeTable::ElectrodeTable(const std::string& path,
                               std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodeTablePath,  // use the electrodeTablePath
                   io)
{
  assert(path == electrodeTablePath);
}

/** Destructor */
ElectrodeTable::~ElectrodeTable() {}

/** Initialization function*/
void ElectrodeTable::initialize(const std::string& description)
{
  // create group
  DynamicTable::initialize(description);

  electrodeDataset->dataset =
      std::unique_ptr<IO::BaseRecordingData>(io->createArrayDataSet(
          IO::BaseDataType::I32, SizeArray {1}, SizeArray {1}, path + "id"));
  groupNamesDataset->dataset = std::unique_ptr<IO::BaseRecordingData>(
      io->createArrayDataSet(IO::BaseDataType::STR(250),
                             SizeArray {0},
                             SizeArray {1},
                             path + "group_name"));
  locationsDataset
      ->dataset = std::unique_ptr<IO::BaseRecordingData>(io->createArrayDataSet(
      IO::BaseDataType::STR(250), SizeArray {0}, SizeArray {1}, path + "location"));
}

void ElectrodeTable::addElectrodes(std::vector<Channel> channels)
{
  // create datasets
  for (const auto& ch : channels) {
    groupReferences.push_back(groupPathBase + ch.groupName);
    groupNames.push_back(ch.groupName);
    electrodeNumbers.push_back(ch.globalIndex);
    locationNames.push_back("unknown");
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
