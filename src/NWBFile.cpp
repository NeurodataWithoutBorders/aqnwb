#include <chrono>
#include <string>
#include <filesystem>

#include "NWBFile.hpp"
#include "BaseIO.hpp"

using namespace AQNWBIO;
namespace fs = std::filesystem;

#define MAX_BUFFER_SIZE 40960


// NWBFile

NWBFile::NWBFile(std::string fileName, std::string idText, std::unique_ptr<BaseIO> io)
    :
    filename(fileName),
    identifierText(idText),
    io(std::move(io))
{
  readyToOpen = true;

//   scaledBuffer.malloc(MAX_BUFFER_SIZE);  TODO - JUCE types, need to adapt for std library
//   intBuffer.malloc(MAX_BUFFER_SIZE);
  bufferSize = MAX_BUFFER_SIZE;
}

NWBFile::~NWBFile() {}

void NWBFile::open()
{
  io->open(filename);
  createFileStructure();
}

void NWBFile::close()
{
  io->close();
}

int NWBFile::createFileStructure()
{
  io->setAttribute("core", "/", "namespace");
  io->setAttribute("NWBFile", "/", "neurodata_type");
  io->setAttribute(NWBVersion, "/", "nwb_version");
  io->setAttribute(identifierText, "/", "object_id");

  if (io->createGroup("/acquisition"))
    return -1;

  if (io->createGroup("/analysis"))
    return -1;

  // TODO - use chrono to get iso date time
  // std::string time = Time::getCurrentTime().formatted("%Y-%m-%dT%H:%M:%S")
  //     + Time::getCurrentTime().getUTCOffsetString(true);

  // createTextDataSet("", "file_create_date", time);

  // Get the current time point
  auto now = std::chrono::system_clock::now();
  std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
  // Convert the time_t to a tm structure
  std::tm* timeInfo = std::localtime(&currentTime);
  // Format the time in ISO 8601 date-time format
  char buffer[20];  // ISO 8601 date-time format requires at least 20 characters
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", timeInfo);

  if (io->createGroup("/general"))
    return -1;
  if (io->createGroup("general/devices"))
    return -1;
  if (io->createGroup("general/extracellular_ephys"))
    return -1;
  if (io->createGroup("general/extracellular_ephys/electrodes"))
    return -1;

  const std::vector<std::string> colnames = {"group", "group_name", "location"};

  io->setAttribute(
      colnames, "general/extracellular_ephys/electrodes", "colnames");
  io->setAttribute("metadata about extracellular electrodes",
                      "general/extracellular_ephys/electrodes",
                      "description");
  io->setAttribute(
      "hdmf-common", "general/extracellular_ephys/electrodes", "namespace");
  io->setAttribute("DynamicTable",
                      "general/extracellular_ephys/electrodes",
                      "neurodata_type");
  // io->setAttributeStr(generateUuid(),
  // "general/extracellular_ephys/electrodes", "object_id");

  if (io->createGroup("/processing"))
    return -1;

  if (io->createGroup("/stimulus"))
    return -1;
  if (io->createGroup("/stimulus/presentation"))
    return -1;
  if (io->createGroup("/stimulus/templates"))
    return -1;

  if (io->createGroup("/specifications"))
    return -1;
  cacheSpecifications("core/", NWBVersion);
  cacheSpecifications("hdmf-common/", HDMFVersion);

  // createStringDataSet("/session_description", "Recording with the Open Ephys
  // GUI"); createStringDataSet("/session_start_time", time);
  // createStringDataSet("/timestamps_reference_time", time);
  // createStringDataSet("/identifier", "test-identifier");

  return 0;
}

bool NWBFile::startRecording(int recordingNumber)
{
  // all recorded data is stored in the "acquisition" group
  std::string rootPath = "/acquisition/";
  
  // TODO - tons of setup things here
  return true;
}

void NWBFile::stopRecording()
{
  
}

void NWBFile::writeData(
    int datasetID, int channel, int nSamples, const float* data, float bitVolts)
{
   // TODO - some things here
}

std::string NWBFile::getFileName()
{
  return filename;
}


void NWBFile::cacheSpecifications(std::string specPath, std::string versionNumber)
{
  io->createGroup("/specifications/" + specPath);
  io->createGroup("/specifications/" + specPath + versionNumber);

  // fs::path currentFile = __FILE__;
  // fs::path schemaDir = currentFile.parent_path().parent_path().parent_path() / "resources/nwb-schema" / specPath / versionNumber;

  // for (auto const& entry : fs::directory_iterator{schemaDir})
  //   if (fs::is_regular_file(entry) && entry.path().extension() == ".json")
  //   {
  //       std::string specName = entry.path().filename().replace_extension("").string();
  //       if (specName.contains("namespace")) specName = "namespace";
  //       createStringDataSet("/specifications/" + specPath + versionNumber + specName, schemaFile.loadFileAsString()); TODO - replace with actual schema writing
  //   }
}


// NWBRecordingEngine
 NWBRecordingEngine::NWBRecordingEngine()
 {
	//  smpBuffer.malloc(MAX_BUFFER_SIZE); // TODO - JUCE type, need to adapt
 }


 NWBRecordingEngine::~NWBRecordingEngine()
 {
     if (nwb != nullptr)
     {
        //  nwb->close();  // TODO - figure out which BaseIO / RecordingData things to handle
        //  nwb.reset();
     }
 }
 
 void NWBRecordingEngine::openFiles(std::string rootFolder, int experimentNumber, int recordingNumber)
 {
 }

 
 void NWBRecordingEngine::closeFiles()
 {
	 nwb->stopRecording();
 }