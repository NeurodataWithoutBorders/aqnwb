#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "NWBFile.hpp"

#include "Utils.hpp"
#include "device/Device.hpp"
#include "file/ElectrodeGroup.hpp"
#include "file/ElectrodeTable.hpp"
#include "io/BaseIO.hpp"

using namespace AQNWBIO;
namespace fs = std::filesystem;

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
  io->createAttribute("core", "/", "namespace");
  io->createAttribute("NWBFile", "/", "neurodata_type");
  io->createAttribute(NWBVersion, "/", "nwb_version");
  io->createAttribute(identifierText, "/", "object_id");

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

  return Status::Success;
}

Status NWBFile::startRecording()
{
  // Later channels/continuous array will be input from the acquisition system
  // with unknown size but self generating for now
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
  for (SizeType i = 0; i < continuousArray.size(); i++) {
    // Setup electrodes and devices
    std::string groupName = "array" + std::to_string(continuousArray[i]);
    std::string devicePath = "general/devices/" + groupName;
    std::string elecPath = "general/extracellular_ephys/" + groupName;

    Device device = Device(devicePath, io, "description", "unknown");
    device.initialize();

    ElectrodeGroup elecGroup =
        ElectrodeGroup(elecPath, io, "description", "unknown", device);
    elecGroup.initialize();
  }

  // Create electrode table
  std::string electrodePath = "general/extracellular_ephys/electrodes/";
  ElectrodeTable elecTable = ElectrodeTable(electrodePath, io, channels);

  elecTable.electrodeDataset->dataset = createRecordingData(
      BaseDataType::I32, SizeArray {1}, SizeArray {1}, electrodePath + "id");
  elecTable.groupNamesDataset->dataset =
      createRecordingData(BaseDataType::STR(250),
                          SizeArray {0},
                          SizeArray {1},
                          electrodePath + "group_name");
  elecTable.locationsDataset->dataset =
      createRecordingData(BaseDataType::STR(250),
                          SizeArray {0},
                          SizeArray {1},
                          electrodePath + "location");

  elecTable.initialize();

  return Status::Success;
}

void NWBFile::stopRecording() {}

void NWBFile::cacheSpecifications(const std::string& specPath,
                                  const std::string& versionNumber)
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

// recording data factory method /
std::unique_ptr<BaseRecordingData> NWBFile::createRecordingData(
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
