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
                             int experimentNumber)
{
    std::string filename = rootFolder + baseName + std::to_string(experimentNumber) + ".nwb";

    // initialize nwbfile object and create base structure
    nwbfile = std::make_unique<NWB::NWBFile>(generateUuid(), std::make_unique<HDF5::HDF5IO>(filename));  // TODO make this generic IO
    nwbfile->initialize();


    // get channel group information from acquisition system
    // TODO - adapt this for customizability but right now use test array
    Channel ch0 = Channel("ch0", "array1", 0, 0);
    Channel ch1 = Channel("ch1", "array1", 1, 1);
    Channel ch3 = Channel("ch3", "array2", 0, 2);
    Channel ch4 = Channel("ch4", "array2", 1, 3);
    std::vector<Types::ChannelGroup> arrays = {Types::ChannelGroup {ch0, ch1}};

    // TODO - use number of channels detected to open a file with optimal chunking for channels

    // start the new recording
    Status result = nwbfile->startRecording(arrays);  // TODO - add recording number to stop and restart recording to same file

    return result;
}

void NWBRecording::closeFiles()
{
  nwbfile->stopRecording();
  nwbfile->finalize();
}

void NWBRecording::writeTimeseriesData(int timeseriesInd,  // TODO - do I want to pass in a timeseries itself here?
                        Channel channel,
                        const float *dataBuffer,
                        const double *timestampBuffer,
                        int numSamples)
{

  // TODO - add lines here that map from channel indices to dataset indices

  // copy data and multiply by scaling factor
  double multFactor = 1 / (float(0x7fff) * channel.getBitVolts());
  std::transform(dataBuffer, dataBuffer + numSamples, scaledBuffer.get(), [multFactor](float value) {
    return value * multFactor;
    });

  // convert float to int16
  std::transform(scaledBuffer.get(), scaledBuffer.get() + numSamples, intBuffer.get(), [](float value) 
  {
    return static_cast<int16_t>(std::lround(value * 32767.0f));
  });

  // write intBuffer data to dataset
  Types::TimeSeriesData* tsData = nwbfile->getTimeSeriesData();
  // tsData[timeseriesInd]->data->writeDataRow(numSamples, channel.localIndex, BaseDataType::I16, intBuffer);
  // if (timeseriesInd == 0)
  // {
  //     tsData[timeseriesInd]->timestamps->writeDataBlock(numSamples, BaseDataType::F64, timestampBuffer);
  // }
}