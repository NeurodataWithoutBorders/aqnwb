#include "nwb/ecephys/ElectricalSeries.hpp"

#include "Utils.hpp"
#include "nwb/file/ElectrodesTable.hpp"

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
Status ElectricalSeries::initialize(
    const IO::BaseArrayDataSetConfig& dataConfig,
    const Types::ChannelVector& channelVector,
    const std::string& description,
    const float& conversion,
    const float& resolution,
    const float& offset)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "ElectricalSeries::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  auto tsInitStatus = TimeSeries::initialize(dataConfig,
                                             "volts",
                                             description,
                                             channelVector[0].getComments(),
                                             conversion,
                                             resolution,
                                             offset);

  this->m_channelVector = channelVector;

  // Extract chunking information from data config
  SizeArray chunking;
  if (dataConfig.isLink()) {
    // For links, use convenience method to derive chunking from target dataset
    const IO::LinkArrayDataSetConfig* linkConfig =
        dynamic_cast<const IO::LinkArrayDataSetConfig*>(&dataConfig);
    if (!linkConfig) {
      std::cerr << "ElectricalSeries::initialize: Failed to cast to "
                   "LinkArrayDataSetConfig for link data"
                << std::endl;
      return Status::Failure;
    }
    chunking = linkConfig->getTargetChunking(*ioPtr);
  } else {
    const IO::ArrayDataSetConfig* arrayConfig =
        dynamic_cast<const IO::ArrayDataSetConfig*>(&dataConfig);
    if (!arrayConfig) {
      std::cerr << "ElectricalSeries::initialize: Failed to cast to "
                   "ArrayDataSetConfig for array data"
                << std::endl;
      return Status::Failure;
    }
    chunking = arrayConfig->getChunking();
  }

  // Use default chunking if empty (e.g., for non-chunked linked datasets)
  if (chunking.empty()) {
    chunking = SizeArray {1, 1};  // Default: 1 sample, 1 channel per chunk
  }

  // get the number of electrodes from the electrode table
  std::string idPath =
      AQNWB::mergePaths(ElectrodesTable::electrodesTablePath, "id");
  std::vector<SizeType> elecTableDsetSize =
      ioPtr->getStorageObjectShape(idPath);
  SizeType numElectrodes = elecTableDsetSize[0];

  // setup variables based on number of channels
  std::vector<int> electrodeInds(channelVector.size());
  std::vector<float> channelConversions(channelVector.size());
  for (size_t i = 0; i < channelVector.size(); ++i) {
    SizeType globalIndex = channelVector[i].getGlobalIndex();
    if (globalIndex >= numElectrodes) {
      std::cerr << "ElectricalSeries::initialize electrode index "
                << globalIndex << " is out of range. Max index is "
                << (numElectrodes - 1) << std::endl;
      return Status::Failure;
    }
    electrodeInds[i] = static_cast<int>(channelVector[i].getGlobalIndex());
    channelConversions[i] = channelVector[i].getConversion();
  }
  m_samplesRecorded = SizeArray(channelVector.size(), 0);

  // make channel conversion dataset (1D array with num_channels elements)
  // Use chunking for channel dimension if available, otherwise default to full size
  SizeArray channelChunking = {channelVector.size()};
  if (chunking.size() >= 2 && chunking[1] > 0) {
    channelChunking = SizeArray {chunking[1]};
  }
  IO::ArrayDataSetConfig channelConversionConfig(
      IO::BaseDataType::F32, SizeArray {channelVector.size()}, channelChunking);
  ioPtr->createArrayDataSet(
      channelConversionConfig,
      AQNWB::mergePaths(getPath(), "/channel_conversion"));
  auto channelConversionRecorder = recordChannelConversion();
  channelConversionRecorder->writeDataBlock(
      std::vector<SizeType>(1, channelVector.size()),
      IO::BaseDataType::F32,
      &channelConversions[0]);
  // add axis attribute for channel conversion
  const signed int axis_value = 1;
  ioPtr->createAttribute(IO::BaseDataType::I32,
                         &axis_value,
                         AQNWB::mergePaths(getPath(), "channel_conversion"),
                         "axis",
                         1);

  // make electrodes dataset (1D array with num_channels elements)
  IO::ArrayDataSetConfig electrodesConfig(
      IO::BaseDataType::I32, SizeArray {channelVector.size()}, channelChunking);
  ioPtr->createArrayDataSet(electrodesConfig,
                            AQNWB::mergePaths(getPath(), "electrodes"));

  auto electrodesRecorder = recordElectrodes();
  electrodesRecorder->writeDataBlock(SizeArray {channelVector.size()},
                                     IO::BaseDataType::I32,
                                     &electrodeInds[0]);
  auto electrodesPath = AQNWB::mergePaths(getPath(), "electrodes");
  ioPtr->createCommonNWBAttributes(
      electrodesPath, "hdmf-common", "DynamicTableRegion");
  ioPtr->createAttribute("the electrodes that generated this electrical series",
                         electrodesPath,
                         "description");
  ioPtr->createReferenceAttribute(ElectrodesTable::electrodesTablePath,
                                  AQNWB::mergePaths(getPath(), "electrodes"),
                                  "table");

  return tsInitStatus;
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
