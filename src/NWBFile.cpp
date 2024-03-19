#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "NWBFile.hpp"

#include "io/BaseIO.hpp"
#include "file/ElectrodeGroup.hpp"
#include "file/ElectrodeTable.hpp"
#include "device/Device.hpp"

using namespace AQNWBIO;
namespace fs = std::filesystem;

constexpr size_t MAX_BUFFER_SIZE = 40960;

// NWBFile

NWBFile::NWBFile(std::string idText, std::shared_ptr<BaseIO> io)
    : identifierText(idText)
    , io(io)
{
  bufferSize = MAX_BUFFER_SIZE;
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

int NWBFile::createFileStructure()
{
  io->setAttribute("core", "/", "namespace");
  io->setAttribute("NWBFile", "/", "neurodata_type");
  io->setAttribute(NWBVersion, "/", "nwb_version");
  io->setAttribute(identifierText, "/", "object_id");

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
  cacheSpecifications("core/", NWBVersion);
  cacheSpecifications("hdmf-common/", HDMFVersion);

  std::string time = getCurrentTime();
  io->createStringDataSet("/file_create_date", time);  // TODO - change to array
  io->createStringDataSet("/session_description", "a recording session");
  io->createStringDataSet("/session_start_time", time);
  io->createStringDataSet("/timestamps_reference_time", time);
  io->createStringDataSet("/identifier", "test-identifier");

  return 0;
}

bool NWBFile::startRecording()
{
  // Later channels/continuous array will be input from the acquisition system
  // but self generating for now
  std::vector<int> continuousArray;
  for (int i = 1; i <= 1; ++i) {
    continuousArray.push_back(i);
  }
  std::vector<int> channels;
  for (int i = 1; i <= 32; ++i) {
    channels.push_back(i);
  }

  // store all recorded data in the acquisition group
  std::string rootPath = "/acquisition/";

  // 1. Create continuous datasets
  for (size_t i = 0; i < continuousArray.size(); i++) {
    // Setup electrodes and devices
    std::string groupName = "array" + std::to_string(continuousArray[i]);
    std::string devicePath = "general/devices/" + groupName;
    std::string elecPath = "general/extracellular_ephys/" + groupName;

    ElectrodeGroup elecGroup = ElectrodeGroup(elecPath, io);
    elecGroup.device = devicePath;
    elecGroup.initialize();

    Device device = Device(devicePath, io);
    device.initialize();
    elecGroup.linkDevice();
  }

  // Create electrode table
  std::string electrodePath = "general/extracellular_ephys/electrodes/";
  std::unique_ptr<ElectrodeTable> elecTable =
      std::make_unique<ElectrodeTable>(electrodePath, io);

  elecTable->channels = channels;
  elecTable->electrodeDataset->dataset =
      createRecordingData(BaseDataType::I32, 1, 1, electrodePath + "id");
  elecTable->groupNamesDataset->dataset = createRecordingData(
      BaseDataType::STR(250), 0, 1, electrodePath + "group_name");
  elecTable->locationsDataset->dataset = createRecordingData(
      BaseDataType::STR(250), 0, 1, electrodePath + "location");

  elecTable->initialize();

  return true;
}

void NWBFile::stopRecording() {}

void NWBFile::writeData(
    int datasetID, int channel, int nSamples, const float* data, float bitVolts)
{
  // TODO - some things here
}

void NWBFile::cacheSpecifications(std::string specPath,
                                  std::string versionNumber)
{
  io->createGroup("/specifications/" + specPath);
  io->createGroup("/specifications/" + specPath + versionNumber);

  fs::path currentFile = __FILE__;
  fs::path schemaDir = currentFile.parent_path().parent_path()
      / "resources/spec" / specPath / versionNumber;

  for (auto const& entry : fs::directory_iterator {schemaDir})
    if (fs::is_regular_file(entry) && entry.path().extension() == ".json") {
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

std::string NWBFile::getCurrentTime()
{
  // Get current time
  auto currentTime =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  // Convert to tm struct to extract date and time components
  std::tm utcTime = *std::gmtime(&currentTime);

  // Format the date and time in ISO 8601 format with the UTC offset
  std::ostringstream oss;
  oss << std::put_time(&utcTime, "%FT%T%z");

  return oss.str();
}

// recording data factory method /
std::unique_ptr<BaseRecordingData> NWBFile::createRecordingData(
    BaseDataType type, int sizeX, int chunkX, const std::string& path)
{
  return std::unique_ptr<BaseRecordingData>(
      io->createDataSet(type, sizeX, chunkX, path));
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

void NWBRecordingEngine::openFiles(std::string rootFolder,
                                   int experimentNumber,
                                   int recordingNumber)
{
}

void NWBRecordingEngine::closeFiles()
{
  nwb->stopRecording();
}
