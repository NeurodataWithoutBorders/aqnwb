#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Types.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "Utils.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("ElectricalSeries", "[ecephys]")
{
  // setup recording info
  SizeType numSamples = 100;
  SizeType numChannels = 2;
  SizeType bufferSize = numSamples / 5;
  std::vector<float> dataBuffer(bufferSize);
  std::vector<double> timestampsBuffer(bufferSize);
  std::vector<Types::ChannelGroup> mockArrays = getMockChannelArrays();
  std::string dataPath = "/esdata";
  BaseDataType dataType = BaseDataType::F32;
  std::vector<std::vector<float>> mockData = getMockData2D(numSamples, numChannels);
  BaseDataType timestampsType = BaseDataType::F64;
  std::vector<double> mockTimestamps = getMockTimestamps(numSamples, 1);
  std::string devicePath = "/device";
  std::string electrodePath = "/elecgroup/";


  SECTION("test initialization") {
    // TODO
  }

  SECTION("test linking to electrode table region") {
    // TODO
  }

  SECTION("test writing channels"){
    // setup io object
    std::string path = getTestFilePath("ElectricalSeries.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // setup electrode table, device, and electrode group
    std::string elecTablePath =  "/electrodes/";
    NWB::ElectrodeTable elecTable = NWB::ElectrodeTable(elecTablePath, io);
    elecTable.initialize();

    // setup electrical series
    NWB::ElectricalSeries es = NWB::ElectricalSeries(dataPath, io, dataType, timestampsType, mockArrays[0], elecTable.getPath(), "volts", "no description", "no comments",
        SizeArray{0, mockArrays[0].size()});
    es.initialize();

    // write channel data
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      es.writeChannel(ch, numSamples, mockData[ch].data(), mockTimestamps.data());
    }
    io->close();

    // Read data back from file
    std::unique_ptr<H5::H5File> file =
        std::make_unique<H5::H5File>(path, H5F_ACC_RDONLY);
    std::unique_ptr<H5::DataSet> dataset =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath + "/data"));
    std::vector<std::vector<float>> dataOut(numChannels,
                                            std::vector<float>(numSamples));
    float* buffer = new float[numSamples * numChannels];

    H5::DataSpace fSpace = dataset->getSpace();
    hsize_t dims[1] = {numSamples * numChannels};
    H5::DataSpace mSpace(1, dims);
    dataset->read(buffer, H5::PredType::NATIVE_FLOAT, mSpace, fSpace);
    
    for (SizeType i = 0; i < numChannels; ++i) {
      for (SizeType j = 0; j < numSamples; ++j) {
        dataOut[i][j] = buffer[j * numChannels + i];
      }
    }
    delete[] buffer;
    REQUIRE_THAT(dataOut[0], Catch::Matchers::Approx(mockData[0]).margin(1));
    REQUIRE_THAT(dataOut[1], Catch::Matchers::Approx(mockData[1]).margin(1));
  }

  SECTION("test samples recorded tracking") {
    // setup io object
    std::string path = getTestFilePath("ElectricalSeriesSampleTracking.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // setup electrode table
    std::string elecTablePath =  "/electrodes/";
    NWB::ElectrodeTable elecTable = NWB::ElectrodeTable(elecTablePath, io);
    elecTable.initialize();

    // setup electrical series
    NWB::ElectricalSeries es = NWB::ElectricalSeries(dataPath, io, dataType, timestampsType, mockArrays[0], elecTable.getPath(), "volts", "no description", "no comments",
        SizeArray{0, mockArrays[0].size()});
    es.initialize();

    // write channel data in segments
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      SizeType samplesRecorded = 0;
      for (SizeType b = 0; b * bufferSize < numSamples; b += 1) {
        // copy chunk of data 
        std::copy(mockData[ch].begin() + samplesRecorded,
                  mockData[ch].begin() + samplesRecorded + bufferSize,
                  dataBuffer.begin());
        std::copy(mockTimestamps.begin() + samplesRecorded,
          mockTimestamps.begin() + samplesRecorded + bufferSize,
          timestampsBuffer.begin());

        es.writeChannel(ch, dataBuffer.size(), dataBuffer.data(), timestampsBuffer.data());
        samplesRecorded += bufferSize;
      }
    }
    io->close();

    // Read data back from file
    std::unique_ptr<H5::H5File> file =
        std::make_unique<H5::H5File>(path, H5F_ACC_RDONLY);
    std::unique_ptr<H5::DataSet> dataset =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath + "/data"));
    std::vector<std::vector<float>> dataOut(numChannels,
                                            std::vector<float>(numSamples));
    float* buffer = new float[numSamples * numChannels];

    H5::DataSpace fSpace = dataset->getSpace();
    hsize_t dims[1] = {numSamples * numChannels};
    H5::DataSpace mSpace(1, dims);
    dataset->read(buffer, H5::PredType::NATIVE_FLOAT, mSpace, fSpace);
    
    for (SizeType i = 0; i < numChannels; ++i) {
      for (SizeType j = 0; j < numSamples; ++j) {
        dataOut[i][j] = buffer[j * numChannels + i];
      }
    }
    delete[] buffer;
    REQUIRE_THAT(dataOut[0], Catch::Matchers::Approx(mockData[0]).margin(1));
    REQUIRE_THAT(dataOut[1], Catch::Matchers::Approx(mockData[1]).margin(1));
  }
}
