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
    , m_electrodeDataset(std::make_unique<ElementIdentifiers>(
          AQNWB::mergePaths(electrodeTablePath, "id"), io))
    , m_groupNamesDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "group_name"), io))
    , m_locationsDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "location"), io))
{
}

ElectrodeTable::ElectrodeTable(const std::string& path,
                               std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodeTablePath, io)
    , m_electrodeDataset(std::make_unique<ElementIdentifiers>(
          AQNWB::mergePaths(electrodeTablePath, "id"), io))
    , m_groupNamesDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "group_name"), io))
    , m_locationsDataset(std::make_unique<VectorData>(
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

  m_electrodeDataset->initialize(std::unique_ptr<IO::BaseRecordingData>(
      m_io->createArrayDataSet(IO::BaseDataType::I32,
                               SizeArray {1},
                               SizeArray {1},
                               AQNWB::mergePaths(m_path, "id"))));
  m_groupNamesDataset->initialize(
      std::unique_ptr<IO::BaseRecordingData>(
          m_io->createArrayDataSet(IO::BaseDataType::STR(250),
                                   SizeArray {0},
                                   SizeArray {1},
                                   AQNWB::mergePaths(m_path, "group_name"))),
      "the name of the ElectrodeGroup this electrode is a part of");
  m_locationsDataset->initialize(
      std::unique_ptr<IO::BaseRecordingData>(
          m_io->createArrayDataSet(IO::BaseDataType::STR(250),
                                   SizeArray {0},
                                   SizeArray {1},
                                   AQNWB::mergePaths(m_path, "location"))),
      "the location of channel within the subject e.g. brain region");
}

void ElectrodeTable::addElectrodes(std::vector<Channel> channelsInput)
{
  // create datasets
  for (const auto& ch : channelsInput) {
    m_groupReferences.push_back(
        AQNWB::mergePaths(m_groupPathBase, ch.getGroupName()));
    m_groupNames.push_back(ch.getGroupName());
    m_electrodeNumbers.push_back(static_cast<int>(ch.getGlobalIndex()));
    m_locationNames.push_back("unknown");
  }
}

void ElectrodeTable::finalize()
{
  setRowIDs(m_electrodeDataset, m_electrodeNumbers);
  addColumn(m_groupNamesDataset, m_groupNames);
  addColumn(m_locationsDataset, m_locationNames);
  addReferenceColumn(
      "group",
      "a reference to the ElectrodeGroup this electrode is a part of",
      m_groupReferences);
}
