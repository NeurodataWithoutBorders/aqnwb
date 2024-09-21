#include "nwb/file/ElectrodeTable.hpp"

#include "Channel.hpp"
#include "Utils.hpp"

using namespace AQNWB::NWB;

// ElectrodeTable
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ElectrodeTable)

/** Constructor */
ElectrodeTable::ElectrodeTable(std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodeTablePath,  // use the electrodeTablePath
                   io,
                   {"group", "group_name", "location"})
    , electrodeDataset(std::make_unique<ElementIdentifiers>(
          AQNWB::mergePaths(electrodeTablePath, "id"), io))
    , groupNamesDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "group_name"), io))
    , locationsDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "location"), io))
{
}

ElectrodeTable::ElectrodeTable(const std::string& path,
                               std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(
          electrodeTablePath,  // use the electrodeTablePath
          io)  // TODO May need to initialize the colNames in DynamicTable
    , electrodeDataset(std::make_unique<ElementIdentifiers>(
          AQNWB::mergePaths(electrodeTablePath, "id"), io))
    , groupNamesDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "group_name"), io))
    , locationsDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "location"), io))
{
  std::cerr << "ElectrodeTable object is required to appear at "
            << this->electrodeTablePath << std::endl;
  assert(path == this->electrodeTablePath);
}

/** Destructor */
ElectrodeTable::~ElectrodeTable() {}

/** Initialization function*/
void ElectrodeTable::initialize(const std::string& description)
{
  // create group
  DynamicTable::initialize(description);

  electrodeDataset->setDataset(std::unique_ptr<IO::BaseRecordingData>(
      m_io->createArrayDataSet(IO::BaseDataType::I32,
                               SizeArray {1},
                               SizeArray {1},
                               AQNWB::mergePaths(m_path, "id"))));
  groupNamesDataset->setDataset(std::unique_ptr<IO::BaseRecordingData>(
      m_io->createArrayDataSet(IO::BaseDataType::STR(250),
                               SizeArray {0},
                               SizeArray {1},
                               AQNWB::mergePaths(m_path, "group_name"))));
  locationsDataset->setDataset(std::unique_ptr<IO::BaseRecordingData>(
      m_io->createArrayDataSet(IO::BaseDataType::STR(250),
                               SizeArray {0},
                               SizeArray {1},
                               AQNWB::mergePaths(m_path, "location"))));
}

void ElectrodeTable::addElectrodes(std::vector<Channel> channels)
{
  // create datasets
  for (const auto& ch : channels) {
    groupReferences.push_back(
        AQNWB::mergePaths(groupPathBase, ch.getGroupName()));
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
