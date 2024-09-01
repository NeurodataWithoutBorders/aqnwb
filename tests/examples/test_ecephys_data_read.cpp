#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "BaseIO.hpp"
#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("ElectricalSeriesReadExample", "[ecephys]")
{
  SECTION("ecephys data read example")
  {
    // setup mock data for writing
    SizeType numSamples = 100;
    SizeType numChannels = 2;
    std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays();
    BaseDataType dataType = BaseDataType::F32;
    std::vector<std::vector<float>> mockData =
        getMockData2D(numSamples, numChannels);
    std::vector<double> mockTimestamps = getMockTimestamps(numSamples, 1);
    // To verify that the data was written correctly, we here transpose the
    // mockData (which is per channel) to the (time x channel) layout used
    // in the ElectricalSeries in the NWB file so we can compare
    std::vector<std::vector<float>> mockDataTransposed;
    mockDataTransposed.resize(numSamples);
    for (SizeType s = 0; s < numSamples; s++) {
      mockDataTransposed[s].resize(numChannels);
      for (SizeType c = 0; c < numChannels; c++) {
        mockDataTransposed[s][c] = mockData[c][s];
      }
    }

    // setup io object
    std::string path = getTestFilePath("ElectricalSeriesReadExample.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);

    // setup the NWBFile
    NWB::NWBFile nwbfile(io);
    nwbfile.initialize(generateUuid());

    // TODO Use RecordingContainers in this example once #84 is merged

    // create a new ElectricalSeries
    Status resultCreate = nwbfile.createElectricalSeries(mockArrays, dataType);
    REQUIRE(resultCreate == Status::Success);

    // get the new ElectricalSeries
    NWB::ElectricalSeries* electricalSeries =
        dynamic_cast<NWB::ElectricalSeries*>(nwbfile.getTimeSeries(0));
    REQUIRE(electricalSeries != nullptr);

    // start recording
    Status resultStart = nwbfile.startRecording();
    REQUIRE(resultStart == Status::Success);

    // write channel data
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      electricalSeries->writeChannel(
          ch, numSamples, mockData[ch].data(), mockTimestamps.data());
    }
    io->flush();

    // Illustrate reading the ElecticalSeries.data back
    std::string electricalSeriesDataPath = electricalSeries->dataPath();
    std::string electricalSeriesPath = electricalSeries->getPath();
    REQUIRE(electricalSeriesDataPath == (electricalSeriesPath + "/data"));

    // Get a ReadDataseWrapper for lazy reading of ElectricalSeries.data
    auto readDataWrapper = electricalSeries->dataLazy();

    // Read the full  ElectricalSeries.data back
    DataBlock<float> dataValues = readDataWrapper->values<float>();

    // Check that the data we read has the expected size and shape
    REQUIRE(dataValues.data.size() == (numSamples * numChannels));
    REQUIRE(dataValues.shape[0] == numSamples);
    REQUIRE(dataValues.shape[1] == numChannels);

    // Iterate through all the time steps
    for (SizeType t = 0; t < numSamples; t++) {
      // Get the data for the single time step t from the DataBlock
      std::vector<float> selectedRange(
          dataValues.data.begin() + (t * numChannels),
          dataValues.data.begin() + ((t + 1) * numChannels));
      // Check that the values are correct
      REQUIRE_THAT(selectedRange,
                   Catch::Matchers::Approx(mockDataTransposed[t]).margin(1));
    }
    // Use the boost multi-array feature to simply interaction with data
    // Create a 2D boost::const_multi_array_ref<float, 2> multidimensional array
    auto boostMulitArray = dataValues.as_multi_array<2>();
    // Iterate through all the time steps again, but now using the boost array
    for (SizeType t = 0; t < numSamples; t++) {
      // Access [t, :], i.e., get a 1D array with the data
      // from all channels for time step t.
      auto row_t = boostMulitArray[t];

      // Compare to check that the data is correct.
      std::vector<float> row_t_vector(
          row_t.begin(), row_t.end());  // convert to std::vector for comparison
      REQUIRE_THAT(row_t_vector,
                   Catch::Matchers::Approx(mockDataTransposed[t]).margin(1));
    }

    // Stop the recording
    io->stopRecording();
    io->close();

    // Open an I/O for reading
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);

    // To read from a specific TimeSeries we can just create it directly from
    // the read IO Using NWB::Container::create we can create any NWB Container
    // class. Alternatively, we can also construct the ElectricalSeries object
    // ourselves
    auto readElectricalSeries =
        NWB::Container::create<NWB::ElectricalSeries>(path, io);

    // Now we can read the data in the same way we did during write
    auto readElectricalSeriesData = electricalSeries->dataLazy();

    // TODO Actually loading the data causes a segfault
    // DataBlock<float> readDataValues = readDataWrapper->values<float>();
    // auto readBoostMulitArray = readDataValues.as_multi_array<2>();

    // Let's read the first 10 timesteps for the first two channels
    /*DataBlock<float> dataSlice = readDataWrapper->values<float>(
        {0, 0},  // start
        {100, 1}  // count
    );
    REQUIRE(dataSlice.shape[0] == 100);
    REQUIRE(dataSlice.shape[1] == 1);*/

    // TODO Add an attribute getter (e.g., for ElectricalSeries.unit) and test
    // that it works as well std::string esUnit = electricalSeries->unit();
    // REQUIRE(esUnit == std::string("volts"));
    // REQUIRE(unitValueGeneric.shape.size() == 0);

    // TODO Check why reading the unit causes the below Assertion in HDF5IO to
    // fail: (this->file->attrExists(dataPath)), function readAttribute, file
    // HDF5IO.cpp, line 161. i.e., the attribute does not seem to exists so
    // either the path is wrong or something is bad with the write?
  }
}
