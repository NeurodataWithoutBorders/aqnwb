---
title: 'AqNWB: A C++ Library for Direct Acquisition of Neurodata Without Borders (NWB) Data'
tags:
  - C++
  - neuroscience
  - NWB
  - data acquisition
authors:
  - name: Stephanie Prince
    equal-contrib: true
    affiliation: 1
  - name: Joshua Siegle
    orcid: 0000-0002-7736-4844
    affiliation: 2
  - name: Oliver Ruebel
    orcid: 0000-0001-9902-1984
    equal-contrib: true
    corresponding: true
    affiliation: 1
affiliations:
 - name: Computational Biosciences Group, Scientific Data Division, Lawrence Berkeley National Laboratory, Berkeley, CA, USA
   index: 1
 - name: Allen Institute for Neural Dynamics, Seattle, WA, USA
   index: 2
date: 03 March 2026
bibliography: paper.bib
---


# Summary

AqNWB is a C++ library that enables direct, high-performance acquisition of neurophysiology and behavioral data in the Neurodata Without Borders (NWB) data standard. Designed for integration with data acquisition systems, AqNWB provides a lightweight, extensible API for writing and reading NWB files in real time, eliminating the need for post-hoc data conversion. AqNWB supports both core NWB types and custom extensions, offering schema-driven data management, efficient read/write, and full compatibility with the NWB ecosystem. By bridging the gap between C++ acquisition software and the NWB standard, AqNWB simplifies standardized data collection, improves metadata integrity, and accelerates reproducible neuroscience research.

# Statement of Need

Neurodata Without Borders (NWB) [@Ruebel2022_eLife] is a leading data standard for neurophysiology, with over 480 public datasets on the DANDI archive encompassing more than 470TB of data across more than 180,000 sessions from over 150 labs worldwide [@nwbDANDIData]. This success has enabled the creation of a rich software ecosystem with over 30 community-built tools for analysis, visualization, and data management supporting NWB.

Despite this widespread adoption, a critical gap remains in the data acquisition ecosystem. Most neurophysiology data acquisition systems are implemented in C++ for high-performance hardware integration, yet NWB software APIs are currently mainly focused on Python (PyNWB, RRID:SCR_017452, [@pynwb]) and MATLAB (MatNWB, RRID:SCR_021156, [@matnwb])[@Ruebel2022_eLife]. Due to this architectural mismatch, researchers typically collect data in proprietary formats and rely on post-hoc conversion scripts to transform data into NWB, creating several significant challenges:

**Barrier to Adoption:** The lack of a native C++ API for NWB represents a significant technical barrier for hardware manufacturers and acquisition software developers who want to support the NWB standard and neuroscientists seeking to adopt NWB throughout the data lifecycle.

**Data Conversion Bottlenecks:** Post-hoc conversion introduces potential points of failure, data loss, and metadata inconsistencies. Conversion scripts are often custom-built for specific systems, making them difficult to maintain and validate across different experimental setups.

**Performance Limitations:** Converting large neurophysiology datasets (often gigabytes to terabytes) is computationally expensive and time-consuming, making conversion particularly challenging for high-throughput experiments or real-time analysis workflows requiring immediate data availability.

AqNWB addresses these challenges by providing a high-performance, lightweight C++ API that enables direct acquisition of neurophysiological and behavior data in NWB files. By integrating NWB support at the point of data collection, AqNWB eliminates conversion bottlenecks, preserves metadata integrity, and facilitates compliance with FAIR data principles. This approach empowers acquisition software developers and research groups to adopt standardized data practices from the outset.

# State of the Fields

NWB software APIs are currently mainly focused on Python (PyNWB) and MATLAB (MatNWB). Researchers commonly convert data to NWB after acquisition using these APIs and dedicated data conversion tools, e.g., NeuroConv and NWB GUIDE. Direct integration of NWB with data acquisition systems is limited by the lack of a native C++ API for NWB. 

# Software Design

AqNWB is built around a multi-layered architecture that separates I/O operations, data management, and type registration to provide efficient, extensible, and robust data acquisition into the NWB format.

**Pluggable I/O Architecture:** AqNWB uses `BaseIO` as an abstract interface with `HDF5IO` as the primary implementation, enabling future integration of additional storage backends such as Zarr. The HDF5 backend supports both data reading and writing operations, including Single Writer Multiple Reader (SWMR) mode for concurrent access during acquisition, chunking and compression for efficient read/write, and automatic memory management.

**Streaming Data Management:** The `BaseRecordingData` class manages individual dataset recording with automatic position tracking, enabling efficient streaming writes without manual state management. The `RecordingObjects` class then provides higher-level coordination for multi-modal experiments with specialized methods for different data types (for example, for writing `ElectricalSeries` data). 

**Dynamic Type System:** The `RegisteredType` class provides a common base class for implementing NWB neurodata types and implements a dynamic registry that maps NWB neurodata types to C++ classes, enabling automatic object creation from file metadata. The system uses efficient hash-based lookups and supports both core NWB types and custom extensions through a unified interface. All instances of `RegisteredType` classes (e.g., a `TimeSeries`) are automatically registered with the `RecordingObjects` of the I/O object to facilitate management of memory and data objects. 

**Macro-based Code Generation:** AqNWB uses a collection of macros defined by the  `RegisteredType` class to generate consistent read and write methods, reduce boilerplate code, ensure consistency of the API, and simplify integration of new NWB types, e.g., `DEFINE_DATASET_FIELD`, `DEFINE_ATTRIBUTE_FIELD`, and `DEFINE_REGISTERED_FIELD` to read/write datasets, attributes, and nested types, respectively.

**Lazy Loading System:** The read system uses `ReadDataWrapper` objects for lazy data loading, supporting both full dataset reads and efficient slicing operations. Data is returned as `DataBlock` objects with typed vectors and shape information, or as `DataBlockGeneric` for unknown types with `std::variant` and `std::mdspan` support.

**Extension Support:** AqNWB provides comprehensive support for NWB extensions through automatic schema conversion from YAML/JSON to C++ headers, namespace registration, and template-based type implementation. Extension schemas are automatically cached in output files for compatibility with other NWB tools.

**Performance Optimization:** The library leverages advanced HDF5 features including configurable chunking strategies, automatic dataset extension for streaming data, and efficient memory management. The macro-based code generation eliminates runtime reflection overhead while maintaining type safety.

# Research Impact Statement

The nwb-format plugin [@nwbOpenEphys] integrates NWB with the OpenEphys [@Siegle_2017] acquisition software, enabling direct recording of NWB data during experiments. This integration allows researchers to adopt the NWB standard from the point of data collection, eliminating the need for post-hoc conversion and ensuring that data is immediately available in a standardized format for analysis and sharing. By facilitating real-time NWB recording, this plugin promotes FAIR data practices, enhances metadata integrity, and accelerates reproducible neuroscience research. Further integrations of AqNWB with other acquisition systems are ongoing. AqNWB supports data acquisition and reading of NWB data and integration of extensions via the following workflows.

## Data Acquisition

AqNWB provides a structured workflow for acquiring neurophysiological data directly into NWB format files. The process is designed to handle both simple single-stream recordings and complex multi-modal experiments with coordinated data streams.

1. **Create the I/O object:** Use `AQNWB::createIO` to create an I/O backend (for example, `HDF5IO`) for writing data. The I/O object handles file creation, dataset management, and low-level write operations. For HDF5 files, this automatically configures optimal chunking strategies and enables SWMR mode for concurrent read access during recording and to ensure data integrity.  
2. **Create and initialize NWBFile:** Construct an `NWBFile` object and call `initialize()` to set up the basic file structure, including required metadata fields (`session_id`, `session_description`, `identifier`) and the `/general`, `/acquisition`, and `/processing` groups. This step also caches the NWB schema specification in the file for compatibility. The `NWBFile` and all subsequently created objects (e.g., `TimeSeries`) are automatically tracked by the `RecordingObjects` object of the I/O object.   
3. **Create metadata objects:** Add essential metadata to `/general`, such as: `ElectrodeTable` (defines electrode configurations, locations, and groupings), `Device` objects (describes recording hardware and acquisition parameters), `ElectrodeGroup` objects (organizes electrodes by physical or logical groupings), and `Subject` information and experimental protocols.  
4. **Create data containers:** Instantiate container objects for each acquisition data stream (for example, `ElectricalSeries` for electrophysiology) initialized with appropriate metadata and dataset configurations.  
5. **Start recording:** Call `startRecording()` on the I/O object to begin the acquisition session. This enables SWMR mode, finalizes dataset creation, and prepares the file for streaming writes.  
6. **Write data:** During acquisition, use individual dataset writes via `BaseRecordingData` objects for fine-grained control container-specific or convenience methods, for example, `writeElectricalSeriesData()` to coordinate write to multiple datasets, such as voltage and timestamp data.  
7. **Stop recording and close:** Call `stopRecording()` to finalize and close the file.

## Data Reading

AqNWB's data reading system is built around two core abstractions: 1\) `ReadDataWrapper` provides a lazy-loading interface for datasets and attributes, only loading data when explicitly requested, and 2\) `DataBlock` represents loaded data. The typical workflow for reading data is as follows:

1. **Open the file:** Use `AQNWB::createIO` to open an existing NWB file in read mode.  
2. **Identify and construct data objects:** Instantiate `RegisteredType` objects for the data you wish to access. This can be done directly with a known path (for example, `ElectricalSeries::create("/acquisition/ElectricalSeries", io)` or generically with automatic type detection (via `RegisteredType::create(path, io)`). AqNWB also supports searching for objects in NWB files via the `findTypes()` or `findOwnedTypes()` functions.  
3. **Create lazy-loading wrappers:** Access datasets and attributes via dedicated read methods (such as `readData()` or the generic `readField()`), returning `ReadDataWrapper` objects, which defer data loading until needed.  
4. **Load and access data:** When ready, request data from the `ReadDataWrapper` wrapper, using: i) `values()` to load the full dataset or attribute as a typed `DataBlock`, ii) `values(start, count)` for efficient, partial reads to only load the requested slices into memory, of iii) `valuesGeneric()` for unknown types, which returns a `DataBlockGeneric` for type-safe, runtime handling.  
5. **Work with loaded data:** Access typed vectors directly via `DataBlock::data`, obtain multi-dimensional views using `DataBlock::as_multi_array()` or using `std::mdspan`, or work with unknown types using `std::variant` via `DataBlockGeneric::as_variant()`.

This unified interface provides efficient, flexible access to all NWB content, including datasets, attributes, references, and nested objects, while minimizing memory usage and supporting both exploratory analysis and large-scale data processing.

## Extension Integration

AqNWB's extension system allows users to seamlessly incorporate custom NWB extensions to support new data types while preserving full compatibility with the NWB ecosystem. The process for developing and integrating extensions is as follows:

1. **Define the extension schema:** Define the YAML schema for the extension using established NWB extension workflows. The schema specifies new neurodata\_type classes, their fields, and inheritance.  
2. **Generate C++ headers:** Use the generate\_spec\_files.py script provided by AqNWB to convert extension schemas into C++ header files with namespace definitions, namespace registration code, and documentation for your extension.  
3. **Implement RegisteredType classes:** For each new neurodata\_type, create a C++ class that: inherits from the appropriate base (Container, Data, or another extension type), uses the REGISTER\_SUBCLASS macro for automatic type registration, implements initialize() methods for data creation, and defines field accessors with macros like DEFINE\_DATASET\_FIELD, DEFINE\_ATTRIBUTE\_FIELD, and DEFINE\_REGISTERED\_FIELD.  
4. **Register and use extension types:** Including the generated headers automatically registers your extension namespace with AqNWB's NamespaceRegistry schema registry and RegisteredType type registry. Extension types are then available for reading and writing, just like core NWB types.

AqNWB's extension system supports automatic schema caching (embedding extension schemas into NWB files for self-describing and portable data), template types for flexible and type-safe data handling, and inheritance/composition for building complex data models atop NWB or other extensions. Extension types are fully integrated with AqNWB's reading, writing, type discovery, and validation systems, ensuring identical performance and functionality to core types.

# AI Usage Disclosure

Original design and implementation of the software by S. Prince and O. Ruebel. GitHub CoPilot was used in development and review of select features documented in the respective pull requests on GitHub. All code (both human and AI-generated) has undergone review by the development team via GitHub pull requests.

# Acknowledgments

Research reported in this publication was supported by the National Institute Of Neurological Disorders And Stroke of the National Institutes of Health under Award Number R03NS145401. The content is solely the responsibility of the authors and does  
not necessarily represent the official views of the National Institutes of Health. Initial design of AqNWB was supported by the Kavli Foundation. 

We thank and acknowledge contributions from Chadwick Boulay, Likhith Chitneni, Benjamin Dichter, Ryan Ly, and Joshua Siegle to the AqNWB software.

We thank the Neurodata Without Borders (NWB) community for developing and maintaining the NWB standard, and the broader open-source community for their feedback and contributions. We are grateful to research groups and acquisition system teams who provided real-world requirements that shaped AqNWB's design.  

# References

