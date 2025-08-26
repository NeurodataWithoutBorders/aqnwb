#include "nwb/file/ElectrodesTable.hpp"

#include "Channel.hpp"
#include "Utils.hpp"

using namespace AQNWB::NWB;

// ElectrodesTable
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ElectrodesTable)

/** Constructor */
ElectrodesTable::ElectrodesTable(std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodeTablePath,  // use the electrodeTablePath
                   io)
    , m_electrodeDataset(std::make_unique<ElementIdentifiers>(
          AQNWB::mergePaths(electrodeTablePath, "id"), io))
    , m_groupNamesDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "group_name"), io))
    , m_locationsDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "location"), io))
{
}

ElectrodesTable::ElectrodesTable(const std::string& path,
                                 std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodeTablePath, io)
    , m_electrodeDataset(std::make_unique<ElementIdentifiers>(
          AQNWB::mergePaths(electrodeTablePath, "id"), io))
    , m_groupNamesDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "group_name"), io))
    , m_locationsDataset(std::make_unique<VectorData>(
          AQNWB::mergePaths(electrodeTablePath, "location"), io))
{
  assert(path == this->electrodeTablePath && "ElectrodesTable object is required to appear at /general/extracellular_ephys/electrodes");
}

/** Destructor */
ElectrodesTable::~ElectrodesTable() {}

/** Initialization function*/
Status ElectrodesTable::initialize(const std::string& description)
{
  // create group
  DynamicTable::initialize(description);
  // IO::BaseDataType vstrType(IO::BaseDataType::Type::V_STR,
  //                           0);  // 0 indicates variable length

  IO::ArrayDataSetConfig electrodeConfig(
      IO::BaseDataType::I32, SizeArray {1}, SizeArray {1});
  Status electrodeStatus = m_electrodeDataset->initialize(
      std::unique_ptr<IO::BaseRecordingData>(m_io->createArrayDataSet(
          electrodeConfig, AQNWB::mergePaths(m_path, "id"))));
  IO::ArrayDataSetConfig groupNameConfig(
      IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {1});
  Status groupNameStatus = m_groupNamesDataset->initialize(
      std::unique_ptr<IO::BaseRecordingData>(m_io->createArrayDataSet(
          groupNameConfig, AQNWB::mergePaths(m_path, "group_name"))),
      "the name of the ElectrodeGroup this electrode is a part of");
  IO::ArrayDataSetConfig locationConfig(
      IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {1});
  Status locationStatus = m_locationsDataset->initialize(
      std::unique_ptr<IO::BaseRecordingData>(m_io->createArrayDataSet(
          locationConfig, AQNWB::mergePaths(m_path, "location"))),
      "the location of channel within the subject e.g. brain region");
  return electrodeStatus && groupNameStatus && locationStatus;
}

void ElectrodesTable::addElectrodes(std::vector<Channel> channelsInput)
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

Status ElectrodesTable::finalize()
{
  Status rowIdStatus = setRowIDs(m_electrodeDataset, m_electrodeNumbers);
  Status locationColStatus = addColumn(m_locationsDataset, m_locationNames);
  Status groupColStatus = addReferenceColumn(
      "group",
      "a reference to the ElectrodeGroup this electrode is a part of",
      m_groupReferences);
  Status groupNameColStatus = addColumn(m_groupNamesDataset, m_groupNames);
  Status finalizeStatus = DynamicTable::finalize();
  return rowIdStatus && locationColStatus && groupColStatus
      && groupNameColStatus && finalizeStatus;
}
