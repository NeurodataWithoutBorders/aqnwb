# NWB Analysis Demo

This is a simple C++ demo project that shows how to read and analyze NWB (Neurodata Without Borders) files using the aqnwb library. The demo reads the file `sub-EF0147_ses-20190204T144339_behavior+ecephys.nwb` and performs basic statistical analysis on the electrophysiology data.

## Prerequisites

- CMake (version 3.15 or higher)
- C++ compiler with C++17 support
- HDF5 library with C++ support
- Boost library
- aqnwb library (built from the parent project)

## Download an example NWB dataset that contains ElectricalSeries data

E.g. https://api.dandiarchive.org/api/assets/4c440a73-9250-45bb-a342-a7da4d01b2fd/download?content_disposition=inline to
download a single file from DANDIset 000232 at https://dandiarchive.org/dandiset/000232/0.240510.2038

Alternatively, we can also run AqNWB test suite and use a suitable file from the tests, e.g,. 
`build/dev/tests/data/createElectricalSeries.nwb`

## Building the Demo

1. Make sure you have built the main aqnwb project first 
   following the instructions in the AqNWB documentation

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
./nwb_analysis_demo ../../sub-EF0147_ses-20190204T144339_behavior+ecephys.nwb
```

The program will:

1. Open the NWB file in read-only mode
2. Search for ElectricalSeries objects in the file
3. Read the data from the first ElectricalSeries found and print basic metadata
4. Calculate and display statistics for each channel of the data:
   - Mean
   - Standard deviation
   - Minimum value
   - Maximum value
   - Range


## Code Structure

- `main.cpp`: Contains the main program logic
- `CMakeLists.txt`: CMake configuration file for building the project

## Example Output

```
Opening NWB file: ../../sub-EF0147_ses-20190204T144339_behavior+ecephys.nwb
Searching for ElectricalSeries objects...
Number of ElectricalSeries: 244
    /processing/ecephys/LFP/ElectricalSeries_97
    /processing/ecephys/LFP/ElectricalSeries_96
    /processing/ecephys/LFP/ElectricalSeries_94
     ...
    /processing/ecephys/LFP/ElectricalSeries_1

Analyzing ElectricalSeries at path: /processing/ecephys/LFP/ElectricalSeries_97
Data shape: [7409, 64]
Number of time points: 7409
Number of channels: 64
Data unit: volts
Data description: no description

Channel Analysis:
   Channel           Mean         StdDev            Min            Max          Range
-------------------------------------------------------------------------------------
         0         1.0628        44.3178      -175.2075       195.0000       370.2075
         1         0.9844        57.7333      -257.3025       235.4625       492.7650
         2         1.2631        45.6507      -177.7425       210.6975       388.4400
         3         1.2300        57.5107      -244.2375       239.4600       483.6975
...

Analysis complete.
```

Note: The actual output will depend on the content of the NWB file.
