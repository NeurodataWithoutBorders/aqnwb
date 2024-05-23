#include "NWBRecording.hpp"

#include "Utils.hpp"
#include "hdf5/HDF5IO.hpp"

constexpr int MAX_BUFFER_SIZE = 40960;

using namespace AQNWB::NWB;

// NWBRecordingEngine
NWBRecording::NWBRecording()
{
  sampleBuffer = std::make_unique<int[]>(MAX_BUFFER_SIZE);
  scaledBuffer = std::make_unique<float[]>(MAX_BUFFER_SIZE);
  intBuffer = std::make_unique<int16_t[]>(MAX_BUFFER_SIZE);
}

NWBRecording::~NWBRecording()
{
  if (nwbfile != nullptr) {
    nwbfile->finalize();
  }
}

Status NWBRecording::openFiles(const std::string& rootFolder,
                               const std::string& baseName,
                               int experimentNumber,
                               std::vector<Types::ChannelGroup> recordingArrays)
{
  std::string filename =
      rootFolder + baseName + std::to_string(experimentNumber) + ".nwb";

  // initialize nwbfile object and create base structure
  nwbfile = std::make_unique<NWB::NWBFile>(
      generateUuid(),
      std::make_unique<HDF5::HDF5IO>(
          filename));  // TODO make this generic IO, what's the best way to
                       // switch between?
  nwbfile->initialize();

  // TODO - use number of channels detected to open a file with optimal chunking
  // for channels

  // start the new recording
  return nwbfile->startRecording(
      recordingArrays);  // TODO - add recording number to stop and restart
                         // recording to same file
}

void NWBRecording::closeFiles()
{
  nwbfile->stopRecording();
  nwbfile->finalize();
}

void NWBRecording::writeTimeseriesData(int timeseriesInd,
                                       Channel channel,
                                       const float* dataBuffer,
                                       const double* timestampBuffer,
                                       int numSamples)
{
  // copy data and multiply by scaling factor
  double multFactor = 1 / (float(0x7fff) * channel.getBitVolts());
  std::transform(dataBuffer,
                 dataBuffer + numSamples,
                 scaledBuffer.get(),
                 [multFactor](float value) { return value * multFactor; });

  // convert float to int16
  std::transform(scaledBuffer.get(),
                 scaledBuffer.get() + numSamples,
                 intBuffer.get(),
                 [](float value) {
                   return static_cast<int16_t>(std::lround(value * 32767.0f));
                 });

  // write intBuffer data to dataset
  nwbfile->writeContinuousData(timeseriesInd,
                               channel.localIndex,
                               numSamples,
                               BaseDataType::I16,
                               intBuffer.get());
  if (timeseriesInd == 0) {
    nwbfile->writeContinuousData(
        timeseriesInd, numSamples, BaseDataType::F64, timestampBuffer);
  }
}