#include "NWBRecording.hpp"

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

Status NWBRecording::openFile(const std::string& rootFolder,
                              const std::string& baseName,
                              int experimentNumber,
                              std::vector<Types::ChannelGroup> recordingArrays,
                              const std::string& IOType)
{
  std::string filename =
      rootFolder + baseName + std::to_string(experimentNumber) + ".nwb";

  // initialize nwbfile object and create base structure
  nwbfile = std::make_unique<NWB::NWBFile>(generateUuid(),
                                           createIO(IOType, filename));
  nwbfile->initialize();

  // start the new recording
  return nwbfile->startRecording(recordingArrays);
}

void NWBRecording::closeFile()
{
  nwbfile->stopRecording();
  nwbfile->finalize();
}

void NWBRecording::writeTimeseriesData(SizeType timeseriesInd,
                                       Channel channel,
                                       const void* data,
                                       const void* timestamps,
                                       SizeType numSamples,
                                       std::vector<SizeType> positionOffset)
{
  // write data and timestamps to datasets
  std::vector<SizeType> dataShape = {numSamples, 1};
  if (channel.localIndex == 0) {
    // write with timestamps if it's the first channel
    nwbfile->writeTimeseries(timeseriesInd,
                             dataShape,
                             positionOffset,
                             BaseDataType::I16,
                             data,
                             BaseDataType::F64,
                             timestamps);
  } else {
    // write without timestamps if its another channel in the same timeseries
    nwbfile->writeTimeseries(
        timeseriesInd, dataShape, positionOffset, BaseDataType::I16, data);
  }
}
