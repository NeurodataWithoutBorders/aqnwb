#include "nwb/ecephys/ElectricalSeries.hpp"

using namespace AQNWB::NWB;

// ElectricalSeries

/** Constructor */
ElectricalSeries::ElectricalSeries(const std::string& path,
                                   std::shared_ptr<BaseIO> io,
                                   const BaseDataType& dataType,
                                   const Types::ChannelGroup& channelGroup,
                                   const std::string& electrodesTablePath,
                                   const std::string& unit,
                                   const std::string& description,
                                   const std::string& comments,
                                   const SizeArray& dsetSize,
                                   const SizeArray& chunkSize,
                                   const float& conversion,
                                   const float& resolution,
                                   const float& offset)
    : TimeSeries(path,
                 io,
                 dataType,
                 unit,
                 description,
                 comments,
                 dsetSize,
                 chunkSize,
                 channelGroup[0].getConversion(),
                 resolution,
                 offset)
    , channelGroup(channelGroup)
    , electrodesTablePath(electrodesTablePath)
{
}

/** Destructor */
ElectricalSeries::~ElectricalSeries() {}

/** Initialization function*/
void ElectricalSeries::initialize()
{
  TimeSeries::initialize();

  // setup variables based on number of channels
  std::vector<SizeType> electrodeInds(channelGroup.size());
  for (size_t i = 0; i < channelGroup.size(); ++i) {
    electrodeInds[i] = channelGroup[i].globalIndex;
  }
  samplesRecorded = SizeArray(channelGroup.size(), 0);

  // make channel conversion dataset
  channelConversion = std::unique_ptr<BaseRecordingData>(
      io->createDataSet(BaseDataType::F32,
                        SizeArray {1},
                        chunkSize,
                        getPath() + "/channel_conversion"));
  io->createCommonNWBAttributes(getPath() + "/channel_conversion",
                                "hdmf-common",
                                "",
                                "Bit volts values for all channels");

  // make electrodes dataset
  electrodesDataset = std::unique_ptr<BaseRecordingData>(io->createDataSet(
      BaseDataType::I32, SizeArray {1}, chunkSize, getPath() + "/electrodes"));
  electrodesDataset->writeDataBlock(
      std::vector<SizeType>(1, channelGroup.size()),
      BaseDataType::I32,
      &electrodeInds[0]);
  io->createCommonNWBAttributes(
      getPath() + "/electrodes", "hdmf-common", "DynamicTableRegion", "");
  io->createReferenceAttribute(
      electrodesTablePath, getPath() + "/electrodes", "table");
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
