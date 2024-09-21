
#include "nwb/ecephys/ElectricalSeries.hpp"

#include "nwb/file/ElectrodeTable.hpp"

using namespace AQNWB::NWB;

// ElectricalSeries

/** Constructor */
ElectricalSeries::ElectricalSeries(const std::string& path,
                                   std::shared_ptr<BaseIO> io,
                                   const BaseDataType& dataType,
                                   const Types::ChannelVector& channelVector,
                                   const std::string& description,
                                   const SizeArray& dsetSize,
                                   const SizeArray& chunkSize,
                                   const float& conversion,
                                   const float& resolution,
                                   const float& offset)
    : TimeSeries(path,
                 io,
                 dataType,
                 "volts",  // default unit for Electrical Series
                 description,
                 channelVector[0].getComments(),
                 dsetSize,
                 chunkSize,
                 channelVector[0].getConversion(),
                 resolution,
                 offset)
    , channelVector(channelVector)
{
}

/** Destructor */
ElectricalSeries::~ElectricalSeries() {}

/** Initialization function*/
void ElectricalSeries::initialize()
{
  TimeSeries::initialize();

  // setup variables based on number of channels
  std::vector<int> electrodeInds(channelVector.size());
  std::vector<float> channelConversions(channelVector.size());
  for (size_t i = 0; i < channelVector.size(); ++i) {
    electrodeInds[i] = channelVector[i].getGlobalIndex();
    channelConversions[i] = channelVector[i].getConversion();
  }
  samplesRecorded = SizeArray(channelVector.size(), 0);

  // make channel conversion dataset
  channelConversion = std::unique_ptr<BaseRecordingData>(
      m_io->createArrayDataSet(BaseDataType::F32,
                               SizeArray {1},
                               chunkSize,
                               getPath() + "/channel_conversion"));
  channelConversion->writeDataBlock(
      std::vector<SizeType>(1, channelVector.size()),
      BaseDataType::F32,
      &channelConversions[0]);
  // add axis attribute for channel conversion
  const signed int axis_value = 1;
  m_io->createAttribute(BaseDataType::I32,
                        &axis_value,
                        this->getPath() + "/channel_conversion",
                        "axis",
                        1);

  // make electrodes dataset
  electrodesDataset = std::unique_ptr<BaseRecordingData>(
      m_io->createArrayDataSet(BaseDataType::I32,
                               SizeArray {1},
                               chunkSize,
                               getPath() + "/electrodes"));
  electrodesDataset->writeDataBlock(
      std::vector<SizeType>(1, channelVector.size()),
      BaseDataType::I32,
      &electrodeInds[0]);
  m_io->createCommonNWBAttributes(
      getPath() + "/electrodes", "hdmf-common", "DynamicTableRegion", "");
  m_io->createReferenceAttribute(
      ElectrodeTable::electrodeTablePath, getPath() + "/electrodes", "table");
}

Status ElectricalSeries::writeChannel(SizeType channelInd,
                                      const SizeType& numSamples,
                                      const void* data,
                                      const void* timestamps)
{
  // get offsets and datashape
  std::vector<SizeType> dataShape = {
      numSamples, 1};  // Note: schema has 1D and 3D but planning to deprecate
  std::vector<SizeType> positionOffset = {samplesRecorded[channelInd],
                                          channelInd};

  // track samples recorded per channel
  samplesRecorded[channelInd] += numSamples;

  // write channel data
  if (channelInd == 0) {
    return writeData(dataShape, positionOffset, data, timestamps);
  } else {
    return writeData(dataShape, positionOffset, data);
  }
}
