
#include "nwb/ecephys/ElectricalSeries.hpp"

#include "nwb/file/ElectrodeTable.hpp"

using namespace AQNWB::NWB;

// ElectricalSeries

/** Constructor */
ElectricalSeries::ElectricalSeries(const std::string& path,
                                   std::shared_ptr<IO::BaseIO> io)
    : TimeSeries(path, io)
{
}

/** Destructor */
ElectricalSeries::~ElectricalSeries() {}

/** Initialization function*/
void ElectricalSeries::initialize(const IO::BaseDataType& dataType,
                                  const Types::ChannelVector& channelVector,
                                  const std::string& description,
                                  const SizeArray& dsetSize,
                                  const SizeArray& chunkSize,
                                  const float& conversion,
                                  const float& resolution,
                                  const float& offset)
{
  TimeSeries::initialize(dataType,
                         "volts",
                         description,
                         channelVector[0].comments,
                         dsetSize,
                         chunkSize,
                         channelVector[0].getConversion(),
                         resolution,
                         offset);

  // setup variables based on number of channels
  std::vector<int> electrodeInds(channelVector.size());
  std::vector<float> channelConversions(channelVector.size());
  for (size_t i = 0; i < channelVector.size(); ++i) {
    electrodeInds[i] = channelVector[i].globalIndex;
    channelConversions[i] = channelVector[i].getConversion();
  }
  samplesRecorded = SizeArray(channelVector.size(), 0);

  // make channel conversion dataset
  channelConversion = std::unique_ptr<IO::BaseRecordingData>(
      io->createArrayDataSet(IO::BaseDataType::F32,
                             SizeArray {1},
                             chunkSize,
                             getPath() + "/channel_conversion"));
  channelConversion->writeDataBlock(
      std::vector<SizeType>(1, channelVector.size()),
      BaseDataType::F32,
      &channelConversions[0]);
  io->createCommonNWBAttributes(getPath() + "/channel_conversion",
                                "hdmf-common",
                                "",
                                "Bit volts values for all channels");

  // make electrodes dataset
  electrodesDataset = std::unique_ptr<IO::BaseRecordingData>(
      io->createArrayDataSet(IO::BaseDataType::I32,
                             SizeArray {1},
                             chunkSize,
                             getPath() + "/electrodes"));
  electrodesDataset->writeDataBlock(
      std::vector<SizeType>(1, channelVector.size()),
      IO::BaseDataType::I32,
      &electrodeInds[0]);
  io->createCommonNWBAttributes(
      getPath() + "/electrodes", "hdmf-common", "DynamicTableRegion", "");
  io->createReferenceAttribute(
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
