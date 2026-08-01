# /// script
# dependencies = [
#     "h5py",
#     "pynwb",
#     "numpy",
#     "remfile"
# ]
# ///

"""
ROS3 Read Process Benchmark (Python Version). 
This provides a direct comparison point for the C++ benchmark implementation
here. NOTE: This script is only intended as a simple comparison point for 
demo purposed. It is not intended to be a fully featured benchmark script.
For fully featured python benchmarking, please see https://nwb-benchmarks.readthedocs.io/

This script benchmarks the performance of reading NWB data from Amazon S3 using 
the PyNWB library and h5py with the ROS3 VFD. It measures the time taken for:
1. Opening the S3 file via NWBHDF5IO.
2. Initializing the NWBFile object.
3. Finding a specific neurodata object by name.
4. Reading a slice of data from that object.

"""

import time
import json
import argparse
from typing import List, Any, Tuple, Dict
from pynwb import NWBHDF5IO, NWBFile
import remfile
import h5py

def read_io(s3_path: str, aws_region: str, driver: str, strict_driver: bool = False) -> Tuple[NWBHDF5IO, str]:
    """
    Opens the NWB file using NWBHDF5IO with the requested driver.
    Equivalent to C++ read_io.

    :param s3_path: S3 URL of the NWB file.
    :param aws_region: AWS region (e.g., us-east-2).
    :param driver: Requested driver ("ros3" or "remfile").
    :param strict_driver: If True, raise an error instead of falling back to remfile.
    :return: A tuple of the NWBHDF5IO instance and the actual driver used.
    """
    def read_io_remfile(remote_path: str) -> NWBHDF5IO:
        rem_file = remfile.File(remote_path)
        h5py_file = h5py.File(rem_file, "r")
        io = NWBHDF5IO(file=h5py_file)
        return io

    if driver == "remfile":
        return read_io_remfile(s3_path), "remfile"

    if driver != "ros3":
        raise ValueError(f"Unknown driver '{driver}' (expected 'ros3' or 'remfile')")

    # In PyNWB, NWBHDF5IO handles the HDF5 file opening and ROS3 configuration.
    try:
        return NWBHDF5IO(s3_path, mode='r', driver='ros3', aws_region=aws_region), "ros3"
    except (ImportError, ValueError) as e:
        if strict_driver:
            raise RuntimeError("h5py with ROS3 support is required for the requested ROS3 benchmark.") from e
        print("h5py with ROS3 support is required. Falling back to remfile.")
        return read_io_remfile(s3_path), "remfile"


def read_nwbfile(io: NWBHDF5IO) -> NWBFile:
    """
    Initializes the PyNWB NWBFile object from the IO object.
    Equivalent to C++ read_nwbfile.

    :param io: The NWBHDF5IO instance used to read the file.
    :return: The initialized NWBFile object.
    """
    nwb = io.read()
    return nwb

def get_object_by_name(nwb: NWBFile, object_name: str) -> Any:
    """
    Simple helper function to retrieve a neurodata object by its name, if it is unique.
    Equivalent to C++ find_object.

    :param nwb: The NWBFile object to search.
    :param object_name: The name of the object to retrieve.
    :return: The matching neurodata object.
    :raises ValueError: If the object is not found or if multiple objects with the same name exist.
    """
    # Find all objects that are matching the given name
    matching_objects = [
        (neurodata_object.name, neurodata_object)
        for neurodata_object in nwb.objects.values()
        if neurodata_object.name == object_name
    ]
    # Raise an error if the object wasn't found
    if len(matching_objects) == 0:
        raise ValueError(f"The specified object name ({object_name}) is not in the NWBFile.")
    # Make sure that the object we are looking for is unique
    elif len(matching_objects) > 1:
        raise ValueError(f"The specified object name ({object_name}) was found multiple times in the NWBFile.")
    # Return the matching object
    return matching_objects[0][1]

def read_slice(nwb_object: Any, start: List[int], count: List[int]) -> Any:
    """
    Reads a slice of data from the object.
    Equivalent to C++ read_slice.

    :param nwb_object: The neurodata object from which to read data.
    :param start: A list of start indices for each dimension.
    :param count: A list of count indices for each dimension.
    :return: The read data slice (typically as a numpy array).
    """
    # In PyNWB, data is typically accessed via the .data attribute
    dataset = nwb_object.data
    
    # Create slices for each dimension
    slices = []
    for s, c in zip(start, count):
        slices.append(slice(s, s + c))
    
    return dataset[tuple(slices)]


def format_benchmark_result(
    requested_driver: str,
    actual_driver: str,
    timings_seconds: Dict[str, float],
    data_size_elements: int,
) -> Dict[str, Any]:
    return {
        "implementation": "python",
        "requested_driver": requested_driver,
        "actual_driver": actual_driver,
        "timings_seconds": timings_seconds,
        "data_size_elements": data_size_elements,
    }


def print_text_result(result: Dict[str, Any]) -> None:
    print("Benchmarking remote read process (Python using PyNWB)...")
    print(f"Requested driver: {result['requested_driver']}")
    print(f"Actual driver: {result['actual_driver']}")
    print(f"read_io took: {result['timings_seconds']['read_io']:.6f} s")
    print(f"read_nwbfile took: {result['timings_seconds']['read_nwbfile']:.6f} s")
    print(f"find_object took: {result['timings_seconds']['find_object']:.6f} s")
    print(f"read_slice took: {result['timings_seconds']['read_slice']:.6f} s")
    print(f"Total time taken: {result['timings_seconds']['total']:.6f} s")
    print(f"Data read size: {result['data_size_elements']} elements")

def main() -> None:
    parser = argparse.ArgumentParser(description="ROS3 read process benchmark (Python version)")
    parser.add_argument("s3_path", help="S3 URL of the NWB file")
    parser.add_argument("aws_region", help="AWS region (e.g., us-east-2)")
    parser.add_argument("object_name", help="Name of the object to find")
    parser.add_argument("start_indices", help="Comma-separated start indices (e.g., '0,0')")
    parser.add_argument("count_indices", help="Comma-separated count indices (e.g., '10,1')")
    parser.add_argument(
        "--driver",
        choices=("ros3", "remfile"),
        default="ros3",
        help="Requested driver to benchmark. Defaults to ros3.",
    )
    parser.add_argument(
        "--force-remfile",
        action="store_true",
        help="Force the use of remfile instead of the ROS3 driver, even if ROS3 is available.",
    )
    parser.add_argument(
        "--strict-driver",
        action="store_true",
        help="Fail instead of falling back to remfile when the requested driver is unavailable.",
    )
    parser.add_argument(
        "--output-format",
        choices=("text", "json"),
        default="text",
        help="Choose human-readable text or machine-readable JSON output.",
    )

    args = parser.parse_args()

    if args.force_remfile:
        requested_driver = "remfile"
    else:
        requested_driver = args.driver

    try:
        start = [int(x) for x in args.start_indices.split(',')]
        count = [int(x) for x in args.count_indices.split(',')]
    except ValueError:
        print("Error: indices must be comma-separated integers.")
        exit(1)
    
    try:
        total_start = time.perf_counter()

        # 1. read_io
        io_start = time.perf_counter()
        io, actual_driver = read_io(
            args.s3_path,
            args.aws_region,
            driver=requested_driver,
            strict_driver=args.strict_driver,
        )
        io_end = time.perf_counter()

        # 2. read_nwbfile
        nwb_start = time.perf_counter()
        nwb = read_nwbfile(io)
        nwb_end = time.perf_counter()

        # 3. find_object
        find_start = time.perf_counter()
        nwb_object = get_object_by_name(nwb, args.object_name)
        find_end = time.perf_counter()

        # 4. read_slice
        slice_start = time.perf_counter()
        data = read_slice(nwb_object, start, count)
        slice_end = time.perf_counter()

        total_end = time.perf_counter()
        result = format_benchmark_result(
            requested_driver=requested_driver,
            actual_driver=actual_driver,
            timings_seconds={
                "read_io": io_end - io_start,
                "read_nwbfile": nwb_end - nwb_start,
                "find_object": find_end - find_start,
                "read_slice": slice_end - slice_start,
                "total": total_end - total_start,
            },
            data_size_elements=int(data.size),
        )

        if args.output_format == "json":
            print(json.dumps(result))
        else:
            print_text_result(result)

        io.close()

    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        exit(1)

if __name__ == "__main__":
    main()