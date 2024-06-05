#include "NWBRecording.hpp"

#include "Channel.hpp"
#include "Utils.hpp"
#include "hdf5/HDF5IO.hpp"

constexpr SizeType MAX_BUFFER_SIZE = 40960;

using namespace AQNWB::NWB;

// NWBRecordingEngine
NWBRecording::NWBRecording()
{
  scaledBuffer = std::make_unique<float[]>(MAX_BUFFER_SIZE);
  intBuffer = std::make_unique<int16_t[]>(MAX_BUFFER_SIZE);
  bufferSize = MAX_BUFFER_SIZE;
}

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
                                       const float* dataBuffer,
                                       const double* timestampBuffer,
                                       SizeType numSamples)
{
  // check if more samples than allocated buffer size
  if (numSamples > bufferSize)
	 {
		 bufferSize = numSamples;
     scaledBuffer = std::make_unique<float[]>(numSamples);
     intBuffer = std::make_unique<int16_t[]>(numSamples);
	 }

  // copy data and multiply by scaling factor
  double multFactor = 1 / (32767.0f * channel.getBitVolts());
  std::transform(dataBuffer,
                 dataBuffer + numSamples,
                 scaledBuffer.get(),
                 [multFactor](float value) { return value * multFactor; });

  // convert float to int16
  std::transform(
      scaledBuffer.get(),
      scaledBuffer.get() + numSamples,
      intBuffer.get(),
      [](float value)
      { return static_cast<int16_t>(std::clamp(value, -32768.0f, 32767.0f)); });

  // write data and timestamps to datasets
  nwbfile->writeTimeseriesData(timeseriesInd,
                               channel.localIndex,
                               numSamples,
                               BaseDataType::I16,
                               intBuffer.get());
  if (channel.localIndex == 0) {
    nwbfile->writeTimeseriesTimestamps(
        timeseriesInd, numSamples, BaseDataType::F64, timestampBuffer);
  }
}
