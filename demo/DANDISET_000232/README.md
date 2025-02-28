# NWB Analysis Demo

This is a simple C++ demo project that shows how to read and analyze NWB (Neurodata Without Borders) files using the aqnwb library. The demo reads the file `sub-EF0147_ses-20190204T144339_behavior+ecephys.nwb` and performs basic statistical analysis on the electrophysiology data.

## Prerequisites

- CMake (version 3.15 or higher)
- C++ compiler with C++17 support
- HDF5 library with C++ support
- Boost library
- aqnwb library (built from the parent project)

## Downloading example data

* DANDIset: https://dandiarchive.org/dandiset/000232/0.240510.2038
* Download file: https://api.dandiarchive.org/api/assets/4c440a73-9250-45bb-a342-a7da4d01b2fd/download?content_disposition=inline

## Building the Demo

1. Make sure you have built the main aqnwb project first:

```bash
# From the root of the aqnwb project
mkdir -p build
cd build
cmake ..
make
```

2. Create a build directory for the demo:

```bash
# From the demo/DANDISET_000232 directory
mkdir -p build
cd build
```

3. Configure and build the demo:

```bash
# Basic configuration
cmake ..

# Or with custom paths to dependencies
cmake .. \
  -DAQNWB_DIR=../../build/dev \
  -DHDF5_DIR=../../libs/hdf5_build \
  -DBOOST_ROOT=../../libs/boost_1_82_0

# Build the project
make
```

## Running the Demo

After building, you can run the demo:

```bash
# From the demo/DANDISET_000232/build directory
cd bin
./nwb_analysis_demo
```

The program will:

1. Open the NWB file in read-only mode
2. Search for ElectricalSeries objects in the file
3. Read the data from the first ElectricalSeries found
4. Calculate and display statistics for each channel:
   - Mean
   - Standard deviation
   - Minimum value
   - Maximum value
   - Range
   - Sample values
5. Display the unit of the data

## Code Structure

- `main.cpp`: Contains the main program logic
- `CMakeLists.txt`: CMake configuration file for building the project

## Example Output

```
Opening NWB file: sub-EF0147_ses-20190204T144339_behavior+ecephys.nwb
Searching for ElectricalSeries objects...
Found 3 ElectricalSeries objects.
    /acquisition/ElectricalSeries1
    /acquisition/ElectricalSeries2
    /acquisition/ElectricalSeries3

Analyzing ElectricalSeries at path: /acquisition/ElectricalSeries1
Data shape: [1000, 16]
Number of time points: 1000
Number of channels: 16

Channel Analysis:
----------------
   Channel          Mean         StdDev            Min            Max          Range
-------------------------------------------------------------------------------------
         0        0.0023        0.9876       -2.5432        2.6789        5.2221
         1        1.0628       44.3178     -175.2070      195.0000      370.2070
         2       -0.1245        1.2367       -3.4567        3.2109        6.6676
         3        0.5678        2.3456       -5.6789        6.7890       12.4679
...

Data unit: volts
Analysis complete.
```

Note: The actual output will depend on the content of the NWB file.
