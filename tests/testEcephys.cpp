#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodesTable.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("registered ecephys types", "[ecephys]")
{
  auto registry = AQNWB::NWB::RegisteredType::getRegistry();
  REQUIRE(registry.find("core::Device") != registry.end());
  REQUIRE(registry.find("core::ElectrodeGroup") != registry.end());
  REQUIRE(registry.find("core::ElectrodesTable") != registry.end());
  REQUIRE(registry.find("core::ElectricalSeries") != registry.end());
  REQUIRE(registry.find("core::SpikeEventSeries") != registry.end());
}

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
  std::string electrodePath =
      "/general/extracellular_ephys/" + mockArrays[0][0].getGroupName();

  SECTION("test writing channels")
  {
    // setup io object
    std::string path = getTestFilePath("ElectricalSeries.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");

    // setup device and electrode group
    auto device = NWB::Device(devicePath, io);
    device.initialize("description", "unknown");
    auto elecGroup = NWB::ElectrodeGroup(electrodePath, io);
    elecGroup.initialize("description", "unknown", device);

    // setup electrode table, device, and electrode group
    NWB::ElectrodesTable elecTable = NWB::ElectrodesTable(io);
    Status elecTableStatus = elecTable.initialize();
    REQUIRE(elecTableStatus == Status::Success);
    elecTable.addElectrodes(mockArrays[0]);
    elecTableStatus = elecTable.finalize();
    REQUIRE(elecTableStatus == Status::Success);

    // Confirm that the electrode table is created correctly
    auto readColNames = elecTable.readColNames()->values().data;
    std::vector<std::string> expectedColNames = {
        "location", "group", "group_name"};
    REQUIRE(readColNames == expectedColNames);

    // setup electrical series
    NWB::ElectricalSeries es = NWB::ElectricalSeries(dataPath, io);
    IO::ArrayDataSetConfig config(
        dataType, SizeArray {0, mockArrays[0].size()}, SizeArray {1, 1});
    es.initialize(config, mockArrays[0], "no description");

    // write channel data
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      es.writeChannel(
          ch, numSamples, mockData[ch].data(), mockTimestamps.data());
    }
    io->flush();
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

    // setup device and electrode group
    auto device = NWB::Device(devicePath, io);
    device.initialize("description", "unknown");
    auto elecGroup = NWB::ElectrodeGroup(electrodePath, io);
    elecGroup.initialize("description", "unknown", device);

    // setup electrode table
    NWB::ElectrodesTable elecTable = NWB::ElectrodesTable(io);
    Status elecTableStatus = elecTable.initialize();
    REQUIRE(elecTableStatus == Status::Success);
    elecTable.addElectrodes(mockArrays[0]);
    elecTableStatus = elecTable.finalize();
    REQUIRE(elecTableStatus == Status::Success);

    // Confirm that the electrode table is created correctly
    auto readColNames = elecTable.readColNames()->values().data;
    std::vector<std::string> expectedColNames = {
        "location", "group", "group_name"};
    REQUIRE(readColNames == expectedColNames);

    // setup electrical series
    NWB::ElectricalSeries es = NWB::ElectricalSeries(dataPath, io);
    IO::ArrayDataSetConfig config(
        dataType, SizeArray {0, mockArrays[0].size()}, SizeArray {1, 1});
    es.initialize(config, mockArrays[0], "no description");

    // write channel data in segments
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      SizeType samplesRecorded = 0;
      for (SizeType b = 0; b * bufferSize < numSamples; b += 1) {
        // copy chunk of data
        std::copy(
            mockData[ch].begin() + static_cast<std::ptrdiff_t>(samplesRecorded),
            mockData[ch].begin()
                + static_cast<std::ptrdiff_t>(samplesRecorded + bufferSize),
            dataBuffer.begin());
        std::copy(
            mockTimestamps.begin()
                + static_cast<std::ptrdiff_t>(samplesRecorded),
            mockTimestamps.begin()
                + static_cast<std::ptrdiff_t>(samplesRecorded + bufferSize),
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

  SECTION("test writing electrodes")
  {
    std::vector<Types::ChannelVector> mockArraysElectrodes =
        getMockChannelArrays(4);

    // setup io object
    std::string path = getTestFilePath("ElectricalSeriesElectrodes.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");

    // setup device and electrode group
    auto device = NWB::Device(devicePath, io);
    device.initialize("description", "unknown");
    auto elecGroup = NWB::ElectrodeGroup(electrodePath, io);
    elecGroup.initialize("description", "unknown", device);

    // setup electrode table
    NWB::ElectrodesTable elecTable = NWB::ElectrodesTable(io);
    elecTable.initialize();
    elecTable.addElectrodes(mockArraysElectrodes[0]);
    elecTable.finalize();

    // setup electrical series
    NWB::ElectricalSeries es = NWB::ElectricalSeries(dataPath, io);
    IO::ArrayDataSetConfig config(BaseDataType::F32,
                                  SizeArray {0, mockArrays[0].size()},
                                  SizeArray {1, 1});
    es.initialize(config, mockArraysElectrodes[0], "no description");
    io->close();

    // // read the data back in
    io = createIO("HDF5", path);
    io->open();

    // Verify electrodes dataset exists and contains correct data
    auto readElectricalSeries =
        NWB::RegisteredType::create<NWB::ElectricalSeries>(dataPath, io);
    auto readElectrodesWrapper = readElectricalSeries->readElectrodes();
    auto readElectrodesValues = readElectrodesWrapper->values();
    for (size_t i = 0; i < mockArraysElectrodes[0].size(); ++i) {
      REQUIRE(static_cast<SizeType>(readElectrodesValues.data[i])
              == mockArraysElectrodes[0][i].getGlobalIndex());
    }

    // Verify dataset attributes
    auto readElectrodesDescriptionWrapper =
        readElectricalSeries->readElectrodesDescription();
    auto readElectrodesDescriptionValues =
        readElectrodesDescriptionWrapper->values().data[0];
    REQUIRE(readElectrodesDescriptionValues
            == "the electrodes that generated this electrical series");

    // Read the references to the ElectrodesTable
    auto readElectrodesTable = readElectricalSeries->readElectrodesTable();
    REQUIRE(readElectrodesTable != nullptr);
    REQUIRE(readElectrodesTable->getPath()
            == AQNWB::NWB::ElectrodesTable::electrodesTablePath);
  }

  SECTION("test reading electrodes")
  {
    std::vector<Types::ChannelVector> mockArraysElectrodes =
        getMockChannelArrays(4);

    // setup io object
    std::string path = getTestFilePath("ElectrodesTableRead.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");

    // setup device and electrode group
    auto device = NWB::Device(devicePath, io);
    device.initialize("description", "unknown");
    auto elecGroup = NWB::ElectrodeGroup(electrodePath, io);
    elecGroup.initialize("description", "unknown", device);

    // setup electrode table
    NWB::ElectrodesTable elecTable = NWB::ElectrodesTable(io);
    elecTable.initialize();
    elecTable.addElectrodes(mockArraysElectrodes[0]);
    elecTable.finalize();

    // // read the data back in
    io = createIO("HDF5", path);
    io->open();

    // Confirm the typename in the file
    AQNWB::IO::DataBlockGeneric typeData = io->readAttribute(AQNWB::mergePaths(
        AQNWB::NWB::ElectrodesTable::electrodesTablePath, "neurodata_type"));
    auto typeBlock = AQNWB::IO::DataBlock<std::string>::fromGeneric(typeData);
    std::string typeName = typeBlock.data[0];
    REQUIRE(typeName == "ElectrodesTable");

    // Read using the RegisteredType::create where we infer the type from the
    // file This should result in a `ElectrodesTable` object
    std::string electrodesTableTypeName2 = io->getFullTypeNameFromFile(
        AQNWB::NWB::ElectrodesTable::electrodesTablePath);
    REQUIRE(electrodesTableTypeName2 == "core::ElectrodesTable");
    auto readElectrodesTable2 = AQNWB::NWB::RegisteredType::create(
        AQNWB::NWB::ElectrodesTable::electrodesTablePath, io);
    REQUIRE(readElectrodesTable2->getFullTypeName() == "core::ElectrodesTable");
    auto readElectrodesTable2Cast =
        std::dynamic_pointer_cast<AQNWB::NWB::ElectrodesTable>(
            readElectrodesTable2);
    REQUIRE(readElectrodesTable2Cast != nullptr);

    // Testing backward compatibility of ElectrodesTable with NWB <=2.8
    // To test for older files, we modify the neurodata_type attribute for our
    // ElectrodesTable to be DynamicTable instead
    io->createAttribute("DynamicTable",
                        AQNWB::NWB::ElectrodesTable::electrodesTablePath,
                        "neurodata_type",
                        true);
    // read to confirm the overwrite worked
    typeData = io->readAttribute(AQNWB::mergePaths(
        AQNWB::NWB::ElectrodesTable::electrodesTablePath, "neurodata_type"));
    auto typeBlock2 = AQNWB::IO::DataBlock<std::string>::fromGeneric(typeData);
    typeName = typeBlock2.data[0];
    REQUIRE(typeName == "DynamicTable");

    // Ensure the mapping of the typename in the I/O works
    std::string electrodesTableTypeName3 = io->getFullTypeNameFromFile(
        AQNWB::NWB::ElectrodesTable::electrodesTablePath);
    REQUIRE(electrodesTableTypeName2 == "core::ElectrodesTable");

    // Ensure that reading with ElectrodesTable type directly still works as
    // expected
    auto readElectrodesTable4 =
        AQNWB::NWB::RegisteredType::create<AQNWB::NWB::ElectrodesTable>(
            AQNWB::NWB::ElectrodesTable::electrodesTablePath, io);
    REQUIRE(readElectrodesTable4 != nullptr);
    REQUIRE(readElectrodesTable4->getFullTypeName() == "core::ElectrodesTable");

    // Confirm that reading with the generic approach where the type is being
    // read from the file, also still works. I.e., confirm that the remapping to
    // the ElectrodesTable type is working as expected
    auto readElectrodesTable5 = AQNWB::NWB::RegisteredType::create(
        AQNWB::NWB::ElectrodesTable::electrodesTablePath, io);
    REQUIRE(readElectrodesTable5 != nullptr);
    REQUIRE(readElectrodesTable5->getFullTypeName() == "core::ElectrodesTable");
    auto readElectrodesTable5_cast =
        std::dynamic_pointer_cast<AQNWB::NWB::ElectrodesTable>(
            readElectrodesTable5);
    REQUIRE(readElectrodesTable5_cast != nullptr);
  }
}

TEST_CASE("SpikeEventSeries", "[ecephys]")
{
  // setup recording info
  SizeType numSamples = 32;
  SizeType numEvents = 10;
  std::string dataPath = "/sesdata";
  BaseDataType dataType = BaseDataType::F32;
  std::vector<double> mockTimestamps = getMockTimestamps(numEvents, 1);
  std::string devicePath = "/device";

  SECTION("test writing events - events x channels x samples")
  {
    // setup mock data
    SizeType numChannels = 4;
    std::vector<Types::ChannelVector> mockArrays =
        getMockChannelArrays(numChannels);
    std::vector<std::vector<float>> mockData =
        getMockData2D(numSamples * numChannels, numEvents);
    std::string electrodePath =
        "/general/extracellular_ephys/" + mockArrays[0][0].getGroupName();

    // setup io object
    std::string path = getTestFilePath("SpikeEventSeries3D.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");

    // setup device and electrode group
    auto device = NWB::Device(devicePath, io);
    device.initialize("description", "unknown");
    auto elecGroup = NWB::ElectrodeGroup(electrodePath, io);
    elecGroup.initialize("description", "unknown", device);

    // setup electrode table, device, and electrode group
    NWB::ElectrodesTable elecTable = NWB::ElectrodesTable(io);
    Status elecTableStatus = elecTable.initialize();
    REQUIRE(elecTableStatus == Status::Success);
    elecTable.addElectrodes(mockArrays[0]);
    elecTableStatus = elecTable.finalize();
    REQUIRE(elecTableStatus == Status::Success);

    // setup electrical series
    NWB::SpikeEventSeries ses = NWB::SpikeEventSeries(dataPath, io);
    IO::ArrayDataSetConfig config(
        dataType, SizeArray {0, numChannels, numSamples}, SizeArray {8, 1, 1});
    ses.initialize(config, mockArrays[0], "no description");

    // write channel data
    for (SizeType e = 0; e < numEvents; ++e) {
      double timestamp = mockTimestamps[e];
      ses.writeSpike(numSamples, numChannels, mockData[e].data(), &timestamp);
    }
    io->close();

    // Read data back from file
    std::unique_ptr<H5::H5File> file =
        std::make_unique<H5::H5File>(path, H5F_ACC_RDONLY);
    std::unique_ptr<H5::DataSet> dataset =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath + "/data"));
    std::vector<std::vector<float>> dataOut(
        numEvents, std::vector<float>(numSamples * numChannels));
    float* buffer = new float[numEvents * numSamples * numChannels];

    H5::DataSpace fSpace = dataset->getSpace();
    hsize_t dims[3];
    fSpace.getSimpleExtentDims(dims, NULL);
    dataset->read(buffer, H5::PredType::NATIVE_FLOAT, fSpace, fSpace);

    for (SizeType i = 0; i < numEvents; ++i) {
      for (SizeType j = 0; j < (numSamples * numChannels); ++j) {
        dataOut[i][j] = buffer[i * (numSamples * numChannels) + j];
      }
    }
    delete[] buffer;
    REQUIRE_THAT(dataOut[0], Catch::Matchers::Approx(mockData[0]).margin(1));
    REQUIRE_THAT(dataOut[1], Catch::Matchers::Approx(mockData[1]).margin(1));
  }

  SECTION("test writing events - events x samples")
  {
    // setup mock data
    std::vector<Types::ChannelVector> mockArrays = getMockChannelArrays(1);
    std::vector<std::vector<float>> mockData =
        getMockData2D(numSamples, numEvents);
    std::string electrodePath =
        "/general/extracellular_ephys/" + mockArrays[0][0].getGroupName();

    // setup io object
    std::string path = getTestFilePath("SpikeEventSeries2D.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    io->createGroup("/general");
    io->createGroup("/general/extracellular_ephys");

    // setup device and electrode group
    auto device = NWB::Device(devicePath, io);
    device.initialize("description", "unknown");
    auto elecGroup = NWB::ElectrodeGroup(electrodePath, io);
    elecGroup.initialize("description", "unknown", device);

    // setup electrode table, device, and electrode group
    NWB::ElectrodesTable elecTable = NWB::ElectrodesTable(io);
    Status elecTableStatus = elecTable.initialize();
    REQUIRE(elecTableStatus == Status::Success);
    elecTable.addElectrodes(mockArrays[0]);
    elecTableStatus = elecTable.finalize();
    REQUIRE(elecTableStatus == Status::Success);

    // setup electrical series
    NWB::SpikeEventSeries ses = NWB::SpikeEventSeries(dataPath, io);
    IO::ArrayDataSetConfig config(
        dataType, SizeArray {0, numSamples}, SizeArray {8, 1});
    ses.initialize(config, mockArrays[0], "no description");

    // write channel data
    for (SizeType e = 0; e < numEvents; ++e) {
      double timestamp = mockTimestamps[e];
      ses.writeSpike(numSamples, 1, mockData[e].data(), &timestamp);
    }
    io->close();

    // Read data back from file
    std::unique_ptr<H5::H5File> file =
        std::make_unique<H5::H5File>(path, H5F_ACC_RDONLY);
    std::unique_ptr<H5::DataSet> dataset =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath + "/data"));
    std::vector<std::vector<float>> dataOut(numEvents,
                                            std::vector<float>(numSamples));
    float* buffer = new float[numEvents * numSamples];

    H5::DataSpace fSpace = dataset->getSpace();
    hsize_t dims[3];
    fSpace.getSimpleExtentDims(dims, NULL);
    dataset->read(buffer, H5::PredType::NATIVE_FLOAT, fSpace, fSpace);

    for (SizeType i = 0; i < numEvents; ++i) {
      for (SizeType j = 0; j < (numSamples); ++j) {
        dataOut[i][j] = buffer[i * (numSamples) + j];
      }
    }
    delete[] buffer;
    REQUIRE_THAT(dataOut[0], Catch::Matchers::Approx(mockData[0]).margin(1));
    REQUIRE_THAT(dataOut[1], Catch::Matchers::Approx(mockData[1]).margin(1));
  }
}
