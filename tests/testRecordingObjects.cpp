//==========================================================================
//  RecordingObjectsTests.cpp
//  Unit‑tests for AQNWB::IO::RecordingObjects
//  (uses the normal NWB recording workflow)
//==========================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/RecordingObjects.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "io/nwbio_utils.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "testUtils.hpp"  // getTestFilePath(), createIO(),
// getMockChannelArrays(),
// getMockChannelArrayNames(),
// getMockData2D(), getMockTimestamps(),
// generateUuid()

using namespace AQNWB;

// -------------------------------------------------------------------------
//   Helper RegisteredType subclasses that deliberately misbehaves – they are
//   still *TimeSeries*  objects, so they can be inserted into a
//   RecordingObjects collection.
//   -------------------------------------------------------------------------
namespace
{
class FaultyTimeSeries : public NWB::TimeSeries
{
public:
  FaultyTimeSeries(const std::string& p,
                   const std::shared_ptr<AQNWB::IO::BaseIO>& io)
      : TimeSeries(p, io)
  {
  }

  // make finalize always fail
  Status finalize() override { return Status::Failure; }

  // a no‑op clear (the default implementation would be fine)
  void clearRecordingDataCache() override {}
};

class ExceptionThrowingSeries : public NWB::TimeSeries
{
public:
  ExceptionThrowingSeries(const std::string& p,
                          const std::shared_ptr<AQNWB::IO::BaseIO>& io)
      : TimeSeries(p, io)
  {
  }

  Status finalize() override { return Status::Success; }

  // throw when the RecordingObjects wrapper asks us to clear the cache
  void clearRecordingDataCache() override
  {
    throw std::runtime_error("simulated clearRecordingDataCache failure");
  }
};
}  // anonymous namespace

// -------------------------------------------------------------------------
//   Test suite
// -------------------------------------------------------------------------
TEST_CASE("RecordingObjects recording workflow tests", "[recording]")
{
  using AQNWB::createIO;
  using AQNWB::IO::BaseIO;
  using AQNWB::IO::RecordingObjects;

  // --------------------------------------------------------------
  //   SECTION 1 – basic collection semantics (add, duplicate, lookup)
  //   --------------------------------------------------------------
  SECTION("add, duplicate handling and lookup")
  {
    const std::string path = getTestFilePath("recObjectsWorkflow_1.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    REQUIRE(io->isOpen());

    // ---- get the container that the IO object manages -----------------
    auto recordingObjects = io->getRecordingObjects();
    REQUIRE(recordingObjects != nullptr);
    REQUIRE(recordingObjects->size() == 0);

    // ---- create a minimal NWB file ------------------------------------
    auto nwbFile = AQNWB::NWB::NWBFile::create(io);
    REQUIRE(nwbFile != nullptr);
    Status s = nwbFile->initialize(generateUuid());
    REQUIRE(s == Status::Success);
    REQUIRE(recordingObjects->size() == 1);  // the NWBFile itself

    // ---- electrodes ----------------------------------------------------
    auto mockRecordingArrays = getMockChannelArrays();  // default 4 channels
    auto electrodesTable = nwbFile->createElectrodesTable(mockRecordingArrays);
    REQUIRE(electrodesTable != nullptr);

    // recordingObjects should hold:
    // obj[0] = core::NWBFile: /
    // obj[1] = core::DynamicTable: /general/extracellular_ephys/electrodes
    // obj[2] = hdmf-common::ElementIdentifiers:
    // /general/extracellular_ephys/electrodes/id obj[3] =
    // hdmf-common::VectorData:
    // /general/extracellular_ephys/electrodes/group_name obj[4] =
    // hdmf-common::VectorData: /general/extracellular_ephys/electrodes/location
    // obj[5] = core::Device: /general/devices/array0
    // obj[6] = core::ElectrodeGroup: /general/extracellular_ephys/array0
    // obj[7] = core::Device: /general/devices/array1
    // obj[8] = core::ElectrodeGroup: /general/extracellular_ephys/array1
    SizeType expectedNumRecordingObjects = 9;
    REQUIRE(recordingObjects->size() == expectedNumRecordingObjects);

    // ---- a single ElectricalSeries ------------------------------------
    std::vector<std::size_t> containerIndexes;
    std::vector<std::string> mockChannelNames =
        getMockChannelArrayNames("esdata");

    s = nwbFile->createElectricalSeries(mockRecordingArrays,
                                        mockChannelNames,
                                        BaseDataType::I16,
                                        containerIndexes);
    REQUIRE(s == Status::Success);
    REQUIRE(containerIndexes.size() == mockChannelNames.size());
    // Added the following recording objects:
    // obj[9] = core::ElectricalSeries: /acquisition/esdata0
    // obj[10] = core::ElectricalSeries: /acquisition/esdata1
    expectedNumRecordingObjects += mockChannelNames.size();
    REQUIRE(recordingObjects->size() == expectedNumRecordingObjects);

    // Test that lookup by index works as expected
    for (SizeType i = 0; i < recordingObjects->size(); ++i) {
      auto obj = recordingObjects->getRecordingObject(i);
      REQUIRE(obj != nullptr);
      REQUIRE(recordingObjects->getRecordingIndex(obj) == i);
      REQUIRE(obj->isRegisteredRecordingObject() == true);
      REQUIRE(obj->getRecordingObjectIndex() == i);
    }

    // duplicate insertion – should return the existing index and not grow
    for (SizeType i = 0; i < recordingObjects->size(); ++i) {
      auto obj = recordingObjects->getRecordingObject(i);
      SizeType dupIdx = recordingObjects->addRecordingObject(obj);
      REQUIRE(dupIdx == i);
      REQUIRE(recordingObjects->size() == expectedNumRecordingObjects);
    }

    // out‑of‑range lookup returns nullptr
    REQUIRE(recordingObjects->getRecordingObject(99) == nullptr);

    // lookup of a never‑added object yields the sentinel value
    auto freshSeries = NWB::ElectricalSeries::create("/freshSeries", io);
    // object is not registered yet because we have not called initialize()
    // nor requested a BaseRecordingData objects, e.g, via
    // freshSeries->recordData()
    REQUIRE(recordingObjects->getRecordingIndex(freshSeries)
            == AQNWB::Types::SizeTypeNotSet);
    // Since freshSeries is not written, requesting recordData() should not
    // register it since retrieving the BaseRecordingData object fails
    auto recordData = freshSeries->recordData();
    REQUIRE(recordData == nullptr);
    REQUIRE(recordingObjects->size() == expectedNumRecordingObjects);
    REQUIRE(freshSeries->isRegisteredRecordingObject() == false);
    SizeType freshIdx = recordingObjects->getRecordingIndex(freshSeries);
    REQUIRE(freshIdx == AQNWB::Types::SizeTypeNotSet);

    // now read an existing series and confirm that it does not get registered
    // at first but gets registered when recordData() is called
    auto existingSeries =
        NWB::ElectricalSeries::create("/acquisition/esdata0", io);
    REQUIRE(existingSeries != nullptr);
    REQUIRE(existingSeries->isRegisteredRecordingObject() == false);
    SizeType existingIdx = recordingObjects->getRecordingIndex(existingSeries);
    REQUIRE(existingIdx == AQNWB::Types::SizeTypeNotSet);
    // Since existingSeries exists in the file, requesting recordData() should
    // register it
    auto existingRecordData = existingSeries->recordData();
    REQUIRE(existingRecordData != nullptr);
    REQUIRE(existingSeries->isRegisteredRecordingObject() == true);

    // recordingObjects should now hold:
    // obj[0] = core::NWBFile: /
    // obj[1] = core::DynamicTable: /general/extracellular_ephys/electrodes
    // obj[2] = hdmf-common::ElementIdentifiers:
    // /general/extracellular_ephys/electrodes/id obj[3] =
    // hdmf-common::VectorData:
    // /general/extracellular_ephys/electrodes/group_name obj[4] =
    // hdmf-common::VectorData: /general/extracellular_ephys/electrodes/location
    // obj[5] = core::Device: /general/devices/array0
    // obj[6] = core::ElectrodeGroup: /general/extracellular_ephys/array0
    // obj[7] = core::Device: /general/devices/array1
    // obj[8] = core::ElectrodeGroup: /general/extracellular_ephys/array1
    // obj[9] = core::ElectricalSeries: /acquisition/esdata0
    // obj[10] = core::ElectricalSeries: /acquisition/esdata1
    // obj[11] = core::ElectricalSeries: /acquisition/esdata0 <-- duplicate
    // because we read the series
    expectedNumRecordingObjects += 1;
    existingIdx = recordingObjects->getRecordingIndex(existingSeries);
    REQUIRE(existingIdx != AQNWB::Types::SizeTypeNotSet);
    REQUIRE(existingIdx == recordingObjects->size() - 1);
    REQUIRE(recordingObjects->size() == expectedNumRecordingObjects);

    // cleanup
    io->close();
  }

  // --------------------------------------------------------------
  //  SECTION 2 – finalize aggregates status (including a failing series)
  //   --------------------------------------------------------------
  SECTION("finalize aggregates status")
  {
    const std::string path = getTestFilePath("recObjectsWorkflow_2.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    REQUIRE(io->isOpen());
    auto recordingObjects = io->getRecordingObjects();
    REQUIRE(recordingObjects != nullptr);
    REQUIRE(recordingObjects->size() == 0);

    // normal workflow – one good ElectricalSeries
    auto nwbFile = NWB::NWBFile::create(io);
    nwbFile->initialize(generateUuid());

    auto mockArrays = getMockChannelArrays();
    auto mockNames = getMockChannelArrayNames("esdata");
    nwbFile->createElectrodesTable(mockArrays);

    std::vector<std::size_t> contIdx;
    nwbFile->createElectricalSeries(
        mockArrays, mockNames, BaseDataType::I16, contIdx);
    REQUIRE(contIdx.size() == mockNames.size());
    REQUIRE(recordingObjects->size()
            == 11);  // see previous section for details

    // Repeated calls to finalize should succeed without error
    for (SizeType i = 0; i < 4; i++) {
      auto finalizeStatus = recordingObjects->finalize();
      REQUIRE(finalizeStatus == Status::Success);
    }

    // inject a deliberately failing object
    auto bad = std::make_shared<FaultyTimeSeries>("/bad_series", io);
    recordingObjects->addRecordingObject(bad);

    // any single failure flips the overall status to Failure
    REQUIRE(recordingObjects->finalize() == Status::Failure);

    io->close();
  }

  // --------------------------------------------------------------
  //   SECTION 3 – clearRecordingDataCache aggregates & tolerates exceptions
  //   --------------------------------------------------------------
  SECTION("clearRecordingDataCache aggregates and tolerates exceptions")
  {
    const std::string path = getTestFilePath("recObjectsWorkflow_3.h5");
    std::shared_ptr<BaseIO> io = createIO("HDF5", path);
    io->open();
    REQUIRE(io->isOpen());
    auto recordingObjects = io->getRecordingObjects();
    REQUIRE(recordingObjects != nullptr);
    REQUIRE(recordingObjects->size() == 0);

    // good series (created through the normal workflow)
    auto nwbFile = NWB::NWBFile::create(io);
    nwbFile->initialize(generateUuid());
    auto mockArrays = getMockChannelArrays();
    nwbFile->createElectrodesTable(mockArrays);
    std::vector<std::size_t> contIdx;
    nwbFile->createElectricalSeries(mockArrays,
                                    getMockChannelArrayNames("esdata"),
                                    BaseDataType::I16,
                                    contIdx);
    REQUIRE(recordingObjects->size() == 11);
    std::vector<SizeType> expextedCacheSize = {0, 0, 1, 1, 1, 0, 0, 0, 0, 2, 2};
    // Confirm that the recording data caches are as expected
    for (SizeType i = 0; i < recordingObjects->size(); ++i) {
      auto obj = recordingObjects->getRecordingObject(i);
      REQUIRE(obj != nullptr);
      std::cout << "obj[" << i << "] = " << obj->getFullTypeName() << ": "
                << obj->getPath()
                << " -- cache size: " << obj->getCacheRecordingData().size()
                << std::endl;
      REQUIRE(obj->getCacheRecordingData().size() <= expextedCacheSize[i]);
    }
    // clear the BaseRecordingData caches – should succeed
    Status clearCachesStatus = recordingObjects->clearRecordingDataCache();
    REQUIRE(clearCachesStatus == Status::Success);
    // Confirm that the recording data caches are cleared
    for (SizeType i = 0; i < recordingObjects->size(); ++i) {
      auto obj = recordingObjects->getRecordingObject(i);
      REQUIRE(obj != nullptr);
      REQUIRE(obj->getCacheRecordingData().size() == 0);
    }

    // add a series that throws when clearing its cache
    auto throwing = std::make_shared<ExceptionThrowingSeries>("/throwing", io);
    recordingObjects->addRecordingObject(throwing);
    REQUIRE(recordingObjects->size() == 12);

    REQUIRE(recordingObjects->clearRecordingDataCache() == Status::Failure);

    // Clear the recordingObjects and check it's cleared
    recordingObjects->clear();
    REQUIRE(recordingObjects->size() == 0);

    // cleanup
    io->close();
  }
}
