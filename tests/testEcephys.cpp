#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodeTable.hpp"
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
  std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays();
  std::string dataPath = "/esdata";
  BaseDataType dataType = BaseDataType::F32;
  std::vector<std::vector<float>> mockData =
      getMockData2D(numSamples, numChannels);
  std::vector<double> mockTimestamps = getMockTimestamps(numSamples, 1);
  std::string devicePath = "/device";
  std::string electrodePath = "/elecgroup/";

  SECTION("test initialization")
  {
    // TODO
  }

  SECTION("test linking to electrode table region")
  {
    // TODO
  }

  SECTION("test writing channels")
  {
    // setup io object
    std::string path = getTestFilePath("ElectricalSeries.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");

    // setup electrode table, device, and electrode group
    NWB::ElectrodeTable elecTable = NWB::ElectrodeTable(io);
    elecTable.initialize();

    // setup electrical series
    NWB::ElectricalSeries es = NWB::ElectricalSeries(dataPath, io);
    es.initialize(dataType,
                  mockArrays[0],
                  "no description",
                  SizeArray {0, mockArrays[0].size()},
                  SizeArray {1, 1});

    // write channel data
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      es.writeChannel(
          ch, numSamples, mockData[ch].data(), mockTimestamps.data());
    }

    io->flush();

    // TODO START  Move this check to a proper test and expand on it
    std::string esdataPath = es.dataPath();
    std::string expectedDataPath = dataPath + "/data";
    REQUIRE(esdataPath == expectedDataPath);

    auto readDataWrapper = es.dataLazy();
    DataBlock<float> dataValues = readDataWrapper->values<float>();
    REQUIRE(dataValues.data.size() == (numSamples * numChannels));
    REQUIRE(dataValues.shape[0] == numSamples);
    REQUIRE(dataValues.shape[1] == numChannels);
    for (SizeType s = 0; s < numSamples; s++) {
      std::vector<float> temp;
      temp.resize(numChannels);
      for (SizeType c = 0; c < numChannels; c++) {
        temp[c] = mockData[c][s];
      }
      std::vector<float> selectedRange(
          dataValues.data.begin() + (s * numChannels),
          dataValues.data.begin() + ((s + 1) * numChannels));
      REQUIRE_THAT(selectedRange, Catch::Matchers::Approx(temp).margin(1));
    }
    // Check the boost multi-array feature
    auto boostMulitArray = dataValues.as_multi_array<2>();
    for (SizeType s = 0; s < numSamples; s++) {
      // Get timestamp s (i.e., the values from all channels at time index s)
      // Accessing [a, :]
      auto row_s = boostMulitArray[s];  // This returns a 1D array representing
                                        // the first row
      // Convert the row from boost to a std:vector
      std::vector<float> row_s_vector(row_s.begin(), row_s.end());

      // Data for comparison
      std::vector<float> temp;
      temp.resize(numChannels);
      for (SizeType c = 0; c < numChannels; c++) {
        temp[c] = mockData[c][s];
      }
      REQUIRE_THAT(row_s_vector, Catch::Matchers::Approx(temp).margin(1));
    }

    // std::cout<<"Data shape:" << dataValues.shape.size() << std::endl;

    //
    // REQUIRE_THAT(readDataValues, Catch::Matchers::Approx(data).margin(1));

    // TODO Add an attribute getter and test that it works as well
    // auto dataLazyWrapper = es.dataLazy();
    // auto dataValueGeneric = dataLazyWrapper->valuesGeneric();
    // TODO Check why this causes Assertion failed:
    // (this->file->attrExists(dataPath)), function readAttribute, file
    // HDF5IO.cpp, line 161.
    //      the attribute does not seem to exists so either the path is wrong or
    //      something is bad with the write?
    // REQUIRE(unitValueGeneric.shape.size() == 0);

    // std::string esUnit = es.unit();
    // REQUIRE(esUnit == std::string("volts"));
    //  TODO END Move this code

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

  SECTION("test samples recorded tracking")
  {
    // setup io object
    std::string path = getTestFilePath("ElectricalSeriesSampleTracking.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");

    // setup electrode table
    NWB::ElectrodeTable elecTable = NWB::ElectrodeTable(io);
    elecTable.initialize();

    // setup electrical series
    NWB::ElectricalSeries es = NWB::ElectricalSeries(dataPath, io);
    es.initialize(dataType,
                  mockArrays[0],
                  "no description",
                  SizeArray {0, mockArrays[0].size()},
                  SizeArray {1, 1});

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

        es.writeChannel(
            ch, dataBuffer.size(), dataBuffer.data(), timestampsBuffer.data());
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
