# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]


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
