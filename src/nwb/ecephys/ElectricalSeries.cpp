
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
Status ElectricalSeries::initialize(const IO::BaseDataType& dataType,
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
                         conversion,
                         resolution,
                         offset);

  this->m_channelVector = channelVector;

  // get the number of electrodes from the electrode table
  std::string idPath = AQNWB::mergePaths(ElectrodeTable::electrodeTablePath, "id");
  std::vector<SizeType> elecTableDsetSize = m_io->getDatasetSize(idPath);
  SizeType numElectrodes = elecTableDsetSize[0];

  // setup variables based on number of channels
  std::vector<int> electrodeInds(channelVector.size());
  std::vector<float> channelConversions(channelVector.size());
  for (size_t i = 0; i < channelVector.size(); ++i) {
    SizeType globalIndex = channelVector[i].getGlobalIndex();
    if (globalIndex >= numElectrodes) {
      std::cerr << "ElectricalSeries::initialize electrode index " << globalIndex 
                << " is out of range. Max index is " << (numElectrodes - 1) << std::endl;
      return Status::Failure;
    }
    electrodeInds[i] = static_cast<int>(channelVector[i].getGlobalIndex());
    channelConversions[i] = channelVector[i].getConversion();
  }
  m_samplesRecorded = SizeArray(channelVector.size(), 0);

  // make channel conversion dataset
  m_channelConversion =
      std::unique_ptr<IO::BaseRecordingData>(m_io->createArrayDataSet(
          IO::BaseDataType::F32,
          SizeArray {1},
          chunkSize,
          AQNWB::mergePaths(getPath(), "/channel_conversion")));
  m_channelConversion->writeDataBlock(
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
  m_electrodesDataset = std::unique_ptr<IO::BaseRecordingData>(
      m_io->createArrayDataSet(IO::BaseDataType::I32,
                               SizeArray {channelVector.size()},
                               chunkSize,
                               AQNWB::mergePaths(getPath(), "electrodes")));

  m_electrodesDataset->writeDataBlock(SizeArray {channelVector.size()},
                                      IO::BaseDataType::I32,
                                      &electrodeInds[0]);
  auto electrodesPath = AQNWB::mergePaths(getPath(), "electrodes");
  m_io->createCommonNWBAttributes(
      electrodesPath, "hdmf-common", "DynamicTableRegion");
  m_io->createAttribute("the electrodes that generated this electrical series",
                        electrodesPath,
                        "description");
  m_io->createReferenceAttribute(ElectrodeTable::electrodeTablePath,
                                 AQNWB::mergePaths(getPath(), "electrodes"),
                                 "table");

  return Status::Success;
}

Status ElectricalSeries::writeChannel(SizeType channelInd,
                                      const SizeType& numSamples,
                                      const void* dataInput,
                                      const void* timestampsInput,
                                      const void* controlInput)
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
    return writeData(
        dataShape, positionOffset, dataInput, timestampsInput, controlInput);
  } else {
    return writeData(dataShape, positionOffset, dataInput);
  }
}
