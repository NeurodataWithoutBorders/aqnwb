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
    // Index = 0; Type = core::NWBFile; Path = /;  -- cache size: 0
    // Index = 1; Type = hdmf-common::ElementIdentifiers; Path =
    // /general/extracellular_ephys/electrodes/id;  -- cache size: 1 Index = 2;
    // Type = hdmf-common::VectorData; Path =
    // /general/extracellular_ephys/electrodes/group_name;  -- cache size: 1
    // Index = 3; Type = hdmf-common::VectorData; Path =
    // /general/extracellular_ephys/electrodes/location;  -- cache size: 1 Index
    // = 4; Type = core::DynamicTable; Path =
    // /general/extracellular_ephys/electrodes;  -- cache size: 0 Index = 5;
    // Type = core::Device; Path = /general/devices/array0;  -- cache size: 0
    // Index = 6; Type = core::ElectrodeGroup; Path =
    // /general/extracellular_ephys/array0;  -- cache size: 0 Index = 7; Type =
    // core::Device; Path = /general/devices/array1;  -- cache size: 0 Index =
    // 8; Type = core::ElectrodeGroup; Path =
    // /general/extracellular_ephys/array1;  -- cache size: 0 Index = 9; Type =
    // hdmf-common::VectorData; Path =
    // /general/extracellular_ephys/electrodes/group;  -- cache size: 0
    SizeType expectedNumRecordingObjects = 10;
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
    // Index = 10; Type = core::ElectricalSeries; Path = /acquisition/esdata0;
    // -- cache size: 2 Index = 11; Type = core::ElectricalSeries; Path =
    // /acquisition/esdata1;  -- cache size: 2
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

    // test that lookup by path works as expected
    for (SizeType i = 0; i < recordingObjects->size(); ++i) {
      auto obj = recordingObjects->getRecordingObject(i);
      auto obj2 = recordingObjects->getRecordingObject(obj->getPath());
      REQUIRE(obj2 != nullptr);
      REQUIRE(obj2 == obj);
      SizeType idx = recordingObjects->getRecordingIndex(obj2);
      REQUIRE(idx == i);
    }

    // test that lookup by object works as expected
    for (SizeType i = 0; i < recordingObjects->size(); ++i) {
      auto obj = recordingObjects->getRecordingObject(i);
      SizeType idx = recordingObjects->getRecordingIndex(obj);
      REQUIRE(idx == i);  // should return the correct index
      REQUIRE(obj->isRegisteredRecordingObject() == true);
      REQUIRE(obj->getRecordingObjectIndex() == i);
    }

    // test that the toString() method works
    std::string recObjStr = recordingObjects->toString();
    REQUIRE(recObjStr.find("Index = 0; Type = core::NWBFile; Path = /;")
            != std::string::npos);
    REQUIRE(recObjStr.find("Index = 11; Type = core::ElectricalSeries; Path = "
                           "/acquisition/esdata1;")
            != std::string::npos);

    // out‑of‑range lookup returns nullptr
    REQUIRE(recordingObjects->getRecordingObject(99) == nullptr);

    // now read an existing series and confirm that we get back the already
    // registered object
    auto existingSeries =
        NWB::ElectricalSeries::create("/acquisition/esdata0", io);
    expectedNumRecordingObjects += 1;  // we created a new object
    REQUIRE(existingSeries != nullptr);
    REQUIRE(existingSeries->isRegisteredRecordingObject() == true);
    REQUIRE(recordingObjects->size() == expectedNumRecordingObjects);
    // NOTE: since we created a new ElectricalSeries object (instead of reusing
    // the existing one), we now have 2 objects for the same path
    SizeType existingIdx = recordingObjects->getRecordingIndex(existingSeries);
    REQUIRE(existingIdx == 12);  // should be the same as before
    // Since existingSeries exists in the file, requesting recordData() should
    // yield the same object that is already in the cache
    auto existingRecordData = existingSeries->recordData();
    REQUIRE(existingRecordData != nullptr);
    REQUIRE(existingSeries->isRegisteredRecordingObject() == true);

    // recordingObjects should now hold:
    // RecordingObjects contents:
    // Index = 0; Type = core::NWBFile; Path = /;
    // Index = 1; Type = hdmf-common::ElementIdentifiers; Path =
    // /general/extracellular_ephys/electrodes/id; Index = 2; Type =
    // hdmf-common::VectorData; Path =
    // /general/extracellular_ephys/electrodes/group_name; Index = 3; Type =
    // hdmf-common::VectorData; Path =
    // /general/extracellular_ephys/electrodes/location; Index = 4; Type =
    // core::DynamicTable; Path = /general/extracellular_ephys/electrodes; Index
    // = 5; Type = core::Device; Path = /general/devices/array0; Index = 6; Type
    // = core::ElectrodeGroup; Path = /general/extracellular_ephys/array0; Index
    // = 7; Type = core::Device; Path = /general/devices/array1; Index = 8; Type
    // = core::ElectrodeGroup; Path = /general/extracellular_ephys/array1; Index
    // = 9; Type = hdmf-common::VectorData; Path =
    // /general/extracellular_ephys/electrodes/group; Index = 10; Type =
    // core::ElectricalSeries; Path = /acquisition/esdata0; Index = 11; Type =
    // core::ElectricalSeries; Path = /acquisition/esdata1; Index = 12; Type =
    // core::ElectricalSeries; Path = /acquisition/esdata0;

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
    REQUIRE(recordingObjects->size() == 12);  // see previous section

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
    REQUIRE(recordingObjects->size() == 12);
    std::vector<SizeType> expextedCacheSize = {
        0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 2, 2};
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
    REQUIRE(recordingObjects->size() == 13);

    REQUIRE(recordingObjects->clearRecordingDataCache() == Status::Failure);

    // Clear the recordingObjects and check it's cleared
    recordingObjects->clear();
    REQUIRE(recordingObjects->size() == 0);

    // cleanup
    io->close();
  }
}
