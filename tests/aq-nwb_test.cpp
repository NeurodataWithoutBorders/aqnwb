
#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "BaseIO.hpp"
#include "HDF5IO.hpp"
#include "NWBDataTypes.hpp"
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
    hdf5io.setAttribute(
        AQNWBIO::BaseDataType::I32, &data, "/data", "single_value");
  }

  // integer array
  SECTION("int_array")
  {
    const int data[] = {1, 2, 3, 4, 5};
    const int dataSize = sizeof(data) / sizeof(data[0]);

    hdf5io.setAttribute(
        AQNWBIO::BaseDataType::I32, &data, "/data", "array", dataSize);
  }

  // string array
  SECTION("str_array")
  {
    const std::vector<std::string> data = {"col1", "col2", "col3"};

    hdf5io.setAttribute(data, "/data", "string_array");
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
    std::shared_ptr<BaseIO> io = std::make_unique<HDF5IO>(filename);
    io->open();
    ElectrodeTable electrodeTable(path, io);
    io->createGroup("array1");

    std::vector<int> channels = {1, 2, 3};
    electrodeTable.channels = channels;
    electrodeTable.groupPath = "array1";
    electrodeTable.electrodeDataset->dataset =
        std::unique_ptr<BaseRecordingData>(
            io->createDataSet(BaseDataType::I32, 1, 1, path + "id"));
    electrodeTable.locationsDataset->dataset =
        std::unique_ptr<BaseRecordingData>(
            io->createDataSet(BaseDataType::STR(250), 0, 1, path + "location"));
    electrodeTable.initialize();

    // Check if the groupReferences, groupNames, electrodeNumbers, and
    // locationNames vectors are populated correctly
    REQUIRE(electrodeTable.locationNames
            == std::vector<std::string> {"unknown", "unknown", "unknown"});
    REQUIRE(electrodeTable.electrodeNumbers == std::vector<int> {1, 2, 3});
    REQUIRE(electrodeTable.getColNames()
            == std::vector<std::string> {"group", "group_name", "location"});
  }

  SECTION("initialize without data")
  {
    std::string filename = getTestFilePath("electrodeTableNoData.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<HDF5IO>(filename);
    io->open();
    ElectrodeTable electrodeTable(path, io);
    electrodeTable.initialize();

    // Check if the groupReferences, groupNames, electrodeNumbers, and
    // locationNames vectors are empty
    REQUIRE(electrodeTable.groupReferences.empty());
    REQUIRE(electrodeTable.groupNames.empty());
    REQUIRE(electrodeTable.electrodeNumbers.empty());
  }
}