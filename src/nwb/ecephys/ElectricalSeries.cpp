
#include "nwb/ecephys/ElectricalSeries.hpp"

#include "Utils.hpp"
#include "nwb/file/ElectrodeTable.hpp"

using namespace AQNWB::NWB;

// ElectricalSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(ElectricalSeries)

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
                         channelVector[0].getComments(),
                         dsetSize,
                         chunkSize,
                         channelVector[0].getConversion(),
                         resolution,
                         offset);

  // setup variables based on number of channels
  std::vector<SizeType> electrodeInds(channelVector.size());
  std::vector<float> channelConversions(channelVector.size());
  for (size_t i = 0; i < channelVector.size(); ++i) {
    electrodeInds[i] = channelVector[i].getGlobalIndex();
    channelConversions[i] = channelVector[i].getConversion();
  }
  m_samplesRecorded = SizeArray(channelVector.size(), 0);

  // make channel conversion dataset
  channelConversion =
      std::unique_ptr<IO::BaseRecordingData>(m_io->createArrayDataSet(
          IO::BaseDataType::F32,
          SizeArray {1},
          chunkSize,
          AQNWB::mergePaths(getPath(), "/channel_conversion")));
  channelConversion->writeDataBlock(
      std::vector<SizeType>(1, channelVector.size()),
      IO::BaseDataType::F32,
      &channelConversions[0]);
  // add axis attribute for channel conversion
  const signed int axis_value = 1;
  m_io->createAttribute(IO::BaseDataType::I32,
                        &axis_value,
                        AQNWB::mergePaths(getPath(), "channel_conversion"),
                        "axis",
                        1);

  // make electrodes dataset
  electrodesDataset = std::unique_ptr<IO::BaseRecordingData>(
      m_io->createArrayDataSet(IO::BaseDataType::I32,
                               SizeArray {1},
                               chunkSize,
                               AQNWB::mergePaths(getPath(), "electrodes")));

  electrodesDataset->writeDataBlock(
      std::vector<SizeType>(1, channelVector.size()),
      IO::BaseDataType::I32,
      &electrodeInds[0]);
  m_io->createCommonNWBAttributes(
      AQNWB::mergePaths(getPath(), "electrodes"),
      "hdmf-common",
      "DynamicTableRegion",
      "the electrodes that generated this electrical series");
  m_io->createReferenceAttribute(ElectrodeTable::electrodeTablePath,
                                 AQNWB::mergePaths(getPath(), "electrodes"),
                                 "table");
}

Status ElectricalSeries::writeChannel(SizeType channelInd,
                                      const SizeType& numSamples,
                                      const void* dataInput,
                                      const void* timestampsInput)
{
  // get offsets and datashape
  std::vector<SizeType> dataShape = {
      numSamples, 1};  // Note: schema has 1D and 3D but planning to deprecate
  std::vector<SizeType> positionOffset = {m_samplesRecorded[channelInd],
                                          channelInd};

  // track samples recorded per channel
  m_samplesRecorded[channelInd] += numSamples;

  // write channel data
  if (channelInd == 0) {
    return writeData(dataShape, positionOffset, dataInput, timestampsInput);
  } else {
    return writeData(dataShape, positionOffset, dataInput);
  }
}
