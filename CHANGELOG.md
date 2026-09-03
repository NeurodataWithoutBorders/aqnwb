# Changelog 

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).



## Upcoming [0.4.0] (~August 2026)

### Added
* **Added support for streaming data read of remote NWB files:**
   * Added `HDF5IO::openS3(...)` method to support opening an existing remote file in S3 in read-only mode using the HDF5 ROS3 driver. (@oruebel, [#307](https://github.com/NeurodataWithoutBorders/aqnwb/pull/307))
   * Added `HDF5IO::openRemote()` method to read remote NWB files over HTTP(S) using the [remfile-cpp](https://github.com/catalystneuro/remfile-cpp) virtual file driver (a C++ port of the Python [remfile](https://github.com/magland/remfile) package), imported as an optional CMake dependency (`AQNWB_USE_REMFILE`, requires `libcurl`). Unlike ROS3, remfile does not require HDF5 to be built with ROS3 support and works with any HTTP(S) server that supports byte-range requests. (@bendichter, [#309](https://github.com/NeurodataWithoutBorders/aqnwb/pull/309))
   * Added new `BaseIO::findObject` and `RegisteredType::findOwnedObject` methods to simplify searching for objects by name. Added `HDF5IO::findObject` override method to optimize the search for HDF5 objects. (@oruebel, [#308](https://github.com/NeurodataWithoutBorders/aqnwb/pull/308))  
   * Added demo for benchmarking ROS3 and remfile performance and comparing with PyNWB S3 reads (`demo/remote_read_benchmark`). (@oruebel, [#308](https://github.com/NeurodataWithoutBorders/aqnwb/pull/308); @bendichter, [#309](https://github.com/NeurodataWithoutBorders/aqnwb/pull/309))
   * Added tutorial on using the ROS3 and remfile drivers to read NWB files in S3 (`docs/pages/userdocs/reads3.dox`) (@oruebel, [#308](https://github.com/NeurodataWithoutBorders/aqnwb/pull/308); @bendichter, [#309](https://github.com/NeurodataWithoutBorders/aqnwb/pull/309))
* **Added support for NWB schema 2.11 and EventTable, TimeIntervals, MeaningsTable, VectorIndex, and Subject:**
   * Updated schema in the `spec/` module to the latest NWB releases: (i) NWB 2.11, (ii) HDMF Common 1.10, and (iii) HDMF Experimental 0.6 (@oruebel, [#300](https://github.com/NeurodataWithoutBorders/aqnwb/pull/300))
   * Added `TimestampVectorData` and `DurationVectorData` types to support event timing data. (@cline, @oruebel, [#301](https://github.com/NeurodataWithoutBorders/aqnwb/pull/301))
   * Updated `DynamicTable::readColumn` using type traits to support reading both `VectorData` columns with a specific data type and columns that are a subtype of `VectorData` (e.g., the new `TimestampVectorData` and `DurationVectorData`). (@cline, @oruebel, [#301](https://github.com/NeurodataWithoutBorders/aqnwb/pull/301))
   * Added new `EventsTable` type to support recording of event data and updated `NWBFile` to `NWBFile::initialize` to create the new top-level `events` group and add create and read methods to support events (e.g., `NWBFile::createEventsTable`). (@cline, @oruebel, [#304](https://github.com/NeurodataWithoutBorders/aqnwb/pull/304))
   * Added `TimeIntervals` type for annotating time intervals and updated `NWBFile` to support the standard `intervals` tables (e.g., `trials`, `invalid_times`, `epochs`). (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Added new `MeaningsTable` type to support adding meanings to `DynamicTable` columns. (@cline, @oruebel, [#302](https://github.com/NeurodataWithoutBorders/aqnwb/pull/302))
   * Added `DynamicTable::createMeaningsTable` and `DynamicTable::readMeaningsTable` methods to support creating and reading `MeaningsTable` objects associated with a `DynamicTable`. (@cline, @oruebel, [#302](https://github.com/NeurodataWithoutBorders/aqnwb/pull/302))
   * Added tutorials for the new `MeaningsTable` and `EventsTable` types in `docs/pages/userdocs/events.dox`. (@cline, @oruebel, [#305](https://github.com/NeurodataWithoutBorders/aqnwb/pull/305))
   * Added tutorials for the new `TimeIntervals` type in `docs/pages/userdocs/time_intervals.dox`. (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Added `Subject` class to represent the `/general/subject` group in NWB files. Added corresponding `SubjectSpec` to simplify configuration and initialization of `Subject`.  Updated `NWBFile::initialize` to accept a `SubjectSpec` argument for subject metadata initialization (@copilot, @oruebel, [#320](https://github.com/NeurodataWithoutBorders/aqnwb/pull/320))
   * Added `isPathOrDescendant` utility functoin to simplify path initialization checks for root groups, e.g, `/events` and `/intervals` (@copilot, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
* **Added support for single-ragged array columns in `DynamicTable`:**
   * Added `VectorIndex` type for storing index columns. (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Added `VectorData::appendData` and `VectorIndex::appendBuffer` to simplify appending values to columns. (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Updated `DynamicTable::addRows` and related methods to support ragged array column writes. (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
* **Added support for row-based read and write of `DynamicTable`:**
   * Added `DynamicTable::addRow` and `DynamicTable::addRows` methods to support row-based data insertion into a `DynamicTable`. Rows are specified as `RowData` (an `unordered_map` from column name to `CellValue` variant), enabling type-safe, column-keyed writes without requiring callers to manage per-column buffers directly. Row IDs are auto-generated if not provided. (@cline, @oruebel, [#305](https://github.com/NeurodataWithoutBorders/aqnwb/pull/305)). 
   * Added new `Types::CellValue` variant type to support single-cell values for `DynamicTable` rows. `CellValue` supports all scalar types, as well as `std::vector` for ragged array columns. Also defined `Types::RowData` as an `unordered_map<std::string, CellValue>` to represent a single row of data in a `DynamicTable`. (@cline, @oruebel, Originally defined in [#305](https://github.com/NeurodataWithoutBorders/aqnwb/pull/305) and then subsequently enhanced and moved in [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Added `DynamicTable::readRows` to support reading of full rows, along with `VectorData::readCellValues` and `VectorIndex::readIndexedCellValues`. (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Added `DynamicTable::toString` to support converting rows to a string representation. (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Added `DynamicTable::getNumberOfRows` to support retrieving the number of rows in a `DynamicTable`. (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Updated the user tutorial on reading data to discuss reading from `DynamicTable`. (@cline, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Added `BaseDataType::isCompatibleCellValue()`  and  `BaseDataType::isCompatibleVector()` for efficient runtime storage-type/variant compatibility check used in `VectorData::appendData()`  and  `appendBuffer()` to safeguard mismatching data types on write. (@copilot, @oruebel, [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
* **Added support for `DataSpec`-based column configuration in `DynamicTable`:**
   * **[BREAKING]** Updated `DynamicTable::initialize`, `ElectrodesTable::initialize`, `EventsTable::initialize`, and `MeaningsTable::initialize` to accept a `std::vector<DataSpecPtr>` column-spec list instead of individual `rowChunkSize` / resolution / flag parameters. This approach separated configuration from initialization, allowing users to fully configure all columns (including custom columns) beforehand such that `initialize` can  then handle the full setup of the table.   (@cline, @oruebel, [#305](https://github.com/NeurodataWithoutBorders/aqnwb/pull/305))
      * **Migration Note:**  Callers should use the corresponding `createDefaultDataSpecs(...)` static factory to obtain the default spec vector optionally modify it, and pass it to `initialize()`. `NWBFile::createElectrodesTable` and `NWBFile::createEventsTable` have been updated accordingly.
   * Added `DynamicTable::addColumn(DataSpecPtr)` overload consistent with the `DataSpec`-based API used by `initialize()` and `createDefaultDataSpecs()`.(@cline, @oruebel, [#301](https://github.com/NeurodataWithoutBorders/aqnwb/pull/301))
   * Added `Data::DataSpecBase` and `Data::DataSpec<T>` to support runtime configuration of `DynamicTable` column layouts and to decouple table schema definition from object initialization. Each `Data` subclass (e.g., `VectorData`, `ElementIdentifiers`, `TimestampVectorData`, `DurationVectorData`) now exposes a nested `DataSpec` struct that bundles the dataset configuration, description, and any type-specific parameters needed to create and initialize the column. (@cline, @oruebel, [#305](https://github.com/NeurodataWithoutBorders/aqnwb/pull/305))
   * Added `DynamicTable::createDefaultDataSpecs` static factory to return the default ordered list of `DataSpec` objects for a table (currently just the `id` column). Subclasses (`ElectrodesTable`, `EventsTable`, `MeaningsTable`) override this to append their own required columns, allowing callers to retrieve, inspect, and customize the default column configuration before passing it to `initialize()`. (@cline, @oruebel, [#305](https://github.com/NeurodataWithoutBorders/aqnwb/pull/305))
   * Added `BaseDataType::BaseDataVariant` scalar variant type and `BaseDataType::createEmptyVectorVariant` helper to support runtime-typed single-cell and buffer operations needed by `addRow`/`addRows`. (@cline, @oruebel, [#305](https://github.com/NeurodataWithoutBorders/aqnwb/pull/305))
   * Added `DyanmicTable::validateDataSpecs` (and `DynamicTable::checkRequiredColumnNames` helper function) to validate column specifications as part of `DyanmicTable::initialize` before initialization. Also updated subclasses of `DynamicTable` (e.g., `ElectrodesTable`, `EventsTable`, `MeaningsTable`) to override `validateDataSpecs` to provide their specific validation logic. (@oruebel, [#305](https://github.com/NeurodataWithoutBorders/aqnwb/pull/305)) 
* **Added support for zero-copy interleaved multichannel writes:**
   * Added `ElectricalSeries::writeAllChannels` method and `IO::writeElectricalSeriesData` overload to simplify zero-copy interleaved multichannel writes. (@copilot, @oruebel, [#293](https://github.com/NeurodataWithoutBorders/aqnwb/pull/293))
   * Added `ElectricalSeries::channelsAtSameSampleOffset` method to check if all channels are at the same sample offset, which is a requirement for using `writeAllChannels`. (@copilot, @oruebel, [#293](https://github.com/NeurodataWithoutBorders/aqnwb/pull/293))

### Changed
* **Enhanced caching of `RegisteredType` (types) and `BaseRecordingData` (datasets) to centralize recording state and avoid duplicate object creation.**
   * **[BREAKING]** Updated `RegisteredType::create()` to cache and reuse one canonical registered object per path. Factory methods resolve the concrete on-disk registered type before reusing a cached object and return `nullptr` for incompatible cached types rather than creating a duplicate or returning the wrong wrapper. `DataTyped`, `VectorDataTyped`, and `NWBDataTyped` remain transient typed facades and are not registered in `RecordingObjects`. (@cline, @oruebel, @copilot, [#328](https://github.com/NeurodataWithoutBorders/aqnwb/pull/328), [#320](https://github.com/NeurodataWithoutBorders/aqnwb/pull/320))
      * **Migration Note:** This change is mostly transparent to users, but it centralizes recording state. Code using `RegisteredType::create<T>()` must handle a `nullptr` result when an incompatible type is already cached at the requested path; use the specialized factories for typed facades.
   * **[BREAKING]** Updated the caching of `BaseRecordingData` to move from a per-object cache as part of `RegisteredType` objects (e.g., `TimeSeries`) to a central cache managed by the `BaseIO` I/O object. This change ensures that we have one `BaseRecordingData` object per dataset in a file, and that all `RegisteredType` objects that reference the same dataset will share the same `BaseRecordingData` object. This change centralizes the recording state. 
      * **Migration Note:** This change mainly affects users who were manually accessing or clearing the cache via `RegisteredType::getCacheRecordingData()` or `RegisteredType::clearRecordingDataCache()`. Instead, access the cache using the I/O object using `BaseIO::getRecordingDataCache()` and `BaseIO::clearRecordingDataCache()`. (@cline, @oruebel, [#328](https://github.com/NeurodataWithoutBorders/aqnwb/pull/328))
* **[BREAKING]** Moved `disableSWMRMode` option from `HDF5IO` constructor to a new `HDF5IO::startRecording(bool disableSWMRMode)` overload. The `BaseIO`-compliant `startRecording()` override is preserved and defaults to SWMR enabled. 
   * **Migration Note**: Code using `HDF5IO(path, true)` must be updated to `HDF5IO(path)` followed by `startRecording(true)`. When the `HDF5IO` object is held as a `std::shared_ptr<BaseIO>` (e.g., from `createIO`), downcast with `std::dynamic_pointer_cast<HDF5IO>` to access the overload. (@oruebel [#297](https://github.com/NeurodataWithoutBorders/aqnwb/pull/297))
* **[BREAKING]** Updated `Types.hpp` to use a `namespace` instead of `class` to group types. (@oruebel, [#325]((https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * **Migration Note:** For most users this change should not require any code changes since the syntax for accessing the types is the same (e.g., `AQNWB::Types::Status`), but for users who were using `using` declarations to import the type can now use `using namespace AQNWB::Types`.
* **[BREAKING]** Refactored `DynamicTable` so it creates and initializes its `ElementIdentifiers` `id` column internally during `initialize()`. As part of this change, `setRowIDs()` now takes only the row ID values and writes them directly to the table's built-in `id` column instead of accepting a separate `ElementIdentifiers` object. (@oruebel, [#302](https://github.com/NeurodataWithoutBorders/aqnwb/pull/302))
   * **Migration Notes:** In practice this aligns the API with the intended usage, since callers should have always used the table's own `id` dataset. The main change is in the signature of `DynamicTable::setRowIDs()`. The change in initialization behavior is part of the move to `DataSpec`-based table configurations such that migration is covered by the changes to `DynamicTable::initialize`.  
* **[BREAKING]** Updated `DynamicTable` column-name management to be immediate and table-driven. `addColumnName()` now flushes new column names to the file as columns are added, `setColNames()` is now restricted to reordering the existing set of columns (it no longer acts as a general overwrite mechanism), and `finalize()` no longer writes the `colnames` attribute because column-name updates are persisted when they occur. This ensures that the `colnames` attribute is always consistent with the actual columns in the table and helps avoid creation of invalid files. (@oruebel, [#304](https://github.com/NeurodataWithoutBorders/aqnwb/pull/304))
   * **Migration Note:** For most users this should not require code changes. The main change is that `setColNames()` now enforces that the new column names are consistent with the existing columns in the tables and, hence, cannot be used to arbitrarily overwrite the column names, but the new column names must be a reordering of the existing column names. If you need to add new columns, use `addColumn()` instead.
* **Updated CI test workflows:**
   * Updated `build-demo.yml` and `tests.yml` CI to support testing of the new ROS3 and remfile features and scripts  (@oruebel, [#308](https://github.com/NeurodataWithoutBorders/aqnwb/pull/308))
   * Updated `lint.yml` workflow to install `clang-format` via `pipx` to ensure use of the latest version of `clang-format` (@copilot, @oruebel [#325](https://github.com/NeurodataWithoutBorders/aqnwb/pull/325))
   * Updated nwbinspector validation tests to ignore the Allen CCF electrode location check when validating mock electrode locations. (@oruebel, [#320](https://github.com/NeurodataWithoutBorders/aqnwb/pull/320))
   * Updated nwbinspector validation tests in the CI to remove dependency on `sanitizer` tests to speed up CI (@oruebel, [#289](https://github.com/NeurodataWithoutBorders/aqnwb/pull/289))
   * Updated unit tests to create `Subject` for all NWB files generated in the test suite to comply with NWB best practices. `nwbinspector` tests require subject by default now. (@oruebel, [#289](https://github.com/NeurodataWithoutBorders/aqnwb/pull/289) removed this requirements by specifying  `--ignore=check_subject_exists` and [#320](https://github.com/NeurodataWithoutBorders/aqnwb/pull/320) removed the ignore to ensure that subject is always present in the test files)

### Fixed
* Fixed `get_utc_offset_seconds` to correctly account for daylight saving time using platform-specific APIs (`tm_gmtoff` on Unix/macOS; `_get_timezone` + `_get_dstbias` on Windows), preventing `session_start_time` from being written ~1 hour ahead of UTC during DST (@cboulay, [#295](https://github.com/NeurodataWithoutBorders/aqnwb/pull/295))
* Fixed `resources/utils/schematype_to_aqnwb.py` to facilitate generation of new types:
    * Emit forward declarations rather than full header includes for referenced generated types, avoiding circular header dependencies. This issue surfaced due to the update to the latest HDMF Common schema types where `DynamicTable` references `MeaningsTable`, which is itself a `DynamicTable` (@copilot, @oruebel, [#300](https://github.com/NeurodataWithoutBorders/aqnwb/pull/300))
    * Updated minimum Python version for `resources/utils` to ensure use of the latest versions of PyNWB with uv (@cline, @oruebel, [#301](https://github.com/NeurodataWithoutBorders/aqnwb/pull/301))
    * Updated cpp code generation to use new `std::weak_ptr` via `getIO` instead of the previous `m_io` shared pointer. (@cline, @oruebel, [#301](https://github.com/NeurodataWithoutBorders/aqnwb/pull/301))
    * Various indentation and formatting fixes in generated cpp code to improve readability (across multiple PRs, e.g., #300, #301).
* Fixed bug in `HDF5IO::canModifyObjects` returning true when a file is opened in read-only. (@oruebel, [#307](https://github.com/NeurodataWithoutBorders/aqnwb/pull/307))
* Fixed HDF5 string type creation to explicitly use UTF-8 character set for fixed-length and variable-length strings in datasets and attributes, improving compatibility with hdmf/PyNWB string decoding (@copilot, @oruebel [#319](https://github.com/NeurodataWithoutBorders/aqnwb/pull/319))
* Fixed Windows CI by updating the CMake generator in `CMakePresets.json` from `"Visual Studio 17 2022"` to `"Visual Studio 18 2026"` to match the updated `windows-latest` runner (@copilot, @oruebel [#319](https://github.com/NeurodataWithoutBorders/aqnwb/pull/319))


## [0.3.0] - 2026-02-23

### Added
* Added `cppcheck` static analysis integration (@copilot, @oruebel [#270](https://github.com/NeurodataWithoutBorders/aqnwb/pull/270)):
  * Added `.github/workflows/cppcheck.yml` CI workflow that runs cppcheck on `src/` and `tests/`, converts results to SARIF using `airtower-luna/convert-to-sarif`, and uploads them to the GitHub Security tab via `github/codeql-action/upload-sarif`
  * Added `cmake/cppcheck.cmake` standalone script and `cmake/cppcheck-targets.cmake` for running cppcheck locally via `cmake --build --preset=dev --target=cppcheck`
  * Updated developer documentation in `docs/pages/devdocs/testing.dox` and `docs/pages/devdocs/install.dox` with cppcheck usage instructions
* Added `getDataType()`, `getChunking()`, and `toLinkArrayDataSetConfig()` functions to `ReadDataWrapper` (@copilot, @oruebel [#266](https://github.com/NeurodataWithoutBorders/aqnwb/pull/266))
* Added support for creating soft-links to existing datasets to avoid data duplication (@copilot, @oruebel [#257](https://github.com/NeurodataWithoutBorders/aqnwb/pull/257))
  * Added `BaseArrayDataSetConfig` abstract base class for polymorphic dataset configuration
  * Added `LinkArrayDataSetConfig` class for creating HDF5 soft-links to existing datasets
  * Updated `ArrayDataSetConfig` to inherit from `BaseArrayDataSetConfig`
  * Updated all NWB type `initialize()` methods to accept `BaseArrayDataSetConfig` (TimeSeries, ElectricalSeries, SpikeEventSeries, AnnotationSeries, Data, NWBData, VectorData)
  * Added user docs page on using links and processing modules `docs/pages/userdocs/links.dox`
  * Added `BaseIO::getStorageObjectDataType` and `BaseIO::getStorageObjectChunking` and corresponding HDF5IO implementations
  * Updated `BaseIO::createArrayDataSet` to raise `std::runtime_error` on failure (rather than returning nullptr) to make error handling more robust and to allow link creation to return nullptr without ambiguity.
  * Added `LinkArrayDataSetConfig::validateTarget()` to validate that a link target is compliant with expected schema requirements and updated initialize methods of Data, VectorData, and TimeSeries accordingly to check that link targets are valid (@copilot, @oruebel [#259](https://github.com/NeurodataWithoutBorders/aqnwb/pull/259))
* Added `AQNWB::IO::ConstMultiArrayView<DTYPE, NDIMS>` as a lightweight, non-owning const multi-dimensional view over a buffer used to facilitate multi-dimensional array access in C++17/20  (@chittti , [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250)) 
* Added UUID/time/endian utilities in `src/Utils.hpp` to replace corresponding Boost utilities (@chittti, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250)) 
* Added AQNWB_CXX_STANDARD option to the cmake build to allow configuration of the std C++ version to support 17, 20, and 23 to allow the use of `std::mdspan` if C++23 is used  (@oruebel, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250))
* Added `ProcessingModule` class in `src/nwb/base/` to support creation of processed data groups in the `/processing/` hierarchy of NWB files (@oruebel, [#255](https://github.com/NeurodataWithoutBorders/aqnwb/pull/255))
* Added `.github/copilot-instructions.md` with comprehensive onboarding documentation for GitHub Copilot coding agents (@copilot, @oruebel, [#256](https://github.com/NeurodataWithoutBorders/aqnwb/pull/256))
* Added `docs/pages/devdocs/github_release_workflow.dox` with instructions for how to make a release (@oruebel, [#279](https://github.com/NeurodataWithoutBorders/aqnwb/pull/279))

### Changed
* Replaced explicit `std::vector<SizeType>` usages with the `SizeArray` type alias throughout the codebase for consistency (@copilot, @oruebel [#262](https://github.com/NeurodataWithoutBorders/aqnwb/pull/262))
* Standardized naming of `initialize()` parameters generated by `schematype_to_aqnwb` utility (@copilot, @oruebel [#261](https://github.com/NeurodataWithoutBorders/aqnwb/pull/261)):
* Removed Boost as a dependency  (@chittti, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250) )
* Updated `DataBlock::as_multi_array()` to return `ConstMultiArrayView` instead of `boost::const_multi_array_ref` to remove the need Boost and for C++17/20 compatibility  (@chittti, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250))
* Updated the docs and examples to discuss the optional use of `std::mdspan` if C++23 is used (@oruebel, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250))
* Changed the dependency includes in CMake so the HDF5 C++ headers and libraries found via `${HDF5_INCLUDE_DIRS}` / `${HDF5_CXX_LIBRARIES}` are exported to AqNWB consumers (@cboulay, [#248](https://github.com/NeurodataWithoutBorders/aqnwb/pull/248)).
* Updated NWBFile to make common root path definitions (e.g., acquisition, processing, stimulus) public to make path generation easier (@oruebel, [#257](https://github.com/NeurodataWithoutBorders/aqnwb/pull/257))

### Fixed
* Fixed `HDF5IO::createAttribute` to use base element type (typeSize=1) for numeric array attributes (instead of array-type with typeSize equal to the number of elements), to enhance compatibility with the NWB schema and PyNWB (@copilot, @oruebel [#276](https://github.com/NeurodataWithoutBorders/aqnwb/pull/276))
* Fixed a broad set of `cppcheck` static analysis warnings (@copilot, @oruebel [#274](https://github.com/NeurodataWithoutBorders/aqnwb/pull/274), [#278](https://github.com/NeurodataWithoutBorders/aqnwb/pull/278); @cline, @oruebel [#280](https://github.com/NeurodataWithoutBorders/aqnwb/pull/280)):
  * **Correctness**: Fixed `BaseIO::stopRecording` to return the computed composite `status` instead of hardcoded `Status::Success`; initialized `SpikeEventSeries::m_eventsRecorded` to `0` in-class; removed dead `chunk`/`DSetCreatPropList` code from `HDF5RecordingData` constructor; updated `~HDF5IO()` to avoid calling the virtual `close()` from the destructor; added `override` to virtual destructors
  * **Performance**: Changed `const std::string` / `const std::vector` pass-by-value parameters and return types to `const &` across `Channel`, `AnnotationSeries`, `ElectrodesTable`, `NWBFile`, `nwbio_utils.hpp`, `BaseIO`, `ReadIO`, `RegisteredType` and test utilities
  * **API hygiene**: Added `explicit` to single-arg constructors on `HDF5IO`, `HDF5RecordingData`, `ElectrodesTable`, and `NWBFile`; added `override` to `HDF5RecordingData::writeDataBlock` overloads; marked `BaseIO::getFullTypeName` `const`; made `ReadDataWrapper::getStorageObjectType` `static`; aligned constructor parameter names in `HDF5IO`, `HDF5ArrayDataSetConfig`, and `DynamicTable::addReferenceColumn`; declared `const std::tm*` pointers in `Utils.hpp`; removed unused variables in tests
  * **STL modernization**: Replaced raw loops with `std::any_of`, `std::accumulate`, `std::transform`, and `std::find_if` in `BaseIO.cpp`, `HDF5IO.cpp`, `RecordingObjects.cpp`, and `ReadIO.hpp`
  * **cppcheck suppressions**: Added `--suppress=duplInheritedMember` and `--suppress=normalCheckLevelMaxBranches` globally to `cmake/cppcheck.cmake` and `.github/workflows/cppcheck.yml`; added inline `cppcheck-suppress` comments for intentional test patterns (`duplicateExpression`, `knownConditionTrueFalse`, `useStlAlgorithm`, `variableScope`)
* Restricted code coverage report to `src/` folder only (@copilot, @oruebel [#262](https://github.com/NeurodataWithoutBorders/aqnwb/pull/262)):
  * Updated `cmake/coverage.cmake` to use a two-step lcov approach: capture all coverage into `coverage_raw.info`, then use `lcov --extract` to filter to `src/*` only, producing the final `coverage.info`
  * Added `.codecov.yml` to instruct the Codecov server to ignore `tests/**`, preventing Codecov-action from including test files it discovers via independent `gcov` scanning of the build directory
* Removed redundant `resources/utils/requirements.txt` and update CI accordingly. Dependencies are now defined in `resources/utils/pyproject.toml` (@copilot, @oruebel [#260](https://github.com/NeurodataWithoutBorders/aqnwb/pull/260))
* Added explicit `permissions` blocks to all GitHub Actions workflow files to restrict `GITHUB_TOKEN` to least-privilege scopes (@copilot, @oruebel [#269](https://github.com/NeurodataWithoutBorders/aqnwb/pull/269))


## [0.2.0] - 2025-12-22

### Added
* Python Utility enhancements:
    * Added `aqnwb-utils` as a command-line utility to provide a common interface for aqnwb command line tools, e.g., `schematype_to_aqnwb.py` and `generate_spec_files.py`. (@oruebel, [#227](https://github.com/NeurodataWithoutBorders/aqnwb/pull/227))
    * Added inline script metadata (PEP 723) to Python utilities to enable direct execution with `uv run` without installation (@oruebel, [#229](https://github.com/NeurodataWithoutBorders/aqnwb/pull/229))
    * Added `pyproject.toml` for modern Python packaging support (@oruebel, [#229](https://github.com/NeurodataWithoutBorders/aqnwb/pull/229))
* Added `RegisteredType::DEFINE_UNNAMED_REGISTERED_FIELD` to simplify creation of read/write methods for RegisteredTypes that do not have a set name in the schema (@oruebel, [#231](https://github.com/NeurodataWithoutBorders/aqnwb/pull/231))
* Added new `NWBData`, `NWBDataInterface`, and `NWBContainer` data types and updated existing classes to match inheritance with NWB schema (@oruebel, [#232](https://github.com/NeurodataWithoutBorders/aqnwb/pull/232))
* Automated tracking and harmonized memory management of `RegisteredType` objects (@oruebel, [#209](https://github.com/NeurodataWithoutBorders/aqnwb/pull/209))
   * Added `BaseIO.m_recording_objects` to track all `RegisteredType` objects used for recording
   * Modified `RegisteredType` to automatically register with `RecordingObjects` instance of the IO
   * Updated `RegisteredType` to use `std::weak_ptr` to the IO to avoid circular referencing
   * Made the constructor of `RegisteredType` and all its subclasses `protected` to prevent direct stack or raw pointer creation. `RegisteredType` objects must now always be created via the `RegisteredType.create` factory, ensuring that all objects are being created as `std::smart_ptr` and registered with the `m_recording_objects` RecordingContainers object of the I/O object
   * Updated `RegisteredType` to inherit from `public std::enable_shared_from_this<RegisteredType>`
   * Updated the `REGISTER_SUBCLASS` macro to add a `create` factory method for all classes
* Harmonized finalization and clean-up of `RegisteredType` objects (@oruebel, [#209](https://github.com/NeurodataWithoutBorders/aqnwb/pull/209))
   * Added `RegisteredType.finalize` to finalize all neurodata_type classes
   * Added `RecordingObjects.finalize` and `RecordingObjects.clearRecordingDataCache` to finalize and clean up all objects in a single call
   * Updated `BaseIO.stopRecording` to call `m_recording_objects.finalize()` and `m_recording_objects.clearRecordingDataCache()`

### Changed
* Updated documentation to refer to the new `aqnwb-utils` command-line utility (@oruebel, [#227](https://github.com/NeurodataWithoutBorders/aqnwb/pull/227))
* Updated Python utilities to use `uv` instead of `pip` for dependency management and updated docs and github workflows to use uv (@oruebel, [#227](https://github.com/NeurodataWithoutBorders/aqnwb/pull/227)
* Enhanced the `schematype_to_aqnwb` utility script:
    * Generated source files are now placed into a folder hierarchy based on the name of the namespace and schemafile of the neurodata_type (@oruebel, [#224](https://github.com/NeurodataWithoutBorders/aqnwb/pull/224))
    * Added functionality to optionally create a simple example app that instantiates all generated classes to help test that all generated classes can be compiled (@oruebel, [#225](https://github.com/NeurodataWithoutBorders/aqnwb/pull/225))
    * Updated generation of header files to ensure proper compilation, e.g.: i) identify and include the headers of all neurodata_types that are being used, ii) fixed formatting of comments to avoid nested multi-line comments, iii) fixed issues with incomplete typenames  (@oruebel, [#225](https://github.com/NeurodataWithoutBorders/aqnwb/pull/225))
    * Added GitHub action to test that all sources files generated by the `schematype_to_aqnwb` utility for the nwb-schema and LabMetaDataExtension example can be compiled (@oruebel, [#225](https://github.com/NeurodataWithoutBorders/aqnwb/pull/225))
    * Updated the rendering of initialize methods to ensure inclusion of all owned parameters (including those in subgroups), add rendering of default values, and add rendering of suggested initialization code (@oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Updated the rendering of initialize cpp source to create the correct call to the parents initialize method @oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Updated rendering of read/record methods via DEFINE_FIELD macros to ensure inclusion of all owned parameters (including those in subgroups), consistent with the updates to the initialize methods (@oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Refactored code to generated initialize methods to centralize logic and make the code more robust. E.g., created new `get_initialize_method_parameters` to compile parameter details in one place and split the  `render_initialize_method`  into two functions. (@oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Enhanced the new `get_initialize_method_parameters` method and rendering functions to correctly track the full path of objects (@oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Added support for rendering `DEFINE_REFERENCED_REGISTERED_FIELD` macros for attributes that are references (@oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Added rendering of virtual destructor in the header source (@oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Updated generated initialize methods to always return a Status (@oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Updated new `get_initialize_method_parameters` to ensure for neurodata_types that are Datasets that we include the dataset itself as a custom parameter that needs initialization (@oruebel, [#230](https://github.com/NeurodataWithoutBorders/aqnwb/pull/230))
    * Added support for `DEFINE_UNNAMED_REGISTERED_FIELD` macros for RegisteredTypes that are unnamed in the schema (@oruebel, [#231](https://github.com/NeurodataWithoutBorders/aqnwb/pull/231)
    * Simplify the required signature of the generated initialize methods by placing optional RegisteredType arguments in comment blocks as these are usually created afterward initialize by the user (@oruebel, [#231](https://github.com/NeurodataWithoutBorders/aqnwb/pull/231))
    * Added support for attributes/datasets with fixed values, which are now created only inside the generated initialize method but no longer setable as a parameter (@oruebel, [#231](https://github.com/NeurodataWithoutBorders/aqnwb/pull/231))
* Enhanced handling of finalization and columns for `DynamicTable` (@oruebel, [#209](https://github.com/NeurodataWithoutBorders/aqnwb/pull/209)) 
   * Updated NWBFile::createElectrodesTable to return the created ElectrodesTable and added a `finalizeTable` parameter to make it configurable whether the table should be finalized.
   * Updated `DynamicTable` and `ElectrodeTable` to handle finalization and columns more robustly to make sure that repeated calls to finalize do not corrupt the data.
      * NOTE: The meaning of the `m_groupReferences`, `m_locationNames`, `m_groupNames` variables has changed slightly in that they now only track new row values that have not been added via finalize.
   * Added `std::unique_ptr<IO::RecordingObjects> m_recordingColumns` and `std::shared_ptr<ElementIdentifiers> m_rowElementIdentifiers` to `DynamicTable` to track the columns added for recording in the table directly.
* Renamed `RecordingContainers` to `RecordingObjects` and updated it to support tracking of all `RegisteredType` objects for recording (@oruebel, [#209](https://github.com/NeurodataWithoutBorders/aqnwb/pull/209))
   * Added `RecordingObjects.clear`, `RecordingObjects.size`, and `RecordingObjects.getRecordingIndex` methods  
   * Updated `RecordingObjects.addRecordingObject` to prevent adding of duplicate objects
   * Added RecordingObjects.getRecordingIndex function to find the index to allow search for a recording object
   * Moved RecordingObjects from the AQNWB::NWBnamespace to the ANWB::IO namespace
   * Moved NWB I/O utility functions (e.g., `writeTimeSeriesData`) from `RecordingObjects` to their own `src/io/nwbio_utils.hpp` header
   * Added RecordingObjects::`getRecordingObject(const std::string& path)` to simplify lookup of objects based on path
   * Added RecordingObjects::toString method for convenient printing   
* Updated `initialize` functions of all `RegisteredType` classes to return a `Status` [#209](https://github.com/NeurodataWithoutBorders/aqnwb/pull/209))
* Changed the value of `Status::SUCCESS` to 1 instead of 0 [#209](https://github.com/NeurodataWithoutBorders/aqnwb/pull/209))
* Added `Types.SizeTypeNotSet` and `Utils.isValidIndex` to centralize definition and checking for invalid indices [#209](https://github.com/NeurodataWithoutBorders/aqnwb/pull/209))

### Fixed
* Resolved various compiler warnings on Windows (`-Wmaybe-uninitialized`, `-Wsign-conversion`, `-Wconversion`, `-Wshadow`, `-Wdeprecated-copy`, `-Wcatch-value`) and fixed cross-platform build issues related to `std::filesystem` linkage. (@oruebel, [#233](https://github.com/NeurodataWithoutBorders/aqnwb/pull/233))

## [0.1.0] - 2025-09-03

This release of AqNWB provides the initial C++ interface for reading and writing Neurodata Without Borders (NWB) files

### Added
* Initial implementation of NWB file creation and management with HDF5 backend
* Introduced Device, ElectrodeGroup, and DynamicTable classes for HDMF/NWB data types
* RecordingContainers for managing TimeSeries objects
* NWB data types for ecephys acquisition: ElectrodesTable, ElectricalSeries, and TimeSeries (@stephprince, [#161](https://github.com/NeurodataWithoutBorders/aqnwb/pull/161))
* NWB data type for annotation: AnnotationSeries (@stephprince, [#141](https://github.com/NeurodataWithoutBorders/aqnwb/pull/141))
* NWB data type for spike detection: SpikeEventSeries (@stephprince, [#92](https://github.com/NeurodataWithoutBorders/aqnwb/pull/92))
* BaseRecordingData management system for data acquisition (@oruebel, [#190](https://github.com/NeurodataWithoutBorders/aqnwb/pull/190))
* SWMR (Single Writer Multiple Readers) mode for concurrent file access (@stephprince, [#45](https://github.com/NeurodataWithoutBorders/aqnwb/pull/45))
* Namespace registry for extension management (@oruebel, [#181](https://github.com/NeurodataWithoutBorders/aqnwb/pull/181))
* Support for reading arbitrary RegisteredTypes, reference attributes and links (@oruebel, [#143](https://github.com/NeurodataWithoutBorders/aqnwb/pull/143), [#158](https://github.com/NeurodataWithoutBorders/aqnwb/pull/158))
* Multi-dimensional data blocks with std::variant support (@oruebel, [#177](https://github.com/NeurodataWithoutBorders/aqnwb/pull/177))
* HDF5 filters and compression for array datasets (@oruebel, [#163](https://github.com/NeurodataWithoutBorders/aqnwb/pull/163), [#165](https://github.com/NeurodataWithoutBorders/aqnwb/pull/165))
* Schema generation script from NWB specifications (@oruebel, [#199](https://github.com/NeurodataWithoutBorders/aqnwb/pull/199))
* Demo applications and extension implementation examples (@oruebel, [#171](https://github.com/NeurodataWithoutBorders/aqnwb/pull/171), [#183](https://github.com/NeurodataWithoutBorders/aqnwb/pull/183))
* NWB file validation using nwbinspector (@stephprince, [#122](https://github.com/NeurodataWithoutBorders/aqnwb/pull/122))
* Cross-platform CI/CD with GitHub Actions (Linux, macOS, Windows) (@stephprince, [#99](https://github.com/NeurodataWithoutBorders/aqnwb/pull/99))
* Code coverage reporting with codecov (@stephprince, [#120](https://github.com/NeurodataWithoutBorders/aqnwb/pull/120), [#135](https://github.com/NeurodataWithoutBorders/aqnwb/pull/135))
* Doxygen documentation with GitHub Pages deployment (@oruebel, [#74](https://github.com/NeurodataWithoutBorders/aqnwb/pull/74))

### Changed
* Refactored BaseRecordingData object management for acquisition (@oruebel, [#190](https://github.com/NeurodataWithoutBorders/aqnwb/pull/190))
* Updated ElectrodesTable type definitions (@oruebel, [#214](https://github.com/NeurodataWithoutBorders/aqnwb/pull/214))
* Restructured documentation with separate user and developer sections (@oruebel, [#159](https://github.com/NeurodataWithoutBorders/aqnwb/pull/159))

### Fixed
* ElectrodesTable reading for NWB <=2.8 compatibility (@oruebel, [#216](https://github.com/NeurodataWithoutBorders/aqnwb/pull/216))
* ElectricalSeries electrode dataset write functionality (@stephprince, [#156](https://github.com/NeurodataWithoutBorders/aqnwb/pull/156))
* Channel conversion axis attribute for ElectricalSeries (@oruebel, [#109](https://github.com/NeurodataWithoutBorders/aqnwb/pull/109))
* Memory management with smart pointers (@stephprince, [#42](https://github.com/NeurodataWithoutBorders/aqnwb/pull/42))
* Build warnings with Doxygen 1.14 (@oruebel, [#202](https://github.com/NeurodataWithoutBorders/aqnwb/pull/202))
