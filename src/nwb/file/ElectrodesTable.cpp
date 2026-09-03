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
{
}

ElectrodesTable::ElectrodesTable(const std::string& path,
                                 std::shared_ptr<IO::BaseIO> io)
    : DynamicTable(electrodesTablePath, io)
{
  if (path != this->electrodesTablePath) {
    std::cerr << "WARNING: ElectrodesTable object is required to appear at "
              << this->electrodesTablePath << ". Ignoring provided path."
              << std::endl;
  }
}

/** Destructor */
ElectrodesTable::~ElectrodesTable() {}

std::vector<DynamicTable::DataSpecPtr> ElectrodesTable::createDefaultDataSpecs(
    const SizeType rowChunkSize)
{
  std::vector<DataSpecPtr> specs =
      DynamicTable::createDefaultDataSpecs(rowChunkSize);

  IO::ArrayDataSetConfig locationConfig(
      IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {rowChunkSize});
  specs.push_back(std::make_shared<VectorData::DataSpec>(
      "location",
      locationConfig,
      "the location of channel within the subject e.g. brain region"));

  IO::ArrayDataSetConfig groupNameConfig(
      IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {rowChunkSize});
  specs.push_back(std::make_shared<VectorData::DataSpec>(
      "group_name",
      groupNameConfig,
      "the name of the ElectrodeGroup this electrode is a part of"));

  // Note: "group" is a reference column, which is currently added dynamically
  // in finalize() We don't add it to the specs here because reference columns
  // are not yet supported in DataSpec. Also, reference columns can currently
  // not be configured with chunking to support resize/append.
  // TODO: Add support for reference columns in DataSpec and configure them
  // here.
  // TODO: Add support for creating reference columns with chunking to support
  // resize/append.

  return specs;
}

Status ElectrodesTable::validateDataSpecs(
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  Status status = DynamicTable::validateDataSpecs(dataSpecs);
  if (status != Status::Success) {
    return status;
  }

  status = checkRequiredColumnSpec<VectorData::DataSpec>(
      "location", dataSpecs, IO::BaseDataType(IO::BaseDataType::V_STR));
  if (status != Status::Success) {
    return status;
  }

  return checkRequiredColumnSpec<VectorData::DataSpec>(
      "group_name", dataSpecs, IO::BaseDataType(IO::BaseDataType::V_STR));
}

/** Initialization function*/
Status ElectrodesTable::initialize(const std::string& description,
                                   const std::vector<DataSpecPtr>& columnSpecs)
{
  std::vector<DataSpecPtr> specsToUse = columnSpecs;
  if (specsToUse.empty()) {
    specsToUse = createDefaultDataSpecs();
  }

  // create group. This configures the "location" and "group_name" columns
  // (among others) via the DataSpec mechanism, creating and registering the
  // corresponding VectorData recording objects exactly once.
  Status dtStatus = DynamicTable::initialize(description, specsToUse);

  return dtStatus;
}

void ElectrodesTable::addElectrodes(const std::vector<Channel>& channelsInput)
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
    Status rowIdStatus = setRowIDs(m_electrodeNumbers);
    m_electrodeNumbers.clear();  // clear after writing
    status = status && rowIdStatus;
  }
  // Add the location names
  if (m_locationNames.size() > 0) {
    auto locationColumn = getConfiguredColumn("location");
    if (!locationColumn) {
      std::cerr << "ElectrodesTable::finalize failed to get location column."
                << std::endl;
      status = Status::Failure;
    } else {
      // Write all strings in a single block
      auto dataset = locationColumn->recordData();
      Status writeStatus =
          dataset->writeDataBlock(SizeArray {m_locationNames.size()},
                                  SizeArray {0},
                                  IO::BaseDataType::V_STR,
                                  m_locationNames);

      m_locationNames.clear();  // clear after writing
      status = status && writeStatus;
    }
  }
  // Add the group names
  if (m_groupNames.size() > 0) {
    auto groupNameColumn = getConfiguredColumn("group_name");
    if (!groupNameColumn) {
      std::cerr << "ElectrodesTable::finalize failed to get group_name column."
                << std::endl;
      status = Status::Failure;
    } else {
      // Write all strings in a single block
      auto dataset = groupNameColumn->recordData();
      Status writeStatus =
          dataset->writeDataBlock(SizeArray {m_groupNames.size()},
                                  SizeArray {0},
                                  IO::BaseDataType::V_STR,
                                  m_groupNames);
      m_groupNames.clear();  // clear after writing
      status = status && writeStatus;
    }
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
  // finalize the parent class
  // This must be done after all columns have been added
  Status dtStatus = DynamicTable::finalize();
  status = status && dtStatus;

  return status;
}
