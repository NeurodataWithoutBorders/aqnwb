#include "nwb/ecephys/ElectricalSeries.hpp"

using namespace AQNWB::NWB;

// ElectricalSeries

/** Constructor */
ElectricalSeries::ElectricalSeries(const std::string& path,
                                   std::shared_ptr<BaseIO> io,
                                   const std::string& description,
                                   const Types::ChannelGroup& channelGroup,
                                   const SizeType& chunkSize,
                                   const std::string& electrodesTablePath)
    : TimeSeries(path, io, description)
    , channelGroup(channelGroup)
    , chunkSize(chunkSize)
    , electrodesTablePath(electrodesTablePath)
{
}

/** Destructor */
ElectricalSeries::~ElectricalSeries() {}

/** Initialization function*/
void ElectricalSeries::initialize()
{
  TimeSeries::initialize();

  std::vector<SizeType> electrodeInds(channelGroup.size());
  for (size_t i = 0; i < channelGroup.size(); ++i) {
    electrodeInds[i] = channelGroup[i].globalIndex;
  }

  // make data dataset
  data = std::unique_ptr<BaseRecordingData>(io->createDataSet(BaseDataType::I16, 
                                            SizeArray {0, channelGroup.size()}, 
                                            SizeArray {chunkSize}, 
                                            getPath() + "/data"));
  io->createDataAttributes(getPath(),
                           channelGroup[0].getConversion(),
                           -1.0f,
                           "volts");

  // make timestamps dataset
  timestamps = std::unique_ptr<BaseRecordingData>(io->createDataSet(BaseDataType::F64,
                                                  SizeArray {0},
                                                  SizeArray {chunkSize},
                                                  getPath() + "/timestamps"));
  io->createTimestampsAttributes(getPath());

  // make channel conversion dataset
  channelConversion = std::unique_ptr<BaseRecordingData>(io->createDataSet(BaseDataType::F32,
                                                                           SizeArray {1},
                                                                           SizeArray {chunkSize},
                                                                           getPath() + "/channel_conversion"));
  io->createCommonNWBAttributes(getPath() + "/channel_conversion",
                                "hdmf-common",
                                "",
                                "Bit volts values for all channels");

  // make electrodes dataset
  electrodesDataset = std::unique_ptr<BaseRecordingData>(io->createDataSet(BaseDataType::I32,
                                                                           SizeArray {1},
                                                                           SizeArray {chunkSize},
                                                                           getPath() + "/electrodes"));
  electrodesDataset->writeDataBlock(channelGroup.size(), 
                                    BaseDataType::I32, 
                                    &electrodeInds[0]);
  io->createCommonNWBAttributes(getPath() + "/electrodes",
                                "hdmf-common",
                                "DynamicTableRegion",
                                "");
  io->createReferenceAttribute(electrodesTablePath,
                               getPath() + "/electrodes",
                               "table");
}

