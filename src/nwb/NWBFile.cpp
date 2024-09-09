#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "nwb/NWBFile.hpp"

#include "Channel.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "spec/core.hpp"
#include "spec/hdmf_common.hpp"
#include "spec/hdmf_experimental.hpp"

using namespace AQNWB::NWB;

constexpr SizeType CHUNK_XSIZE = 2048;

std::vector<SizeType> NWBFile::emptyContainerIndexes = {};

// NWBFile
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(NWBFile)

NWBFile::NWBFile(std::shared_ptr<IO::BaseIO> io)
    : Container("/", io)
{
}

NWBFile::NWBFile(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : Container("/", io)  // Always use "/" for the path
{
  std::cerr << "NWBFile object is always the root. Path must be /" << std::endl;
  assert(path == "/");
}

NWBFile::~NWBFile() {}

Status NWBFile::initialize(const std::string& identifierText)
{
  if (std::filesystem::exists(io->getFileName())) {
    return io->open(false);
  } else {
    io->open(true);
    return createFileStructure(identifierText);
  }
}

Status NWBFile::createFileStructure(const std::string& identifierText)
{
  if (!io->canModifyObjects()) {
    return Status::Failure;
  }

  io->createCommonNWBAttributes(
      this->path, this->getNamespace(), this->getTypeName(), "");
  io->createAttribute(AQNWB::SPEC::CORE::version, "/", "nwb_version");

  io->createGroup("/acquisition");
  io->createGroup("/analysis");
  io->createGroup("/processing");
  io->createGroup("/stimulus");
  io->createGroup("/stimulus/presentation");
  io->createGroup("/stimulus/templates");
  io->createGroup("/general");
  io->createGroup("/general/devices");
  io->createGroup("/general/extracellular_ephys");

  io->createGroup("/specifications");
  io->createReferenceAttribute("/specifications", "/", ".specloc");

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
  io->createStringDataSet("/file_create_date", timeVec);
  io->createStringDataSet("/session_description", "a recording session");
  io->createStringDataSet("/session_start_time", time);
  io->createStringDataSet("/timestamps_reference_time", time);
  io->createStringDataSet("/identifier", identifierText);

  return Status::Success;
}

Status NWBFile::finalize()
{
  return io->close();
}

Status NWBFile::createElectricalSeries(
    std::vector<Types::ChannelVector> recordingArrays,
    const IO::BaseDataType& dataType,
    RecordingContainers* recordingContainers,
    std::vector<SizeType>& containerIndexes)
{
  if (!io->canModifyObjects()) {
    return Status::Failure;
  }

  // store all recorded data in the acquisition group
  std::string rootPath = "/acquisition/";

  // Setup electrode table
  ElectrodeTable elecTable = ElectrodeTable(io);
  elecTable.initialize();

  // Create continuous datasets
  for (const auto& channelVector : recordingArrays) {
    // Setup electrodes and devices
    std::string groupName = channelVector[0].groupName;
    std::string devicePath = "/general/devices/" + groupName;
    std::string electrodePath = "/general/extracellular_ephys/" + groupName;
    std::string electricalSeriesPath = rootPath + groupName;

    Device device = Device(devicePath, io);
    device.initialize("description", "unknown");

    ElectrodeGroup elecGroup = ElectrodeGroup(electrodePath, io);
    elecGroup.initialize("description", "unknown", device);

    // Setup electrical series datasets
    auto electricalSeries =
        std::make_unique<ElectricalSeries>(electricalSeriesPath, io);
    electricalSeries->initialize(
        dataType,
        channelVector,
        "Stores continuously sampled voltage data from an "
        "extracellular ephys recording",
        SizeArray {0, channelVector.size()},
        SizeArray {CHUNK_XSIZE, 0});
    recordingContainers->addContainer(std::move(electricalSeries));
    containerIndexes.push_back(recordingContainers->containers.size() - 1);

    // Add electrode information to electrode table (does not write to datasets
    // yet)
    elecTable.addElectrodes(channelVector);
  }

  // write electrode information to datasets
  elecTable.finalize();

  return Status::Success;
}

template<SizeType N>
void NWBFile::cacheSpecifications(
    const std::string& specPath,
    const std::string& versionNumber,
    const std::array<std::pair<std::string_view, std::string_view>, N>&
        specVariables)
{
  io->createGroup("/specifications/" + specPath + "/");
  io->createGroup("/specifications/" + specPath + "/" + versionNumber);

  for (const auto& [name, content] : specVariables) {
    io->createStringDataSet("/specifications/" + specPath + "/" + versionNumber
                                + "/" + std::string(name),
                            std::string(content));
  }
}

// recording data factory method /
std::unique_ptr<AQNWB::IO::BaseRecordingData> NWBFile::createRecordingData(
    IO::BaseDataType type,
    const SizeArray& size,
    const SizeArray& chunking,
    const std::string& path)
{
  return std::unique_ptr<IO::BaseRecordingData>(
      io->createArrayDataSet(type, size, chunking, path));
}
