#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "NWBFile.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "BaseIO.hpp"

using namespace AQNWBIO;
namespace fs = std::filesystem;

#define MAX_BUFFER_SIZE \
  40960  // TODO - maybe instead use constexpr size_t MAX_BUFFER_SIZE = 40960;

// NWBFile

NWBFile::NWBFile(std::string idText, std::unique_ptr<BaseIO> io)
    : identifierText(idText)
    , io(std::move(io))
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
  io->createGroup("general/extracellular_ephys/electrodes");

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
  io->setAttribute(
      generateUuid(), "general/extracellular_ephys/electrodes", "object_id");

  io->createGroup("/specifications");
  cacheSpecifications("core/", NWBVersion);
  cacheSpecifications("hdmf-common/", HDMFVersion);

  std::string time = getCurrentTime();
  io->createStringDataSet("/file_create_date",
                          time);  // TODO - this should be an array
  io->createStringDataSet("/session_description", "a recording session");
  io->createStringDataSet("/session_start_time", time);
  io->createStringDataSet("/timestamps_reference_time", time);
  io->createStringDataSet("/identifier", "test-identifier");

  return 0;
}

bool NWBFile::startRecording(int recordingNumber)
{
  // all recorded data is stored in the "acquisition" group
  std::string rootPath = "/acquisition/";

  // TODO - tons of setup things here
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

std::string NWBFile::generateUuid()
{
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuidStr = boost::uuids::to_string(uuid);

  return uuidStr;
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