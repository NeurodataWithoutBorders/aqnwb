# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]


## [0.1.0] - 2025-09-03

This is the first official release of AqNWB, providing a C++ interface for reading and writing Neurodata Without Borders (NWB) files.

### Added
* Initial implementation of NWB file creation and management with HDF5 backend
* Introduced Device, ElectrodeGroup, and DynamicTable classes for HDMF/NWB data types
* NWB data types for ecephys acquisition: ElectrodesTable, ElectricalSeries, and TimeSeries (#161)
* NWB data types for spike detection, annotation: AnnotationSeries and SpikeEventSeries (#141, #92)
* RecordingContainers for managing TimeSeries objects
* BaseRecordingData management system for data acquisition (#190)
* SWMR (Single Writer Multiple Readers) mode for concurrent file access (#45)
* Namespace registry for extension management (#181)
* Support for reading arbitrary RegisteredTypes, reference attributes and links (#143, #158)
* Multi-dimensional data blocks with std::variant support (#177)
* HDF5 filters and compression for array datasets (#163, #165)
* Schema generation script from NWB specifications (#199)
* Demo applications and extension implementation examples (#171, #183)
* NWB file validation using nwbinspector (#122)
* Cross-platform CI/CD with GitHub Actions (Linux, macOS, Windows) (#99)
* Code coverage reporting with codecov (#120, #135)
* Doxygen documentation with GitHub Pages deployment (#74)

### Changed
* Refactored BaseRecordingData object management for acquisition (#190)
* Updated ElectrodesTable type definitions (#214)
* Restructured documentation with separate user and developer sections (#159)

### Fixed
* ElectrodesTable reading for NWB <=2.8 compatibility (#216)
* ElectricalSeries electrode dataset write functionality (#156)
* Channel conversion axis attribute for ElectricalSeries (#109)
* Memory management with smart pointers (#42)
* Build warnings with Doxygen 1.14 (#202)

