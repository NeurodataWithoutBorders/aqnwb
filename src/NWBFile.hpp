#pragma once

#include <cstdint>
#include "BaseIO.hpp"
#include "HDF5IO.hpp"

using namespace AQNWBIO;

/**

Represents an NWB File

*/
class NWBFile
{
public:
  /** Constructor */
  NWBFile(std::string fileName, std::string idText, std::unique_ptr<BaseIO> io);
  NWBFile(const NWBFile&) = delete;  // non construction-copyable
  NWBFile& operator=(const NWBFile&) = delete;  // non copiable

  /** Destructor */
  ~NWBFile();

  /** Opens the file for writing */
  void open();

  void close();

  bool startRecording(int recordingNumber);

  /** Writes the num_samples value and closes the relevant datasets */
  void stopRecording();

  /** Writes continuous data for a particular channel */
  void writeData(int datasetID,
                 int channel,
                 int nSamples,
                 const float* data,
                 float bitVolts);

  /** Saves the specification files for the schema */
  void cacheSpecifications(std::string specPath, std::string versionNumber);

  /** Returns the name of this NWB file */
  std::string getFileName();

//   /** Generate a new uuid string*/
//   std::string generateUuid();

  /** Indicate NWB version files will be saved as */
  const std::string NWBVersion = "2.7.0";

  /** Indicate HDMF version for schema files */
  const std::string HDMFVersion = "1.8.0";

protected:
  /** Create the default file structure */
  int createFileStructure();

private:
  /** Creates a new dataset to hold text data (messages) */
  void createTextDataSet(std::string path, std::string name, std::string text);

  const std::string filename;
  const std::string identifierText;
  std::unique_ptr<BaseIO> io;
  std::vector<float> scaledBuffer;  // TODO - switched out for std::vector, not sure if it's best substitute
  std::vector<int16_t> intBuffer;
  size_t bufferSize;
  bool readyToOpen;

};

/**

    Represents an NWB recording engine to manage recording process

    */
class NWBRecordingEngine
{
public:
  /** Constructor */
  NWBRecordingEngine();
  NWBRecordingEngine(const NWBRecordingEngine&) =
      delete;  // non construction-copyable
  NWBRecordingEngine& operator=(const NWBRecordingEngine&) =
      delete;  // non copiable

  /** Destructor */
  ~NWBRecordingEngine();

  /** Called when recording starts to open all needed files */
  void openFiles(std::string rootFolder,
                 int experimentNumber,
                 int recordingNumber);

  /** Called when recording stops to close all files and do all the necessary
   * cleanup */
  void closeFiles();

private:
  /** Pointer to the current NWB file */
  std::unique_ptr<NWBFile> nwb;

  /** Holds integer sample numbers for writing */
  std::vector<int64_t> smpBuffer;
};
