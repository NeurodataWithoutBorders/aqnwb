#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>

#include "nwb/NWBFile.hpp"

#include "Channel.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/misc/AnnotationSeries.hpp"
#include "spec/NamespaceRegistry.hpp"
#include "spec/core.hpp"
#include "spec/hdmf_common.hpp"
#include "spec/hdmf_experimental.hpp"

using namespace AQNWB::NWB;

constexpr SizeType CHUNK_XSIZE =
    2048;  // TODO - replace these with io settings input
constexpr SizeType SPIKE_CHUNK_XSIZE =
    8;  // TODO - replace with io settings input

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(NWBFile)

NWBFile::NWBFile(std::shared_ptr<IO::BaseIO> io)
    : Container("/", io)
{
}

NWBFile::NWBFile(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : Container("/", io)  // Always use "/" for the path
{
  assert(path == "/" && "NWBFile object is always the root. Path must be /");
}

NWBFile::~NWBFile() {}

Status NWBFile::initialize(const std::string& identifierText,
                           const std::string& description,
                           const std::string& dataCollection,
                           const std::string& sessionStartTime,
                           const std::string& timestampsReferenceTime)
{
  if (!m_io->isOpen()) {
    return Status::Failure;
  }
  // TODO: Call Container::initialize() instead. However, then we need to check in 
  //      Container that we don't create the root group and also remove the redundant call
  //      to create the common attributes. For no we can just call registerRecordingObject
  //      directly to to add this NWBFile object to RecordingObjects
  Status registerStatus = registerRecordingObject();  // Container::initialize();

  std::string currentTime = getCurrentTime();
  // use the current time if sessionStartTime is empty
  std::string useSessionStartTime =
      (!sessionStartTime.empty()) ? sessionStartTime : currentTime;
  // use the current time if timestampsReferenceTime is empty
  std::string useTimestampsReferenceTime = (!timestampsReferenceTime.empty())
      ? timestampsReferenceTime
      : currentTime;
  // check that sessionStartTime and timestampsReferenceTime are ISO8601
  if (!isISO8601Date(useSessionStartTime)) {
    std::cerr << "NWBFile::initialize sessionStartTime not in ISO8601 format: "
              << useSessionStartTime << std::endl;
    return Status::Failure;
  }
  if (!isISO8601Date(useTimestampsReferenceTime)) {
    std::cerr
        << "NWBFile::initialize timestampsReferenceTime not in ISO8601 format: "
        << useTimestampsReferenceTime << std::endl;
    return Status::Failure;
  }

  // Check that the file is empty and initialize if it is
  bool fileInitialized = isInitialized();
  if (!fileInitialized) {
    Status createStatus = createFileStructure(identifierText,
                                              description,
                                              dataCollection,
                                              useSessionStartTime,
                                              useTimestampsReferenceTime);
    return (createStatus == Status::Success && registerStatus == Status::Success) 
           ? Status::Success : Status::Failure;
  } else {
    return registerStatus;  // File is already initialized, return register status
  }
}

bool NWBFile::isInitialized() const
{
  std::vector<std::pair<std::string, StorageObjectType>> existingGroupObjects =
      m_io->getStorageObjects("/", StorageObjectType::Group);
  if (existingGroupObjects.size() == 0) {
    return false;
  }
  // Define the set of required objects
  static const std::unordered_set<std::string> requiredObjects = {
      "acquisition",
      "analysis",
      "processing",
      "stimulus",
      "general",
      "specifications"};

  // Set to keep track of found objects
  std::unordered_set<std::string> foundObjects;

  // Iterate over the existing objects and add to foundObjects if it's a
  // required object
  for (const auto& obj : existingGroupObjects) {
    if (requiredObjects.find(obj.first) != requiredObjects.end()) {
      foundObjects.insert(obj.first);
    }
  }

  // Check if all required objects are found
  return (foundObjects.size() == requiredObjects.size());
}

Status NWBFile::createFileStructure(const std::string& identifierText,
                                    const std::string& description,
                                    const std::string& dataCollection,
                                    const std::string& sessionStartTime,
                                    const std::string& timestampsReferenceTime)
{
  if (!m_io->canModifyObjects()) {
    return Status::Failure;
  }

  // Create the namespace, neurodata_type, and nwb_version attributes
  m_io->createCommonNWBAttributes(
      m_path, this->getNamespace(), this->getTypeName());
  m_io->createAttribute(AQNWB::SPEC::CORE::version, "/", "nwb_version");

  // Create the top-level group structure of the NWB file
  m_io->createGroup("/acquisition");
  m_io->createGroup("/analysis");
  m_io->createGroup("/processing");
  m_io->createGroup("/stimulus");
  m_io->createGroup("/stimulus/presentation");
  m_io->createGroup("/stimulus/templates");
  m_io->createGroup("/general");
  m_io->createGroup("/general/devices");
  m_io->createGroup("/general/extracellular_ephys");
  if (dataCollection != "") {
    m_io->createStringDataSet("/general/data_collection", dataCollection);
  }

  // Setupe the specifications cache in the file
  m_io->createGroup(m_specificationsPath);
  m_io->createReferenceAttribute(m_specificationsPath, "/", ".specloc");
  // Cache all namespaces registered with the namespace registry
  const auto& allNamespaces =
      AQNWB::SPEC::NamespaceRegistry::instance().getAllNamespaces();
  for (const auto& [name, info] : allNamespaces) {
    cacheSpecifications(info);
  }

  // Create additional required datasets
  std::vector<std::string> timeVec = {sessionStartTime};
  m_io->createStringDataSet("/file_create_date", timeVec);
  m_io->createStringDataSet("/session_description", description);
  m_io->createStringDataSet("/session_start_time", sessionStartTime);
  m_io->createStringDataSet("/timestamps_reference_time",
                            timestampsReferenceTime);
  m_io->createStringDataSet("/identifier", identifierText);
  return Status::Success;
}

Status NWBFile::createElectrodesTable(
    std::vector<Types::ChannelVector> recordingArrays)
{
  auto electrodeTable = NWB::ElectrodeTable::create(m_io);
  electrodeTable->initialize();
  for (const auto& channelVector : recordingArrays) {
    electrodeTable->addElectrodes(channelVector);
  }

  for (size_t i = 0; i < recordingArrays.size(); ++i) {
    const auto& channelVector = recordingArrays[i];

    // Setup electrodes and devices
    std::string groupName = channelVector[0].getGroupName();
    std::string devicePath = AQNWB::mergePaths("/general/devices", groupName);
    std::string electrodePath =
        AQNWB::mergePaths("/general/extracellular_ephys", groupName);

    // Check if device exists for groupName, create device and electrode group
    // if it does not
    if (!m_io->objectExists(devicePath)) {
      auto device = NWB::Device::create(devicePath, m_io);
      device->initialize("description", "unknown");

      auto elecGroup = NWB::ElectrodeGroup::create(electrodePath, m_io);
      elecGroup->initialize("description", "unknown", device);
    }
  }

  // write electrodes information to datasets
  // (requires that ElectrodeGroup data is initialized)
  electrodeTable->finalize();

  return Status::Success;
}

Status NWBFile::createElectricalSeries(
    std::vector<Types::ChannelVector> recordingArrays,
    std::vector<std::string> recordingNames,
    const IO::BaseDataType& dataType)
{
  if (!m_io->canModifyObjects()) {
    return Status::Failure;
  }

  if (recordingNames.size() != recordingArrays.size()) {
    return Status::Failure;
  }

  // Setup electrode table if it was not yet created
  bool electrodeTableCreated =
      m_io->objectExists(ElectrodeTable::electrodeTablePath);
  if (!electrodeTableCreated) {
    std::cerr << "NWBFile::createElectricalSeries requires an electrodes table "
                 "to be present"
              << std::endl;
    return Status::Failure;
  }

  // Create datasets
  for (size_t i = 0; i < recordingArrays.size(); ++i) {
    const auto& channelVector = recordingArrays[i];
    const std::string& recordingName = recordingNames[i];
    std::string electricalSeriesPath =
        AQNWB::mergePaths(m_acquisitionPath, recordingName);

    // Setup electrical series datasets
    IO::ArrayDataSetConfig config(dataType,
                                  SizeArray {0, channelVector.size()},
                                  SizeArray {CHUNK_XSIZE, 0});
    auto electricalSeries = ElectricalSeries::create(electricalSeriesPath, m_io);
    Status esStatus = electricalSeries->initialize(
        config,
        channelVector,
        "Stores continuously sampled voltage data from an "
        "extracellular ephys recording");
    if (esStatus != Status::Success) {
      return esStatus;
    }
  }

  return Status::Success;
}

Status NWBFile::createSpikeEventSeries(
    std::vector<Types::ChannelVector> recordingArrays,
    std::vector<std::string> recordingNames,
    const IO::BaseDataType& dataType)
{
  if (!m_io->canModifyObjects()) {
    return Status::Failure;
  }

  if (recordingNames.size() != recordingArrays.size()) {
    return Status::Failure;
  }

  // Setup electrode table if it was not yet created
  bool electrodeTableCreated =
      m_io->objectExists(ElectrodeTable::electrodeTablePath);
  if (!electrodeTableCreated) {
    std::cerr << "NWBFile::createElectricalSeries requires an electrodes table "
                 "to be present"
              << std::endl;
    return Status::Failure;
  }

  // Create datasets
  for (size_t i = 0; i < recordingArrays.size(); ++i) {
    const auto& channelVector = recordingArrays[i];
    const std::string& recordingName = recordingNames[i];

    // Setup electrodes and devices
    std::string groupName = channelVector[0].getGroupName();
    std::string devicePath = AQNWB::mergePaths("/general/devices", groupName);
    std::string electrodePath =
        AQNWB::mergePaths("/general/extracellular_ephys", groupName);
    std::string spikeEventSeriesPath =
        AQNWB::mergePaths(m_acquisitionPath, recordingName);

    // Check if device exists for groupName, create device and electrode group
    // if not
    if (!m_io->objectExists(devicePath)) {
      auto device = Device::create(devicePath, m_io);
      device->initialize("description", "unknown");

      auto elecGroup = ElectrodeGroup::create(electrodePath, m_io);
      elecGroup->initialize("description", "unknown", device);
    }

    // Setup Spike Event Series datasets
    IO::ArrayDataSetConfig config(
        dataType,
        channelVector.size() == 1 ? SizeArray {0, 0}
                                  : SizeArray {0, channelVector.size(), 0},
        channelVector.size() == 1 ? SizeArray {SPIKE_CHUNK_XSIZE, 1}
                                  : SizeArray {SPIKE_CHUNK_XSIZE, 1, 1});

    auto spikeEventSeries = SpikeEventSeries::create(spikeEventSeriesPath, m_io);
    spikeEventSeries->initialize(
        config,
        channelVector,
        "Stores spike waveforms from an extracellular ephys recording");
  }

  return Status::Success;
}

Status NWBFile::createAnnotationSeries(std::vector<std::string> recordingNames)
{
  if (!m_io->canModifyObjects()) {
    return Status::Failure;
  }

  for (size_t i = 0; i < recordingNames.size(); ++i) {
    const std::string& recordingName = recordingNames[i];

    std::string annotationSeriesPath =
        AQNWB::mergePaths(m_acquisitionPath, recordingName);

    // Setup annotation series datasets
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {CHUNK_XSIZE});
    auto annotationSeries = AnnotationSeries::create(annotationSeriesPath, m_io);
    annotationSeries->initialize(
        "Stores user annotations made during an experiment",
        "no comments",
        config);
  }

  return Status::Success;
}

void NWBFile::cacheSpecifications(const Types::NamespaceInfo& namespaceInfo)
{
  std::string specFullPath =
      AQNWB::mergePaths(m_specificationsPath, namespaceInfo.name);
  std::string specFullVersionPath =
      AQNWB::mergePaths(specFullPath, namespaceInfo.version);
  m_io->createGroup(specFullPath);
  m_io->createGroup(specFullVersionPath);

  for (const auto& [name, content] : namespaceInfo.specVariables) {
    m_io->createStringDataSet(
        AQNWB::mergePaths(specFullVersionPath, std::string(name)),
        std::string(content));
  }
}
