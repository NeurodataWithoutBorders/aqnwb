#include "nwb/file/ElectrodeTable.hpp"

#include "Channel.hpp"

using namespace AQNWB::NWB;

// ElectrodeTable

/** Constructor */
ElectrodeTable::ElectrodeTable(std::shared_ptr<BaseIO> io,
                               const std::string& description)
    : DynamicTable(electrodeTablePath,  // use the electrodeTablePath
                   io,
                   description,
                   {"group", "group_name", "location"})
{
}

/** Destructor */
ElectrodeTable::~ElectrodeTable() {}

/** Initialization function*/
void ElectrodeTable::initialize()
{
  // create group
  DynamicTable::initialize();

  electrodeDataset->setDataset(
      std::unique_ptr<BaseRecordingData>(m_io->createArrayDataSet(
          BaseDataType::I32, SizeArray {1}, SizeArray {1}, m_path + "id")));
  groupNamesDataset->setDataset(std::unique_ptr<BaseRecordingData>(
      m_io->createArrayDataSet(BaseDataType::STR(250),
                               SizeArray {0},
                               SizeArray {1},
                               m_path + "group_name")));
  locationsDataset->setDataset(std::unique_ptr<BaseRecordingData>(
      m_io->createArrayDataSet(BaseDataType::STR(250),
                               SizeArray {0},
                               SizeArray {1},
                               m_path + "location")));
}

void ElectrodeTable::addElectrodes(std::vector<Channel> channels)
{
  // create datasets
  for (const auto& ch : channels) {
    groupReferences.push_back(groupPathBase + ch.getGroupName());
    groupNames.push_back(ch.getGroupName());
    electrodeNumbers.push_back(ch.getGlobalIndex());
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
