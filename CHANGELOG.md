# Changelog 

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
* Added support for creating soft-links to existing datasets to avoid data duplication (@copilot, @oruebel [#257](https://github.com/NeurodataWithoutBorders/aqnwb/pull/257))
  * Added `BaseArrayDataSetConfig` abstract base class for polymorphic dataset configuration
  * Added `LinkArrayDataSetConfig` class for creating HDF5 soft-links to existing datasets
  * Updated `ArrayDataSetConfig` to inherit from `BaseArrayDataSetConfig`
  * Updated all NWB type `initialize()` methods to accept `BaseArrayDataSetConfig` (TimeSeries, ElectricalSeries, SpikeEventSeries, AnnotationSeries, Data, NWBData, VectorData)
  * Added user docs page on using links and processing modules
  * Added `BaseIO::getStorageObjectDataType` and `BaseIO::getStorageObjectChunking` and corresponding HDF5IO implementations
  * Updated `BaseIO::createArrayDataSet` to raise `std::runtime_error` on failure (rather than returning nullptr) to make error handling more robust and to allow link creation to return nullptr without ambiguity.
* Added `AQNWB::IO::ConstMultiArrayView<DTYPE, NDIMS>` as a lightweight, non-owning const multi-dimensional view over a buffer used to facilitate multi-dimensional array access in C++17/20  (@chittti , [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250)) 
* Added UUID/time/endian utilities in `src/Utils.hpp` to replace corresponding Boost utilities (@chittti, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250)) 
* Added AQNWB_CXX_STANDARD option to the cmake build to allow configuration of the std C++ version to support 17, 20, and 23 to allow the use of `std::mdspan` if C++23 is used  (@oruebel, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250))
* Added `ProcessingModule` class in `src/nwb/base/` to support creation of processed data groups in the `/processing/` hierarchy of NWB files (@oruebel, [#255](https://github.com/NeurodataWithoutBorders/aqnwb/pull/255))
* Added `.github/copilot-instructions.md` with comprehensive onboarding documentation for GitHub Copilot coding agents (@copilot, @oruebel, [#256](https://github.com/NeurodataWithoutBorders/aqnwb/pull/256))

### Changed
* Removed Boost as a dependency  (@chittti, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250) )
* Updated `DataBlock::as_multi_array()` to return `ConstMultiArrayView` instead of `boost::const_multi_array_ref` to remove the need Boost and for C++17/20 compatibility  (@chittti, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250))
* Updated the docs and examples to discuss the optional use of `std::mdspan` if C++23 is used (@oruebel, [#250](https://github.com/NeurodataWithoutBorders/aqnwb/pull/250))
* Changed the dependency includes in CMake so the HDF5 C++ headers and libraries found via `${HDF5_INCLUDE_DIRS}` / `${HDF5_CXX_LIBRARIES}` are exported to AqNWB consumers (@cboulay, [#248](https://github.com/NeurodataWithoutBorders/aqnwb/pull/248)).
* Updated NWBFile to make common root path definitions (e.g., acquisition, processing, stimulus) public to make path generation easier (@oruebel, [#257](https://github.com/NeurodataWithoutBorders/aqnwb/pull/257))

### Fixed
* Removed redundant `resources/utils/requirements.txt` and update CI accordingly. Dependencies are now defined in `resources/utils/pyproject.toml` (@copilot, @oruebel [#260](https://github.com/NeurodataWithoutBorders/aqnwb/pull/260))


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
