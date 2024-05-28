#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "NWBFile.hpp"

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Utils.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodeTable.hpp"

using namespace AQNWB::NWB;

constexpr SizeType CHUNK_XSIZE = 2048;

// NWBFile

NWBFile::NWBFile(const std::string& idText, std::shared_ptr<BaseIO> io)
    : identifierText(idText)
    , io(io)
{
}

NWBFile::~NWBFile() {}

void NWBFile::initialize()
{
  if (std::filesystem::exists(io->getFileName())) {
    io->open(false);
  } else {
    io->open(true);
    createFileStructure();
  }
}

void NWBFile::finalize()
{
  io->close();
}

Status NWBFile::createFileStructure()
{
  io->createCommonNWBAttributes("/", "core", "NWBFile", "");
  io->createAttribute(NWBVersion, "/", "nwb_version");

  io->createGroup("/acquisition");
  io->createGroup("/analysis");
  io->createGroup("/processing");
  io->createGroup("/stimulus");
  io->createGroup("/stimulus/presentation");
  io->createGroup("/stimulus/templates");
  io->createGroup("/general");
  io->createGroup("general/devices");
  io->createGroup("general/extracellular_ephys");

  io->createGroup("/specifications");
  io->createReferenceAttribute("/specifications", "/", ".specloc");
  cacheSpecifications("core/", NWBVersion);
  cacheSpecifications("hdmf-common/", HDMFVersion);
  cacheSpecifications("hdmf-experimental/", HDMFExperimentalVersion);

  std::string time = getCurrentTime();
  std::vector<std::string> timeVec = {time};
  io->createStringDataSet("/file_create_date", timeVec);
  io->createStringDataSet("/session_description", "a recording session");
  io->createStringDataSet("/session_start_time", time);
  io->createStringDataSet("/timestamps_reference_time", time);
  io->createStringDataSet("/identifier", identifierText);

  return Status::Success;
}

Status NWBFile::startRecording(std::vector<Types::ChannelGroup> recordingArrays)
{
  // store all recorded data in the acquisition group
  std::string rootPath = "/acquisition/";

  timeseriesData.clear();
  timeseriesData.reserve(recordingArrays.size());

  // Setup electrode table
  std::string electrodeTablePath = "general/extracellular_ephys/electrodes/";
  ElectrodeTable elecTable = ElectrodeTable(electrodeTablePath, io);
  elecTable.initialize();
  elecTable.electrodeDataset->dataset =
      createRecordingData(BaseDataType::I32,
                          SizeArray {1},
                          SizeArray {1},
                          electrodeTablePath + "id");
  elecTable.groupNamesDataset->dataset =
      createRecordingData(BaseDataType::STR(250),
                          SizeArray {0},
                          SizeArray {1},
                          electrodeTablePath + "group_name");
  elecTable.locationsDataset->dataset =
      createRecordingData(BaseDataType::STR(250),
                          SizeArray {0},
                          SizeArray {1},
                          electrodeTablePath + "location");

  // Create continuous datasets
  for (const auto& channelGroup : recordingArrays) {
    // Setup electrodes and devices
    std::string groupName = channelGroup[0].groupName;
    std::string devicePath = "general/devices/" + groupName;
    std::string electrodePath = "general/extracellular_ephys/" + groupName;
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
        "Stores continuously sampled voltage data from an "
        "extracellular ephys recording",
        channelGroup, 
        CHUNK_XSIZE,
        elecTable.getPath());
    electricalSeries->initialize();
    timeseriesData.push_back(std::move(electricalSeries));

    // Add electrode information to electrode table (does not write to datasets
    // yet)
    elecTable.addElectrodes(channelGroup);
  }

  // write electrode information to datasets
  elecTable.finalize();

  return Status::Success;
}

void NWBFile::stopRecording() {}

Status NWBFile::writeTimeseriesTimestamps(SizeType datasetInd,
                                          SizeType numSamples,
                                          BaseDataType type,
                                          const void* data)
{
  if (!timeseriesData[datasetInd])
    return Status::Failure;

  return timeseriesData[datasetInd]->timestamps->writeDataBlock(
      numSamples, type, data);
}

Status NWBFile::writeTimeseriesData(SizeType datasetInd,
                                    SizeType rowInd,
                                    SizeType numSamples,
                                    BaseDataType type,
                                    const void* data)
{
  if (!timeseriesData[datasetInd])
    return Status::Failure;

  return timeseriesData[datasetInd]->data->writeDataRow(
      numSamples, rowInd, type, data);
}

void NWBFile::cacheSpecifications(const std::string& specPath,
                                  const std::string& versionNumber)
{
  io->createGroup("/specifications/" + specPath);
  io->createGroup("/specifications/" + specPath + versionNumber);

  std::filesystem::path currentFile = __FILE__;
  std::filesystem::path schemaDir =
      currentFile.parent_path().parent_path().parent_path() / "resources/spec"
      / specPath / versionNumber;

  for (auto const& entry : std::filesystem::directory_iterator {schemaDir})
    if (std::filesystem::is_regular_file(entry)
        && entry.path().extension() == ".json")
    {
      std::string specName =
          entry.path().filename().replace_extension("").string();
      if (specName.find("namespace") != std::string::npos)
        specName = "namespace";

      std::ifstream schemaFile(entry.path());
      std::stringstream buffer;
      buffer << schemaFile.rdbuf();

      io->createStringDataSet(
          "/specifications/" + specPath + versionNumber + "/" + specName,
          buffer.str());
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
      io->createDataSet(type, size, chunking, path));
}
