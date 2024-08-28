#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "NWBFile.hpp"

#include "../BaseIO.hpp"
#include "../Channel.hpp"
#include "../Utils.hpp"
#include "../spec/core.hpp"
#include "../spec/hdmf_common.hpp"
#include "../spec/hdmf_experimental.hpp"
#include "device/Device.hpp"
#include "ecephys/ElectricalSeries.hpp"
#include "file/ElectrodeGroup.hpp"
#include "file/ElectrodeTable.hpp"

using namespace AQNWB::NWB;

constexpr SizeType CHUNK_XSIZE = 2048;

// NWBFile

NWBFile::NWBFile(const std::string& idText, std::shared_ptr<BaseIO> io)
    : identifierText(idText)
    , io(io)
{
}

NWBFile::~NWBFile() {}

Status NWBFile::initialize()
{
  if (std::filesystem::exists(io->getFileName())) {
    return io->open(false);
  } else {
    io->open(true);
    return createFileStructure();
  }
}

Status NWBFile::finalize()
{
  recordingContainers.reset();
  return io->close();
}

Status NWBFile::createFileStructure()
{
  if (!io->canModifyObjects()) {
    return Status::Failure;
  }

  io->createCommonNWBAttributes("/", "core", "NWBFile", "");
  io->createAttribute(AQNWB::spec::core::version, "/", "nwb_version");

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
  cacheSpecifications("core", spec::core::version, spec::core::registerVariables);
  cacheSpecifications("hdmf-common", spec::hdmf_common::version, spec::hdmf_common::registerVariables);
  cacheSpecifications("hdmf-experimental", spec::hdmf_experimental::version, spec::hdmf_experimental::registerVariables);

  std::string time = getCurrentTime();
  std::vector<std::string> timeVec = {time};
  io->createStringDataSet("/file_create_date", timeVec);
  io->createStringDataSet("/session_description", "a recording session");
  io->createStringDataSet("/session_start_time", time);
  io->createStringDataSet("/timestamps_reference_time", time);
  io->createStringDataSet("/identifier", identifierText);

  return Status::Success;
}

Status NWBFile::createElectricalSeries(
    std::vector<Types::ChannelVector> recordingArrays,
    const BaseDataType& dataType)
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

    Device device = Device(devicePath, io, "description", "unknown");
    device.initialize();

    ElectrodeGroup elecGroup =
        ElectrodeGroup(electrodePath, io, "description", "unknown", device);
    elecGroup.initialize();

    // Setup electrical series datasets
    auto electricalSeries = std::make_unique<ElectricalSeries>(
        electricalSeriesPath,
        io,
        dataType,
        channelVector,
        "Stores continuously sampled voltage data from an "
        "extracellular ephys recording",
        SizeArray {0, channelVector.size()},
        SizeArray {CHUNK_XSIZE, 0});
    electricalSeries->initialize();
    recordingContainers->addData(std::move(electricalSeries));

    // Add electrode information to electrode table (does not write to datasets
    // yet)
    elecTable.addElectrodes(channelVector);
  }

  // write electrode information to datasets
  elecTable.finalize();

  return Status::Success;
}

Status NWBFile::startRecording()
{
  return io->startRecording();
}

void NWBFile::stopRecording()
{
  io->stopRecording();
}

void NWBFile::cacheSpecifications(const std::string& specPath, 
                                  const std::string& version,
                                  void (*registerFunc)(std::map<std::string, const std::string*>&))
{
  std::map<std::string, const std::string*> registry;
  registerFunc(registry);

  io->createGroup("/specifications/" + specPath + "/");
  io->createGroup("/specifications/" + specPath + "/" + version);

  for (const auto& [name, content] : registry) {
      io->createStringDataSet("/specifications/" + specPath + "/" + version + "/" + name, *content);
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
      io->createArrayDataSet(type, size, chunking, path));
}

TimeSeries* NWBFile::getTimeSeries(const SizeType& timeseriesInd)
{
  if (timeseriesInd >= this->recordingContainers->containers.size()) {
    return nullptr;
  } else {
    return this->recordingContainers->containers[timeseriesInd].get();
  }
}

// Recording Container

RecordingContainers::RecordingContainers(const std::string& name)
    : name(name)
{
}

RecordingContainers::~RecordingContainers() {}

void RecordingContainers::addData(std::unique_ptr<TimeSeries> data)
{
  this->containers.push_back(std::move(data));
}
