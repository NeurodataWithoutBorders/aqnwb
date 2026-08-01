# Remote Read Benchmark Demo

This is a simple C++ demo project that benchmarks the process of reading data from a NWB (Neurodata Without Borders) file stored on Amazon S3 using the aqnwb library, using either the HDF5 ROS3 VFD or the [remfile-cpp](https://github.com/catalystneuro/remfile-cpp) VFD.

The benchmark measures the time taken for each of the following steps:
1. **`read_io`**: Creating the HDF5IO object and opening the S3 file.
2. **`read_nwbfile`**: Creating the NWBFile object.
3. **`find_object`**: Locating a specific object in the NWB file by name.
4. **`read_slice`**: Reading a specific slice of data from the object.

## Prerequisites

- CMake (version 3.15 or higher)
- C++ compiler with C++17 support
- aqnwb library (installed from the parent project)
- HDF5 installed with ROS3 VFD support
- Python 3.x and [uv](https://github.com/astral-sh/uv) (recommended for running the Python benchmark)

## Building the Demo

1. Make sure you have built the main aqnwb project first 
   following the instructions in the AqNWB documentation,
   including installing the aqnwb library to your system
   or another local path.

2. Create a build directory for the demo:

```bash
cd demo/remote_read_benchmark
mkdir -p build
cd build
```

3. Configure and build the demo:

```bash
# If aqnwb was installed to the system then it will be found automatically
cmake ..

# Otherwise, provide the path to the aqnwb install:
cmake .. -DCMAKE_PREFIX_PATH=/path/to/aqnwb/install

# Build the project
cmake --build .
```

Note: If HDF5 is installed in a non-standard location, you will also need to provide the HDF5 root path:
`cmake .. -DCMAKE_PREFIX_PATH="/path/to/aqnwb/install" -DHDF5_ROOT="/path/to/hdf5/install"`

## Running the Demo

After building, you can run the benchmark using a file on S3.

```bash
cd demo/remote_read_benchmark/build/bin
./remote_read_benchmark <s3_path> <aws_region> <object_name> <start_indices> <count_indices> [driver]
```

The optional `driver` argument selects the HDF5 virtual file driver: `ros3`
(the default) or `remfile`. The remfile driver requires aqnwb to be built
with remfile support (`AQNWB_USE_REMFILE=ON`, the default on non-Windows
platforms); it ignores the `aws_region` argument and works with any HTTP(S)
server that supports byte-range requests, not just S3.

### Example

You can use the following parameters to test with a known DANDI archive file:

```bash
cd demo/remote_read_benchmark/build/bin
./remote_read_benchmark \
    "https://dandiarchive.s3.amazonaws.com/blobs/fec/8a6/fec8a690-2ece-4437-8877-8a002ff8bd8a" \
    "us-east-2" \
    "ElectricalSeriesAp" \
    "0,0" \
    "10,1"
```

To run the same benchmark with the remfile driver, append `remfile`:

```bash
./remote_read_benchmark \
    "https://dandiarchive.s3.amazonaws.com/blobs/fec/8a6/fec8a690-2ece-4437-8877-8a002ff8bd8a" \
    "us-east-2" \
    "ElectricalSeriesAp" \
    "0,0" \
    "10,1" \
    remfile
```

## Python Benchmark

A Python equivalent of the benchmark is provided as `benchmark.py`. This script uses PyNWB and h5py to perform the same sequence of operations and measure their timing.

### Running the Python Benchmark

The script is configured for use with `uv`, which handles dependency installation automatically.

```bash
uv run demo/remote_read_benchmark/benchmark.py \
    "https://dandiarchive.s3.amazonaws.com/blobs/fec/8a6/fec8a690-2ece-4437-8877-8a002ff8bd8a" \
    "us-east-2" \
    "ElectricalSeriesAp" \
    "0,0" \
    "10,1"
```

By default, the script will attempt to use the ROS3 driver (falling back to
`remfile` if h5py was not built with ROS3 support). To force the use of
`remfile` instead of the ROS3 driver (e.g. to compare the performance of
the two read strategies), pass the `--force-remfile` flag.

For automation, both the C++ and Python benchmarks also support machine-readable
JSON output:

```bash
./remote_read_benchmark \
    "https://dandiarchive.s3.amazonaws.com/blobs/fec/8a6/fec8a690-2ece-4437-8877-8a002ff8bd8a" \
    "us-east-2" \
    "ElectricalSeriesAp" \
    "0,0" \
    "10,1" \
    --json

python demo/remote_read_benchmark/benchmark.py \
    "https://dandiarchive.s3.amazonaws.com/blobs/fec/8a6/fec8a690-2ece-4437-8877-8a002ff8bd8a" \
    "us-east-2" \
    "ElectricalSeriesAp" \
    "0,0" \
    "10,1" \
    --driver ros3 \
    --strict-driver \
    --output-format json
```

The helper scripts `run_benchmark_matrix.py` and `summarize_benchmark_results.py`
are used by the dedicated GitHub Actions performance workflow to run all
benchmark variants repeatedly and render markdown summary tables.

## Code Structure

- `main.cpp`: Contains the C++ benchmarking logic and timing measurements.
- `CMakeLists.txt`: CMake configuration file for building the C++ project.
- `benchmark.py`: Contains the Python benchmarking logic using PyNWB.
- `run_benchmark_matrix.py`: Repeats all benchmark variants for one HDF5 environment and writes raw JSON results.
- `summarize_benchmark_results.py`: Combines JSON artifacts into markdown summary tables.
