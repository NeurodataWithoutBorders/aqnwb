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
import argparse
from typing import List, Any
from pynwb import NWBHDF5IO, NWBFile
import remfile
import h5py

def read_io(s3_path: str, aws_region: str, force_remfile: bool = False) -> NWBHDF5IO:
    """
    Opens the NWB file using NWBHDF5IO with ROS3 VFD.
    Equivalent to C++ read_io.

    :param s3_path: S3 URL of the NWB file.
    :param aws_region: AWS region (e.g., us-east-2).
    :param force_remfile: If True, remfile is used directly instead of attempting
                          to use the ROS3 driver. This is useful for benchmarking/comparing the
                          two different read strategies, or on systems where h5py was not built
                          with ROS3 support.
    :return: An instance of NWBHDF5IO.
    """
    def read_io_remfile(s3_path):
        print("Using remfile to read the NWB file from S3.")
        rem_file = remfile.File(s3_path)
        h5py_file = h5py.File(rem_file, "r")
        io = NWBHDF5IO(file=h5py_file)
        return io

    if force_remfile:
        return read_io_remfile(s3_path)

    # In PyNWB, NWBHDF5IO handles the HDF5 file opening and ROS3 configuration.
    try:
        return NWBHDF5IO(s3_path, mode='r', driver='ros3', aws_region=aws_region)
    except (ImportError, ValueError) as e:
        print("h5py with ROS3 support is required. Falling back to remfile.")
        return read_io_remfile(s3_path)


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

def main() -> None:
    parser = argparse.ArgumentParser(description="ROS3 read process benchmark (Python version)")
    parser.add_argument("s3_path", help="S3 URL of the NWB file")
    parser.add_argument("aws_region", help="AWS region (e.g., us-east-2)")
    parser.add_argument("object_name", help="Name of the object to find")
    parser.add_argument("start_indices", help="Comma-separated start indices (e.g., '0,0')")
    parser.add_argument("count_indices", help="Comma-separated count indices (e.g., '10,1')")
    parser.add_argument(
        "--force-remfile",
        action="store_true",
        help="Force the use of remfile instead of the ROS3 driver, even if ROS3 is available.",
    )

    args = parser.parse_args()

    try:
        start = [int(x) for x in args.start_indices.split(',')]
        count = [int(x) for x in args.count_indices.split(',')]
    except ValueError:
        print("Error: indices must be comma-separated integers.")
        exit(1)
    
    try:
        print("Benchmarking ROS3 read process (Python using PyNWB)...")
        
        total_start = time.perf_counter()
        
        # 1. read_io
        io_start = time.perf_counter()
        io = read_io(args.s3_path, args.aws_region, force_remfile=args.force_remfile)
        io_end = time.perf_counter()
        print(f"read_io took: {io_end - io_start:.6f} s")
        
        # 2. read_nwbfile
        nwb_start = time.perf_counter()
        nwb = read_nwbfile(io)
        nwb_end = time.perf_counter()
        print(f"read_nwbfile took: {nwb_end - nwb_start:.6f} s")
        
        # 3. find_object
        find_start = time.perf_counter()
        nwb_object = get_object_by_name(nwb, args.object_name)
        find_end = time.perf_counter()
        print(f"find_object took: {find_end - find_start:.6f} s")
        
        # 4. read_slice
        slice_start = time.perf_counter()
        data = read_slice(nwb_object, start, count)
        slice_end = time.perf_counter()
        print(f"read_slice took: {slice_end - slice_start:.6f} s")
        
        total_end = time.perf_counter()
        print(f"Total time taken: {total_end - total_start:.6f} s")
        print(f"Data read size: {data.size} elements")
        
        io.close()
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        exit(1)

if __name__ == "__main__":
    main()