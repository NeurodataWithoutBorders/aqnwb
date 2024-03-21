
#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "io/BaseIO.hpp"
#include "io/HDF5IO.hpp"
#include "file/ElectrodeTable.hpp"
#include "NWBFile.hpp"

namespace fs = std::filesystem;

std::string getTestFilePath(std::string filename)
{
  // create data directory if it doesn't exist
  fs::path dirPath = fs::current_path() / "data";
  fs::directory_entry dir(dirPath);
  if (!dir.exists()) {
    fs::create_directory(dir);
  }

  // get filename and remove old file
  fs::path filepath = dirPath / filename;
  if (fs::exists(filepath)) {
    fs::remove(filepath);
  }

  return filepath.u8string();
}

TEST_CASE("writeAttributes", "[hdf5io]")
{
  // create and open file
  std::string filename = getTestFilePath("test_attributes.h5");
  HDF5IO hdf5io(filename);
  hdf5io.open();

  hdf5io.createGroup("/data");

  // single attribute
  SECTION("single_value")
  {
    const signed int data = 1;
    hdf5io.createAttribute(
        AQNWBIO::BaseDataType::I32, &data, "/data", "single_value");
  }

  // integer array
  SECTION("int_array")
  {
    const int data[] = {1, 2, 3, 4, 5};
    const int dataSize = sizeof(data) / sizeof(data[0]);

    hdf5io.createAttribute(
        AQNWBIO::BaseDataType::I32, &data, "/data", "array", dataSize);
  }

  // string array
  SECTION("str_array")
  {
    const std::vector<std::string> data = {"col1", "col2", "col3"};

    hdf5io.createAttribute(data, "/data", "string_array");
  }

  // soft link
  SECTION("link")
  {
    std::vector<std::string> data;
    hdf5io.createLink("/data/link", "linked_data");
  }

  // close file
  hdf5io.close();
}

TEST_CASE("saveNWBFile", "[nwb]")
{
  std::string filename = getTestFilePath("test_nwb_file.h5");

  NWBFile nwbfile("123", std::make_unique<HDF5IO>(filename));
  nwbfile.initialize();
  nwbfile.finalize();
}

TEST_CASE("startRecording", "[nwb]")
{
  std::string filename = getTestFilePath("test_recording.h5");

  NWBFile nwbfile("123", std::make_unique<HDF5IO>(filename));
  nwbfile.initialize();
  bool result = nwbfile.startRecording();
  nwbfile.finalize();

  REQUIRE(result == true);
}

TEST_CASE("ElectrodeTable", "[datatypes]")
{
  std::string path = "/electrodes/";
  SECTION("initialize with example data")
  {
    std::string filename = getTestFilePath("electrodeTable.h5");
    std::vector<int> channels = {1, 2, 3};
    std::shared_ptr<BaseIO> io = std::make_unique<HDF5IO>(filename);
    io->open();
    io->createGroup("array1");
    ElectrodeTable electrodeTable(path, io, channels);
    electrodeTable.setGroupPath("array1");
    electrodeTable.electrodeDataset->dataset =
      std::unique_ptr<BaseRecordingData>(
        io->createDataSet(BaseDataType::I32, std::vector<size_t>{1}, std::vector<size_t>{1}, path + "id"));

    electrodeTable.locationsDataset->dataset =
      std::unique_ptr<BaseRecordingData>(
        io->createDataSet(BaseDataType::STR(250), std::vector<size_t>{0}, std::vector<size_t>{1}, path + "location"));
    electrodeTable.initialize();

    BaseRecordingData* data = io->getDataSet(path + "id");
    int* buffer = new int[3];
    static_cast<HDF5RecordingData*>(data)->readDataBlock(BaseDataType::I32, buffer);
    std::vector<int> read_channels(buffer, buffer + 3);
    delete[] buffer;

    // Don't forget to delete the buffer when you're done with it
    REQUIRE(channels == read_channels);
    // Check if the groupReferences, groupNames, electrodeNumbers, and
    // locationNames vectors are populated correctly
    // REQUIRE(electrodeTable.getColumn("location")
    //         == std::vector<std::string> {"unknown", "unknown", "unknown"};
    // REQUIRE(electrodeTable.getColNames()
    //         == std::vector<std::string> {"group", "group_name", "location"});
  }

  SECTION("initialize without empty channels")
  {
    std::string filename = getTestFilePath("electrodeTableNoData.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<HDF5IO>(filename);
    io->open();
    ElectrodeTable electrodeTable(path, io, std::vector<int>(), "none");
    electrodeTable.initialize();

    // Check if the groupReferences, groupNames, electrodeNumbers, and
    // locationNames vectors are empty
    // REQUIRE(electrodeTable.getColumn("groupReferences").empty());
    // REQUIRE(electrodeTable.getColumn("group_name").empty());
  }
}
