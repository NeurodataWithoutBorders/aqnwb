#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include "NWBFile.hpp"
#include "NWBDataTypes.hpp"
#include "BaseIO.hpp"

using namespace AQNWBIO;
namespace fs = std::filesystem;

constexpr size_t MAX_BUFFER_SIZE = 40960;

// NWBFile

NWBFile::NWBFile(std::string idText, std::shared_ptr<BaseIO> io)
    : identifierText(idText)
    , io(io)
{
  //   scaledBuffer.malloc(MAX_BUFFER_SIZE);  TODO - JUCE types, need to adapt
  //   for std library intBuffer.malloc(MAX_BUFFER_SIZE);
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
  // store all recorded data in the acquisition group
  std::string rootPath = "/acquisition/";
  std::string electrodePath = "general/extracellular_ephys/electrodes/";

  // Later this will be inputs but self generate for now
  std::vector<int> continuousArray;
  for (int i = 1; i <= 32; ++i) {
    continuousArray.push_back(i);
  }  
  // 1. Create continuous datasets
  for (size_t i = 0; i < continuousArray.size(); i++)
  {
    std::string groupName = "electrode" + std::to_string(continuousArray[i]);
    std::string fullPath = "general/extracellular_ephys/" + groupName;
    io->createGroup(fullPath);
    io->setGroupAttributes(fullPath, "core", "ElectrodeGroup", "description");
    io->setAttribute("unknown", fullPath, "location");

    std::string devicePath = "general/devices/" + groupName;
    Device* device = new Device(devicePath, io);
    io->createLink("/" + fullPath + "/device", devicePath);
  }

  // Create electrode table
  std::vector<int> channels;    // NOTE - Later channels/continuous array will be inputs but self generate for now
  for (int i = 1; i <= 32; ++i) {
    channels.push_back(i);
  }

  ElectrodeTable* elecTable = new ElectrodeTable(electrodePath, io);
  elecTable->initialize(channels);

  elecTable->electrodeDataset = createRecordingData(BaseDataType::I32, 1, 1, "general/extracellular_ephys/electrodes/id");
  checkError(elecTable->electrodeDataset->writeDataBlock(static_cast<int>(elecTable->electrodeNumbers.size()), BaseDataType::I32, &elecTable->electrodeNumbers[0]));
  elecTable->groupNamesDataset = createRecordingData(BaseDataType::STR(250), 0, 1, electrodePath + "group_name");
  elecTable->locationsDataset = createRecordingData(BaseDataType::STR(250), 0, 1, electrodePath + "location");

  for (size_t i = 0; i < elecTable->groupNames.size(); i++)
    elecTable->groupNamesDataset->writeDataBlock(1, BaseDataType::STR(elecTable->groupNames[i].size()), &elecTable->groupNames[i]);

  for (size_t i = 0; i < elecTable->groupNames.size(); i++)
    elecTable->locationsDataset->writeDataBlock(1, BaseDataType::STR(7), "unknown");

  io->createReferenceDataSet("general/extracellular_ephys/electrodes/group", elecTable->groupReferences);  // TODO - leaving off erroring here
  
  elecTable->addIdentifiers("id", "a reference to the ElectrodeGroup this electrode is a part of");
  elecTable->addColumn("group_name", "the name of the ElectrodeGroup this electrode is a part of");
  elecTable->addColumn("location", "the location of channel within the subject e.g. brain region");
  elecTable->addColumn("group", "a reference to the ElectrodeGroup this electrode is a part of");

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
std::unique_ptr<BaseRecordingData> NWBFile::createRecordingData(BaseDataType type, int sizeX, int chunkX, const std::string& path) {  
    return std::unique_ptr<BaseRecordingData>(io->createDataSet(type, sizeX, chunkX, path));
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