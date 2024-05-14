#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "NWBFile.hpp"

#include "BaseIO.hpp"
#include "Utils.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/hdmf/table/DynamicTableRegion.hpp"

using namespace AQNWB::NWB;

constexpr int CHUNK_XSIZE = 2048;

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

Status NWBFile::startRecording()
{
  // TODO - update these inputs below to be passed in from the acquisition
  // All of these values will be inputs from the acquisition system
  // with unknown size but self generating for now
  float samplingRate = 30000;
  std::vector<int> continuousArray;
  continuousArray.resize(1);
  for (std::size_t i = 0; i < continuousArray.size(); i++) {
    continuousArray[i] = i + 1;
  }
  std::vector<int> channels;
  std::vector<int> electrodeInds;
  std::vector<float> channelConversions;
  channels.resize(32);
  channelConversions.resize(32);
  electrodeInds.resize(32);
  for (std::size_t i = 0; i < channels.size(); i++) {
    channels[i] = i + 1;
    electrodeInds[i] = i;
    channelConversions[i] = 1e6;
  }

  // Create empty electrode table
  std::string electrodeTablePath = "general/extracellular_ephys/electrodes/";
  ElectrodeTable elecTable = ElectrodeTable(electrodeTablePath, io, channels);
  elecTable.initialize();

  // store all recorded data in the acquisition group
  std::string rootPath = "/acquisition/";

  // Create continuous datasets
  for (SizeType i = 0; i < continuousArray.size(); i++) {
    // Setup electrodes and devices
    std::string groupName = "array" + std::to_string(continuousArray[i]);
    std::string devicePath = "general/devices/" + groupName;
    std::string electrodePath = "general/extracellular_ephys/" + groupName;
    std::string electricalSeriesPath = rootPath + groupName;

    Device device = Device(devicePath, io, "description", "unknown");
    device.initialize();

    ElectrodeGroup elecGroup =
        ElectrodeGroup(electrodePath, io, "description", "unknown", device);
    elecGroup.initialize();
        
    // Setup electrical series datasets
    ElectricalSeries* electricalSeries = new ElectricalSeries(electricalSeriesPath, io,
        "Stores continuously sampled voltage data from an extracellular ephys recording",
        elecTable.getPath());
    electricalSeries->initialize();

    electricalSeries->data = createRecordingData(
        BaseDataType::I16, SizeArray {0}, SizeArray {CHUNK_XSIZE}, electricalSeries->getPath() + "/data");
    io->createDataAttributes(electricalSeries->getPath(), channelConversions[0], -1.0f, "volts");
    
    electricalSeries->timestamps = createRecordingData(
        BaseDataType::F64, SizeArray {0}, SizeArray {CHUNK_XSIZE}, electricalSeries->getPath() + "/timestamps");
    io->createTimestampsAttributes(electricalSeries->getPath(), 1/samplingRate);

    electricalSeries->channelConversion = createRecordingData(
        BaseDataType::F32, SizeArray {1}, SizeArray {CHUNK_XSIZE}, electricalSeries->getPath() + "/channel_conversion");
    io->createCommonNWBAttributes(electricalSeries->getPath() + "/channel_conversion", "hdmf-common", "", "Bit volts values for all channels");

    electricalSeries->electrodesDataset = createRecordingData(BaseDataType::I32, SizeArray {1}, SizeArray {CHUNK_XSIZE}, electricalSeries->getPath() + "/electrodes");
    electricalSeries->electrodesDataset->writeDataBlock(electrodeInds.size(), BaseDataType::I32, &electrodeInds[0]);
    io->createCommonNWBAttributes(electricalSeries->getPath() + "/electrodes", "hdmf-common", "DynamicTableRegion", "");
    io->createReferenceAttribute(elecTable.getPath(), electricalSeries->getPath() + "/electrodes", "table");
  }

  // Add electrode information to electrode table
  elecTable.electrodeDataset->dataset = createRecordingData(
      BaseDataType::I32, SizeArray {1}, SizeArray {1}, electrodeTablePath + "id");
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

  elecTable.addElectrodes();

  return Status::Success;
}

void NWBFile::stopRecording() {}

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

// NWBRecordingEngine
NWBRecordingEngine::NWBRecordingEngine()
{
  //  smpBuffer.malloc(MAX_BUFFER_SIZE); // TODO - JUCE type, need to adapt
}

NWBRecordingEngine::~NWBRecordingEngine()
{
  if (nwb != nullptr) {
    //  nwb->close();  // TODO - figure out which BaseIO / RecordingData things
    //  to handle nwb.reset();
  }
}

void NWBRecordingEngine::openFiles(const std::string& rootFolder,
                                   int experimentNumber,
                                   int recordingNumber)
{
}

void NWBRecordingEngine::closeFiles()
{
  nwb->stopRecording();
}
