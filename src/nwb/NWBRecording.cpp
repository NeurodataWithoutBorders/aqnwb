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
                                       const float* data,
                                       const double* timestamps,
                                       SizeType numSamples,
                                       std::vector<SizeType> positionOffset)
{
  scaledData = std::make_unique<float[]>(numSamples);
  intData = std::make_unique<int16_t[]>(numSamples);

  // copy data and multiply by scaling factor
  double multFactor = 1 / (32767.0f * channel.getBitVolts());
  std::transform(data,
                 data + numSamples,
                 scaledData.get(),
                 [multFactor](float value) { return value * multFactor; });

  // convert float to int16
  std::transform(
      scaledData.get(),
      scaledData.get() + numSamples,
      intData.get(),
      [](float value)
      { return static_cast<int16_t>(std::clamp(value, -32768.0f, 32767.0f)); });

  // write data and timestamps to datasets
  std::vector<SizeType> dataShape = {numSamples, 1};
  if (channel.localIndex == 0) {
    // write with timestamps if it's the first channel
    nwbfile->writeTimeseries(timeseriesInd,
                             dataShape,
                             positionOffset,
                             BaseDataType::I16,
                             intData.get(),
                             BaseDataType::F64,
                             timestamps);
  } else {
    // write without timestamps if its another channel in the same timeseries
    nwbfile->writeTimeseries(timeseriesInd,
                             dataShape,
                             positionOffset,
                             BaseDataType::I16,
                             intData.get());
  }
}
