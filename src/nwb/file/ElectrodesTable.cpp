#include "nwb/file/ElectrodesTable.hpp"

#include "Channel.hpp"
#include "Utils.hpp"

using namespace AQNWB::NWB;

// ElectrodesTable
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ElectrodesTable)

/** Constructor */
ElectrodesTable::ElectrodesTable(std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodesTablePath,  // use the electrodesTablePath
                   io)
    , m_electrodeDataset(ElementIdentifiers::create(
          AQNWB::mergePaths(electrodesTablePath, "id"), io))
    , m_groupNamesVectorData(VectorData::create(
          AQNWB::mergePaths(electrodeTablePath, "group_name"), io))
    , m_locationsVectorData(VectorData::create(
          AQNWB::mergePaths(electrodeTablePath, "location"), io))
{
}

ElectrodesTable::ElectrodesTable(const std::string& path,
                                 std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodeTablePath, io)
    , m_electrodeDataset(ElementIdentifiers>::create(
          AQNWB::mergePaths(electrodesTablePath, "id"), io))  
    , m_groupNamesVectorData(VectorData::create(
          AQNWB::mergePaths(electrodeTablePath, "group_name"), io))
    , m_locationsVectorData(VectorData::create(
          AQNWB::mergePaths(electrodeTablePath, "location"), io))
{
  assert(path == this->electrodesTablePath && "ElectrodesTable object is required to appear at /general/extracellular_ephys/electrodes");
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
  Status electrodeStatus = m_rowElementIdentifiers->initialize(electrodeConfig);

  IO::ArrayDataSetConfig groupNameConfig(
      IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {1});
  Status groupNameStatus = m_groupNamesVectorData->initialize(
      groupNameConfig,
      "the name of the ElectrodeGroup this electrode is a part of");

  IO::ArrayDataSetConfig locationConfig(
      IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {1});
  Status locationStatus = m_locationsVectorData->initialize(
      locationConfig,
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
  Status status = Status::Success;
  // Check if new values have been added for the columns and update them
  // Updated electrode numbers
  if (m_electrodeNumbers.size() > 0) {
    Status rowIdStatus = setRowIDs(m_rowElementIdentifiers, m_electrodeNumbers);
    m_electrodeNumbers.clear();  // clear after writing
    status = status && rowIdStatus;
  }
  // Add the location names
  if (m_locationNames.size() > 0) {
    Status locationColStatus =
        addColumn(m_locationsVectorData, m_locationNames);
    m_locationNames.clear();  // clear after writing
    status = status && locationColStatus;
  }
  // Add the group references
  if (m_groupReferences.size() > 0) {
    // create the references to the ElectrodeGroup objects
    Status groupColStatus = addReferenceColumn(
        "group",
        "a reference to the ElectrodeGroup this electrode is a part of",
        m_groupReferences);
    status = status && groupColStatus;
    m_groupReferences.clear();  // clear after writing
  }
  // Add the group names
  if (m_groupNames.size() > 0) {
    Status groupNameColStatus = addColumn(m_groupNamesVectorData, m_groupNames);
    m_groupNames.clear();  // clear after writing
    status = status && groupNameColStatus;
  }
  // finalize the parent class to write the col names
  // This must be done after all columns have been added
  Status dtStatus = DynamicTable::finalize();
  status = status && dtStatus;

  return status;
}
