#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "nwb/NWBFile.hpp"

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Utils.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "spec/core.hpp"
#include "spec/hdmf_common.hpp"
#include "spec/hdmf_experimental.hpp"

using namespace AQNWB::NWB;

constexpr SizeType CHUNK_XSIZE =
    2048;  // TODO - replace these with io settings input
constexpr SizeType SPIKE_CHUNK_XSIZE =
    8;  // TODO - replace with io settings input

std::vector<SizeType> NWBFile::emptyContainerIndexes = {};

// NWBFile

NWBFile::NWBFile(const std::string& idText, std::shared_ptr<BaseIO> io)
    : Container("/", io)
    , m_identifierText(idText)

{
}

NWBFile::~NWBFile() {}

Status NWBFile::initialize(const std::string description,
                           const std::string dataCollection)
{
  if (std::filesystem::exists(m_io->getFileName())) {
    return m_io->open(false);
  } else {
    m_io->open(true);
    return createFileStructure(description, dataCollection);
  }
}

Status NWBFile::finalize()
{
  return m_io->close();
}

Status NWBFile::createFileStructure(std::string description,
                                    std::string dataCollection)
{
  if (!m_io->canModifyObjects()) {
    return Status::Failure;
  }

  m_io->createCommonNWBAttributes("/", "core", "NWBFile", "");
  m_io->createAttribute(AQNWB::SPEC::CORE::version, "/", "nwb_version");

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

  m_io->createGroup("/specifications");
  m_io->createReferenceAttribute("/specifications", "/", ".specloc");

  cacheSpecifications(
      "core", AQNWB::SPEC::CORE::version, AQNWB::SPEC::CORE::specVariables);
  cacheSpecifications("hdmf-common",
                      AQNWB::SPEC::HDMF_COMMON::version,
                      AQNWB::SPEC::HDMF_COMMON::specVariables);
  cacheSpecifications("hdmf-experimental",
                      AQNWB::SPEC::HDMF_EXPERIMENTAL::version,
                      AQNWB::SPEC::HDMF_EXPERIMENTAL::specVariables);

  std::string time = getCurrentTime();
  std::vector<std::string> timeVec = {time};
  m_io->createStringDataSet("/file_create_date", timeVec);
  m_io->createStringDataSet("/session_description", description);
  m_io->createStringDataSet("/session_start_time", time);
  m_io->createStringDataSet("/timestamps_reference_time", time);
  m_io->createStringDataSet("/identifier", m_identifierText);

  return Status::Success;
}

Status NWBFile::createElectricalSeries(
    std::vector<Types::ChannelVector> recordingArrays,
    std::vector<std::string> recordingNames,
    const BaseDataType& dataType,
    RecordingContainers* recordingContainers,
    std::vector<SizeType>& containerIndexes)
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
    m_electrodeTable = std::make_unique<ElectrodeTable>(m_io);
    m_electrodeTable->initialize();

    // Add electrode information to table (does not write to datasets yet)
    for (const auto& channelVector : recordingArrays) {
      m_electrodeTable->addElectrodes(channelVector);
    }
  }

  // Create datasets
  for (size_t i = 0; i < recordingArrays.size(); ++i) {
    const auto& channelVector = recordingArrays[i];
    const std::string& recordingName = recordingNames[i];

    // Setup electrodes and devices
    std::string groupName = channelVector[0].getGroupName();
    std::string devicePath = "/general/devices/" + groupName;
    std::string electrodePath = "/general/extracellular_ephys/" + groupName;
    std::string electricalSeriesPath = acquisitionPath + "/" + recordingName;

    // Check if device exists for groupName, create device and electrode group
    // if not
    if (!m_io->objectExists(devicePath)) {
      Device device = Device(devicePath, m_io, "description", "unknown");
      device.initialize();

      ElectrodeGroup elecGroup =
          ElectrodeGroup(electrodePath, m_io, "description", "unknown", device);
      elecGroup.initialize();
    }

    // Setup electrical series datasets
    auto electricalSeries = std::make_unique<ElectricalSeries>(
        electricalSeriesPath,
        m_io,
        dataType,
        channelVector,
        "Stores continuously sampled voltage data from an "
        "extracellular ephys recording",
        SizeArray {0, channelVector.size()},
        SizeArray {CHUNK_XSIZE, 0});
    electricalSeries->initialize();
    recordingContainers->addContainer(std::move(electricalSeries));
    containerIndexes.push_back(recordingContainers->size() - 1);
  }

  // write electrode information to datasets
  // (requires that the ElectrodeGroup has been written)
  if (!electrodeTableCreated) {
    m_electrodeTable->finalize();
  }

  return Status::Success;
}

Status NWBFile::createSpikeEventSeries(
    std::vector<Types::ChannelVector> recordingArrays,
    std::vector<std::string> recordingNames,
    const BaseDataType& dataType,
    RecordingContainers* recordingContainers,
    std::vector<SizeType>& containerIndexes)
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
    m_electrodeTable = std::make_unique<ElectrodeTable>(m_io);
    m_electrodeTable->initialize();

    // Add electrode information to table (does not write to datasets yet)
    for (const auto& channelVector : recordingArrays) {
      m_electrodeTable->addElectrodes(channelVector);
    }
  }

  // Create datasets
  for (size_t i = 0; i < recordingArrays.size(); ++i) {
    const auto& channelVector = recordingArrays[i];
    const std::string& recordingName = recordingNames[i];

    // Setup electrodes and devices
    std::string groupName = channelVector[0].getGroupName();
    std::string devicePath = "/general/devices/" + groupName;
    std::string electrodePath = "/general/extracellular_ephys/" + groupName;
    std::string spikeEventSeriesPath = acquisitionPath + "/" + recordingName;

    // Check if device exists for groupName, create device and electrode group
    // if not
    if (!m_io->objectExists(devicePath)) {
      Device device = Device(devicePath, m_io, "description", "unknown");
      device.initialize();

      ElectrodeGroup elecGroup =
          ElectrodeGroup(electrodePath, m_io, "description", "unknown", device);
      elecGroup.initialize();
    }

    // Setup Spike Event Series datasets
    SizeArray dsetSize;
    SizeArray chunkSize;
    if (channelVector.size() == 1) {
      dsetSize = SizeArray {0, 0};
      chunkSize = SizeArray {SPIKE_CHUNK_XSIZE, 1};
    } else {
      dsetSize = SizeArray {0, channelVector.size(), 0};
      chunkSize = SizeArray {SPIKE_CHUNK_XSIZE, 1, 1};
    }

    auto spikeEventSeries = std::make_unique<SpikeEventSeries>(
        spikeEventSeriesPath,
        m_io,
        dataType,
        channelVector,
        "Stores spike waveforms from an extracellular ephys recording",
        dsetSize,
        chunkSize);
    spikeEventSeries->initialize();
    recordingContainers->addContainer(std::move(spikeEventSeries));
    containerIndexes.push_back(recordingContainers->size() - 1);
  }

  // write electrode information to datasets
  // (requires that the ElectrodeGroup has been written)
  if (!electrodeTableCreated) {
    m_electrodeTable->finalize();
  }

  return Status::Success;
}

template<SizeType N>
void NWBFile::cacheSpecifications(
    const std::string& specPath,
    const std::string& versionNumber,
    const std::array<std::pair<std::string_view, std::string_view>, N>&
        specVariables)
{
  m_io->createGroup("/specifications/" + specPath);
  m_io->createGroup("/specifications/" + specPath + "/" + versionNumber);

  for (const auto& [name, content] : specVariables) {
    m_io->createStringDataSet("/specifications/" + specPath + "/"
                                  + versionNumber + "/" + std::string(name),
                              std::string(content));
  }
}

// recording data factory method /
std::unique_ptr<AQNWB::BaseRecordingData> NWBFile::createRecordingData(
    BaseDataType type,
    const SizeArray& size,
    const SizeArray& chunking,
    const std::string& path)
{
  return std::unique_ptr<BaseRecordingData>(
      m_io->createArrayDataSet(type, size, chunking, path));
}
