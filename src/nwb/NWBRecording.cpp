
#include "nwb/NWBRecording.hpp"

#include "Channel.hpp"
#include "Utils.hpp"
#include "hdf5/HDF5IO.hpp"

using namespace AQNWB::NWB;

// NWBRecordingEngine
NWBRecording::NWBRecording() {}

NWBRecording::~NWBRecording()
{
  if (nwbfile != nullptr) {
    nwbfile->finalize();
  }
}

Status NWBRecording::openFile(const std::string& filename,
                              std::vector<Types::ChannelVector> recordingArrays,
                              const std::string& IOType,
                              RecordingContainers* recordingContainers)
{
  // close any existing files
  if (nwbfile != nullptr) {
    this->closeFile();
  }

  // initialize nwbfile object and create base structure
  nwbfile = std::make_unique<NWB::NWBFile>(generateUuid(),
                                           createIO(IOType, filename));
  nwbfile->initialize();

  // create the datasets
  nwbfile->createElectricalSeries(recordingArrays,
                                  BaseDataType::I16,
                                  recordingContainers);

  // start the new recording
  return nwbfile->startRecording();
}

void NWBRecording::closeFile()
{
  nwbfile->stopRecording();
  nwbfile->finalize();
}
