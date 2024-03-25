
#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "NWBFile.hpp"
#include "file/ElectrodeTable.hpp"
#include "io/BaseIO.hpp"
#include "io/HDF5IO.hpp"

using namespace AQNWBIO;

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
  Status result = nwbfile.startRecording();
  nwbfile.finalize();

  REQUIRE(result == Status::Success);
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
        std::unique_ptr<BaseRecordingData>(io->createDataSet(
            BaseDataType::I32, SizeArray {1}, SizeArray {1}, path + "id"));

    electrodeTable.locationsDataset->dataset =
        std::unique_ptr<BaseRecordingData>(
            io->createDataSet(BaseDataType::STR(250),
                              SizeArray {0},
                              SizeArray {1},
                              path + "location"));
    electrodeTable.initialize();

    // Check if id datasets are created correctly
    size_t numChannels = 3;
    BaseRecordingData* id_data = io->getDataSet(path + "id");
    int* buffer = new int[numChannels];
    static_cast<HDF5RecordingData*>(id_data)->readDataBlock(BaseDataType::I32,
                                                            buffer);
    std::vector<int> read_channels(buffer, buffer + numChannels);
    delete[] buffer;
    REQUIRE(channels == read_channels);
  }

  SECTION("initialize without empty channels")
  {
    std::string filename = getTestFilePath("electrodeTableNoData.h5");
    std::shared_ptr<BaseIO> io = std::make_unique<HDF5IO>(filename);
    io->open();
    ElectrodeTable electrodeTable(path, io, std::vector<int>(), "none");
    electrodeTable.initialize();
  }
}
