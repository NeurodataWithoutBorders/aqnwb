#include <numeric>
#include <variant>

// Use mdspan if available (C++23 or with library support)
#if defined(__cpp_lib_mdspan)
#  include <mdspan>
#endif

#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/RecordingObjects.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodesTable.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

// [example_compute_mean_from_variant]
// Helper function to compute the mean of a vector
template<typename T>
inline double compute_mean(const T& data)
{
  if (data.empty()) {
    throw std::runtime_error("Data vector is empty");
  }
  double sum = std::accumulate(data.begin(), data.end(), 0.0);
  return sum / static_cast<double>(data.size());
}

// Function to compute the mean using std::visit
inline double compute_mean_variant(
    const BaseDataType::BaseDataVectorVariant& variant)
{
  return std::visit(
      [](auto&& arg) -> double
      {
        using T = std::decay_t<decltype(arg)>;
        // Check that the variant represents a BaseDataType we can compute on
        if constexpr (std::is_same_v<T, std::monostate>) {
          throw std::runtime_error("Invalid data type");
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
          throw std::runtime_error("Cannot compute mean of string data");
        } else {
          return compute_mean(arg);  // Compute the mean
        }
      },
      variant);
}
// [example_compute_mean_from_variant]

TEST_CASE("ElectricalSeriesReadExample", "[ecephys]")
{
  SECTION("ecephys data read example")
  {
    std::cout << "Creating the mock data for the ElectricalSeriesReadExample"
              << std::endl;
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

    std::cout << "Creating the ElectricalSeriesReadExample.h5 file"
              << std::endl;
    // [example_read_create_file_snippet]
    // setup io object
    std::string path = getTestFilePath("ElectricalSeriesReadExample.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();

    // setup the NWBFile
    auto nwbfile = NWB::NWBFile::create(io);
    Status initStatus = nwbfile->initialize(generateUuid());
    REQUIRE(initStatus == Status::Success);

    // RecordingObjects are now automatically managed by the IO object

    // create a new ElectricalSeries
    auto electrodesTable = nwbfile->createElectrodesTable(mockArrays);
    std::vector<SizeType> containerIndexes = {};
    Status resultCreate = nwbfile->createElectricalSeries(
        mockArrays, mockChannelNames, dataType, containerIndexes);
    REQUIRE(resultCreate == Status::Success);

    // get the new ElectricalSeries
    auto recordingObjects = io->getRecordingObjects();
    auto registeredTypePtr =
        recordingObjects->getRecordingObject(containerIndexes[0]);
    REQUIRE(registeredTypePtr != nullptr);
    REQUIRE(registeredTypePtr->getFullTypeName() == "core::ElectricalSeries");
    auto electricalSeries =
        std::dynamic_pointer_cast<NWB::ElectricalSeries>(registeredTypePtr);
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

    std::cout
        << "Reading the ElectricalSeriesReadExample.h5 file via the write I/O"
        << std::endl;
    // [example_read_get_data_wrapper_snippet]
    // Get a ReadDatasetWrapper<float> for lazy reading of ElectricalSeries.data
    // By specifying the value type as a template parameter allows us to read
    // typed data. However, in the particular case of ElectricalSeries.data, we
    // could also have used readData() with <float> as the template parameter
    // is already set to float by default for ElectricalSeries.readData()
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
          dataValues.data.begin()
              + static_cast<std::vector<float>::difference_type>(t
                                                                 * numChannels),
          dataValues.data.begin()
              + static_cast<std::vector<float>::difference_type>(
                  (t + 1) * numChannels));
      // Check that the values are correct
      REQUIRE_THAT(selectedRange,
                   Catch::Matchers::Approx(mockDataTransposed[t]).margin(1));
    }
// [example_read_validate_datablock_snippet]

// [example_read_get_array_view_snippet]
// Use the multi-array view to simplify interaction with data
#if defined(__cpp_lib_mdspan)
    // Use std::mdspan for multi-dimensional access if available
    // Use std::mdspan for multi-dimensional access if available
    float* data_ptr = dataValues.data.data();
    std::mdspan<float, std::dextents<size_t, 2>> dataView(
        data_ptr, dataValues.shape[0], dataValues.shape[1]);
#else
    // Pre-C++23: use as_multi_array for multi-dimensional access
    auto dataView = dataValues.as_multi_array<2>();
#endif
    // [example_read_get_array_view_snippet]

    // [example_read_validate_array_view_snippet]
    // Iterate through all the time steps again, but now using the view
    for (SizeType t = 0; t < numSamples; t++) {
#if defined(__cpp_lib_mdspan)
      // Use mdspan call operator for multi-dimensional access
      std::vector<float> row_t_vector;
      for (SizeType c = 0; c < numChannels; ++c) {
        row_t_vector.push_back(dataView(t, c));
      }
#else
      // Pre-C++23: use as_multi_array row access
      auto row_t = dataView[static_cast<SizeType>(t)];
      std::vector<float> row_t_vector(row_t.begin(), row_t.end());
#endif
      // Compare to check that the data is correct.
      REQUIRE_THAT(row_t_vector,
                   Catch::Matchers::Approx(mockDataTransposed[t]).margin(1));
    }
    // [example_read_validate_array_view_snippet]

    // [example_read_attribute_snippet]
    // Get a ReadDataWrapper<ReadObjectType::Attribute, float> to read data
    // lazily
    auto readDataResolutionWrapper = electricalSeries->readDataResolution();
    // Read the data values as a DataBlock<float>
    auto resolutionValueFloat = readDataResolutionWrapper->values();
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

    std::cout << "Closing the write I/O" << std::endl;
    // [example_read_finish_recording_snippet]
    // Stop the recording
    io->flush();
    io->stopRecording();
    io->close();
    // [example_read_finish_recording_snippet]

    std::cout << "Reading the ElectricalSeriesReadExample.h5 file via a new I/O"
              << std::endl;
    // [example_read_new_io_snippet]
    // Open a new I/O for reading
    std::shared_ptr<BaseIO> readio = createIO("HDF5", path);
    readio->open(FileMode::ReadOnly);
    // [example_read_new_io_snippet]

    // [example_read_predefined_types]
    // Read the NWBFile
    auto readNWBFile =
        NWB::RegisteredType::create<AQNWB::NWB::NWBFile>("/", readio);
    // Read the ElectrodesTable
    auto readElectrodesTable = readNWBFile->readElectrodesTable();
    // read the location data. Note that both the type of the class and
    // the data values is being set for us, here, VectorDataTyped<std::string>
    auto locationColumn = readElectrodesTable->readLocationColumn();
    auto locationColumnValues = locationColumn->readData()->values();
    // confirm that the values are correct
    std::vector<std::string> expectedLocationValues = {
        "unknown", "unknown", "unknown", "unknown"};
    REQUIRE(locationColumnValues.data == expectedLocationValues);
    // [example_read_predefined_types]

    std::cout << "Searching and reading the ElectricalSeries container"
              << std::endl;
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
    // Print the path and type of the found objects
    for (const auto& pair : found_electrical_series) {
      std::cout << "Path=" << pair.first << " Full type=" << pair.second
                << std::endl;
    }
    // [example_search_types_check_snippet]

    std::cout << "Reading the ElectricalSeries container " << std::endl;
    // [example_read_only_snippet]
    // Read the ElectricalSeries from the file.
    std::string esdata_path = "/acquisition/esdata0";
    auto readElectricalSeries =
        NWB::RegisteredType::create<AQNWB::NWB::ElectricalSeries>(esdata_path,
                                                                  readio);
    // [example_read_only_snippet]

    std::cout << "Reading the ElectricalSeries data" << std::endl;
    // [example_read_only_fields_snippet]
    // Now we can read the data in the same way we did during write
    auto readElectricalSeriesData = readElectricalSeries->readData();
    auto readDataValues = readElectricalSeriesData->values();
    REQUIRE(readDataValues.data.size() == (numSamples * numChannels));
    REQUIRE(readDataValues.shape[0] == numSamples);
    REQUIRE(readDataValues.shape[1] == numChannels);
    // Use the multi-array view to simplify interaction with multi-dimensional
    // data
#if defined(__cpp_lib_mdspan)
    // Use std::mdspan for multi-dimensional access if available
    float* read_data_ptr = readDataValues.data.data();
    std::mdspan<float, std::dextents<size_t, 2>> readDataView(
        read_data_ptr, readDataValues.shape[0], readDataValues.shape[1]);
    REQUIRE(readDataView.extent(0) == numSamples);
    REQUIRE(readDataView.extent(1) == numChannels);
#else
    // Pre-C++23: use as_multi_array for multi-dimensional access
    auto readDataView = readDataValues.as_multi_array<2>();
    REQUIRE(readDataView.shape()[0] == numSamples);
    REQUIRE(readDataView.shape()[1] == numChannels);
#endif
    // [example_read_only_fields_snippet]

    std::cout << "Reading a subset of the ElectricalSeries data" << std::endl;
    // [example_read_only_datasubset_snippet]
    // We can also read just subsets of the data, e.g., the first 10 time steps
    // for the first channel. "auto dataSlice" is again of type DataBlock<float>
    std::vector<SizeType> start = {0, 0};
    std::vector<SizeType> count = {9, 1};
    auto dataSlice = readElectricalSeriesData->values(start, count);
    // Validate that the slice was read correctly
    REQUIRE(dataSlice.data.size() == 9);
    REQUIRE(dataSlice.shape[0] == 9);
    REQUIRE(dataSlice.shape[1] == 1);
    // [example_read_only_datasubset_snippet]

    std::cout << "Reading the ElectricalSeries unit attribute" << std::endl;
    // [example_read_only_stringattr_snippet]
    // Or read a string attribute, e.g., the unit
    std::string esUnitValue =
        readElectricalSeries->readDataUnit()->values().data[0];
    REQUIRE(esUnitValue == std::string("volts"));
    // [example_read_only_stringattr_snippet]

    // [example_read_generic_dataset_field_snippet]
    // Read the data field via the generic readField method
    auto readElectricalSeriesData3 =
        readElectricalSeries->readField<StorageObjectType::Dataset, float>(
            std::string("data"));
    // Read the data values as usual
    auto readDataValues3 = readElectricalSeriesData3->values();
    REQUIRE(readDataValues3.data.size() == (numSamples * numChannels));
    // [example_read_generic_dataset_field_snippet]

    // [example_read_generic_registeredtype_field_snippet]
    // read the ElectricalSeries from the NWBFile object via the readField
    // method returning a generic std::shared_ptr<RegisteredType>
    auto readRegisteredType = readNWBFile->readField(esdata_path);
    // cast the generic pointer to the more specific ElectricalSeries
    std::shared_ptr<AQNWB::NWB::ElectricalSeries> readElectricalSeries2 =
        std::dynamic_pointer_cast<AQNWB::NWB::ElectricalSeries>(
            readRegisteredType);
    REQUIRE(readElectricalSeries2 != nullptr);
    // [example_read_generic_registeredtype_field_snippet]

    // [example_use_std_variant_to_compute_on_data]
    // Compute the mean using the std::variant approach. We specify
    // the types of variables for clarity, but could us "auto" instead
    DataBlockGeneric genericDataBlock =
        readElectricalSeriesData->valuesGeneric();
    BaseDataType::BaseDataVectorVariant variantData =
        genericDataBlock.as_variant();
    double meanFromVariant = compute_mean_variant(variantData);
    // Compare with computing the mean from the typed DataBlock<float>. We
    // specify the template type for clarity although the compiler can infer it.
    double meanFromTypedVector =
        compute_mean<std::vector<float>>(readDataValues.data);
    REQUIRE(meanFromVariant == Catch::Approx(meanFromTypedVector));
    // [example_use_std_variant_to_compute_on_data]

    // Close the io
    readio->close();
  }
}
