#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RecordingContainers.hpp"
#include "nwb/RegisteredType.hpp"
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
    // [example_read_mockdata_snippet]
    // setup mock data for writing
    SizeType numSamples = 100;
    SizeType numChannels = 2;
    std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays();
    BaseDataType dataType = BaseDataType::F32;
    std::vector<std::string> mockChannelNames =
        getMockChannelArrayNames("esdata");
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
    // [example_read_mockdata_snippet]

    // [example_read_create_file_snippet]
    // setup io object
    std::string path = getTestFilePath("ElectricalSeriesReadExample.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);

    // setup the NWBFile
    NWB::NWBFile nwbfile(io);
    nwbfile.initialize(generateUuid());

    // create the RecordingContainer for managing recordings
    std::unique_ptr<NWB::RecordingContainers> recordingContainers =
        std::make_unique<NWB::RecordingContainers>();

    // create a new ElectricalSeries
    Status resultCreate = nwbfile.createElectricalSeries(
        mockArrays, mockChannelNames, dataType, recordingContainers.get());
    REQUIRE(resultCreate == Status::Success);

    // get the new ElectricalSeries
    NWB::ElectricalSeries* electricalSeries =
        static_cast<NWB::ElectricalSeries*>(
            recordingContainers->getContainer(0));
    REQUIRE(electricalSeries != nullptr);

    // start recording
    Status resultStart = io->startRecording();
    REQUIRE(resultStart == Status::Success);

    // write channel data
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      electricalSeries->writeChannel(
          ch, numSamples, mockData[ch].data(), mockTimestamps.data());
    }
    io->flush();
    // [example_read_create_file_snippet]

    // [example_read_get_data_wrapper_snippet]
    // Get a ReadDatasetWrapper<float> for lazy reading of ElectricalSeries.data
    // By specifying the value type as a template parameter allows us to read
    // typed data
    auto readDataWrapper = electricalSeries->readData<float>();
    // [example_read_get_data_wrapper_snippet]

    // [example_read_check_data_exists_snippet]
    REQUIRE(readDataWrapper->exists());
    // [example_read_check_data_exists_snippet]

    // [example_read_get_datablock_snippet]
    // Read the full  ElectricalSeries.data back
    DataBlock<float> dataValues = readDataWrapper->values();
    // [example_read_get_datablock_snippet]

    // [example_read_validate_datablock_snippet]
    // Check that the data we read has the expected size and shape
    REQUIRE(dataValues.data.size() == (numSamples * numChannels));
    REQUIRE(dataValues.shape[0] == numSamples);
    REQUIRE(dataValues.shape[1] == numChannels);
    REQUIRE(dataValues.typeIndex == typeid(float));

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
    // [example_read_validate_datablock_snippet]

    // [example_read_get_boostarray_snippet]
    // Use the boost multi-array feature to simply interaction with data
    auto boostMulitArray = dataValues.as_multi_array<2>();
    // [example_read_get_boostarray_snippet]

    // [example_read_validate_boostarray_snippet]
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
    // [example_read_validate_boostarray_snippet]

    // [example_read_attribute_snippet]
    // Get a ReadDataWrapper<ReadObjectType::Attribute, float> to read data
    // lazily
    auto readDataResolutionWrapper = electricalSeries->readDataResolution();
    // Read the data values
    DataBlock<float> resolutionValueFloat = readDataResolutionWrapper->values();
    REQUIRE(resolutionValueFloat.shape.empty());  // Scalar
    REQUIRE(resolutionValueFloat.data.size() == 1);
    REQUIRE(int(resolutionValueFloat.data[0]) == -1);
    REQUIRE(resolutionValueFloat.typeIndex == typeid(float));
    // [example_read_attribute_snippet]

    // [example_read_get_data_wrapper_as_generic_snippet]
    // Get a generic ReadDatasetWrapper<std::any> for lazy reading of
    // ElectricalSeries.data
    auto readDataWrapperGeneric = electricalSeries->readData();
    // Instead of using values() to read typed data, we can read data as generic
    // data first via valuesGeneric
    DataBlockGeneric dataValuesGeneric =
        readDataWrapperGeneric->valuesGeneric();
    // Note that the I/O backend determines the data type and allocates
    // the memory for us. The std::type_index is stored in our data block as
    // well
    REQUIRE(dataValuesGeneric.typeIndex == typeid(float));
    // We can then later convert the data block to a typed data block
    DataBlock<float> dataValueFloat =
        DataBlock<float>::fromGeneric(dataValuesGeneric);
    // [example_read_get_data_wrapper_as_generic_snippet]

    // [example_read_getpath_snippet]
    // Reading the ElectricalSeries.data back (during the recording)
    std::string electricalSeriesDataPath = readDataWrapperGeneric->getPath();
    std::string electricalSeriesPath = electricalSeries->getPath();
    REQUIRE(electricalSeriesDataPath == (electricalSeriesPath + "/data"));
    // [example_read_getpath_snippet]

    // [example_read_finish_recording_snippet]
    // Stop the recording
    io->stopRecording();
    io->close();
    // [example_read_finish_recording_snippet]

    // Open a new I/O for reading
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open();

    // [example_search_types_snippet]
    std::unordered_set<std::string> typesToSearch = {"core::ElectricalSeries"};
    std::unordered_map<std::string, std::string> found_electrical_series =
        readio->findTypes(
            "/",  // start search at the root of the file
            typesToSearch,  // search for all ElectricalSeries
            IO::SearchMode::CONTINUE_ON_TYPE  // search also within types
        );
    
    // [example_search_types_snippet]
    // [example_search_types_check_snippet]
    // We should have esdata1 and esdata2
    REQUIRE(found_electrical_series.size() == 2);
    std::string esdata_path;
    // Print the path and type of the found objects
    for (const auto& pair : found_electrical_series) {
      std::cout << "Path=" << pair.first << " Full type=" << pair.second
                << std::endl;
      esdata_path = pair.first;
    }
    // [example_search_types_check_snippet]

    // [example_read_only_snippet]
    // Read the ElectricalSeries from the file. This returns a generic
    // std::unique_ptr<AQNWB::NWB::RegisteredType>
    auto readRegisteredType = NWB::RegisteredType::create(esdata_path, readio);
    // If we need operations that are specific for the ElectricalSeries,
    // then we can cast the returned pointer via
    auto readElectricalSeries =
        std::dynamic_pointer_cast<AQNWB::NWB::ElectricalSeries>(
            readRegisteredType);

    // Now we can read the data in the same way we did during write
    auto readElectricalSeriesData = readElectricalSeries->readData<float>();
    DataBlock<float> readDataValues = readElectricalSeriesData->values();
    auto readBoostMultiArray = readDataValues.as_multi_array<2>();
    // [example_read_only_snippet]

    // Test that reading a string attribute works
    auto esDescr = readElectricalSeries->readDescription();
    auto esDescrData = esDescr->values();
    REQUIRE(esDescrData.data.size() == 1);
    REQUIRE(esDescrData.shape.size() == 0);
    REQUIRE(esDescrData.data[0]
            == std::string("Stores continuously sampled voltage data from an "
                           "extracellular ephys recording"));

    // TODO Slicing doesn't seem to work quite yet. The full data is loaded
    // instead. Let's read the first 10 timesteps for the first two channels
    /*DataBlock<float> dataSlice = readDataWrapper->values<float>(
        {0, 0},  // start
        {10, 1}  // count
    );
    REQUIRE(dataSlice.data.size() == 10);
    REQUIRE(dataSlice.shape[0] == 10);
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
