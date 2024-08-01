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
                              std::vector<Types::ChannelVector> recordingArrays,
                              const std::string& IOType)
{
  std::string filename =
      rootFolder + baseName + std::to_string(experimentNumber) + ".nwb";

  // initialize nwbfile object and create base structure
  nwbfile = std::make_unique<NWB::NWBFile>(generateUuid(),
                                           createIO(IOType, filename));
  nwbfile->initialize();

  // create the datasets
  nwbfile->createElectricalSeries(recordingArrays);

  // start the new recording
  return nwbfile->startRecording();
}

void NWBRecording::closeFile()
{
  nwbfile->finalize();
}

Status NWBRecording::writeTimeseriesData(
    const std::string& containerName,
    const SizeType& timeseriesInd,
    const Channel& channel,
    const std::vector<SizeType>& dataShape,
    const std::vector<SizeType>& positionOffset,
    const void* data,
    const void* timestamps)
{
  TimeSeries* ts = nwbfile->getTimeSeries(timeseriesInd);

  if (ts == nullptr)
    return Status::Failure;

  // write data and timestamps to datasets
  if (channel.localIndex == 0) {
    // write with timestamps if it's the first channel
    return ts->writeData(dataShape, positionOffset, data, timestamps);
  } else {
    // write without timestamps if its another channel in the same timeseries
    return ts->writeData(dataShape, positionOffset, data);
  }
}
