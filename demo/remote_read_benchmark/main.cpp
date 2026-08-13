/**
 * @file main.cpp
 * @brief Remote Read Benchmark (C++ Version).
 *
 * This program benchmarks reading NWB data from Amazon S3 with the AQNWB
 * library using either the HDF5 ROS3 VFD or the remfile VFD. It measures the
 * time taken for:
 * 1. Opening the S3 file via HDF5IO.
 * 2. Initializing the NWBFile object.
 * 3. Finding a specific neurodata object by name.
 * 4. Reading a slice of data from that object.
 *
 * This provides a direct comparison point for the Python benchmark
 * implementation, which also supports ROS3 and remfile.
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/base/TimeSeries.hpp"

using namespace AQNWB;
using namespace AQNWB::IO;
using namespace AQNWB::NWB;

/**
 * @brief Creates the HDF5IO object and opens the file with the
 *        requested driver ("ros3" or "remfile").
 *
 * @param s3Path The S3 URL of the NWB file.
 * @param awsRegion The AWS region (e.g., us-east-1).
 * @param driver The VFD driver to use ("ros3" or "remfile")
 * @return A shared pointer to the initialized HDF5IO object.
 * @throws std::runtime_error If the S3 file cannot be opened.
 */
std::shared_ptr<HDF5::HDF5IO> read_io(const std::string& s3Path,
                                      const std::string& awsRegion,
                                      const std::string& driver)

{
  auto readio = std::make_shared<HDF5::HDF5IO>(s3Path);
  Status status = Status::Failure;
  if (driver == "ros3") {
#ifdef H5_HAVE_ROS3_VFD
    status = readio->openS3(awsRegion);
#else
    throw std::runtime_error("HDF5 was built without ROS3 VFD support");
#endif
  } else if (driver == "remfile") {
#ifdef AQNWB_HAVE_REMFILE_VFD
    status = readio->openRemote();
#else
    throw std::runtime_error("aqnwb was built without remfile VFD support");
#endif
  } else {
    throw std::runtime_error("Unknown driver '" + driver
                             + "' (expected 'ros3' or 'remfile')");
  }
  if (status != Status::Success) {
    throw std::runtime_error("Failed to open file at " + s3Path
                             + " with driver " + driver);
  }
  return readio;
}

/**
 * @brief Creates the NWBFile object using the provided IO object.
 *
 * @param readio A shared pointer to the HDF5IO object.
 * @return A shared pointer to the created NWBFile object.
 * @throws std::runtime_error If the NWBFile object cannot be created.
 */
std::shared_ptr<NWBFile> read_nwbfile(std::shared_ptr<HDF5::HDF5IO> readio)
{
  auto nwbFile = NWBFile::create("/", readio);
  if (!nwbFile) {
    throw std::runtime_error("Failed to create NWBFile object");
  }
  return nwbFile;
}

/**
 * @brief Finds a specific object in the NWB file by its name.
 *
 * @param nwbFile A shared pointer to the NWBFile object.
 * @param objectName The name of the object to search for.
 * @return A shared pointer to the found RegisteredType object.
 * @throws std::runtime_error If the object is not found in the NWB file.
 */
std::shared_ptr<RegisteredType> find_object(std::shared_ptr<NWBFile> nwbFile,
                                            const std::string& objectName)
{
  // Search for all typed objects in the NWB file
  std::string objPath = nwbFile->findOwnedObject(objectName);
  if (!objPath.empty()) {
    return RegisteredType::create(objPath, nwbFile->getIO());
  } else {
    throw std::runtime_error("Object with name '" + objectName
                             + "' not found in the NWB file");
  }
}

/**
 * @brief Reads a slice of data from the object's "data" field.
 *
 * This implementation uses the generic readField method to access the "data"
 * dataset.
 *
 * @tparam T The data type of the elements being read.
 * @param object A shared pointer to the RegisteredType object.
 * @param start The starting indices for each dimension.
 * @param count The number of elements to read for each dimension.
 * @return A vector containing the read data slice.
 * @throws std::runtime_error If the "data" field cannot be read.
 */
template<typename T>
std::vector<T> read_slice(std::shared_ptr<RegisteredType> object,
                          const SizeArray& start,
                          const SizeArray& count)
{
  auto readWrapper = object->readField<DatasetField, T>("data");
  if (!readWrapper) {
    throw std::runtime_error(
        "Failed to read 'data' field from the object. Ensure it is a "
        "data-bearing object.");
  }
  auto dataSlice = readWrapper->values(start, count);
  return dataSlice.data;
}

/**
 * @brief Prints the usage instructions for the benchmark program.
 *
 * @param programName The name of the program to be displayed in the usage
 * message.
 */
void printUsage(const char* programName)
{
  std::cout << "Usage: " << programName
            << " <s3_path> <aws_region> <object_name> <start_indices> "
               "<count_indices> [driver]"
            << std::endl;
  std::cout << "Example: " << programName
            << " https://bucket.s3.amazonaws.com/file us-east-1 my_timeseries "
               "\"0,0\" \"10,1\""
            << std::endl;
  std::cout << "Note: indices should be comma-separated strings." << std::endl;
  std::cout << "driver: 'ros3' (default) or 'remfile'. The aws_region argument "
               "is ignored by remfile."
            << std::endl;
}

/**
 * @brief Parses a comma-separated string of integers into a SizeArray.
 *
 * This helper function removes optional quotes from the input string and
 * converts comma-separated values into size_t.
 *
 * @param s The input string containing comma-separated indices (e.g., "0,0").
 * @return A SizeArray (vector of size_t) containing the parsed indices.
 * @throws std::invalid_argument or std::out_of_range if string conversion
 * fails.
 */
SizeArray parseIndices(const std::string& s)
{
  SizeArray indices;
  size_t pos = 0;
  std::string token;
  std::string temp = s;
  // Remove quotes if present
  if (!temp.empty() && temp.front() == '"')
    temp.erase(0, 1);
  if (!temp.empty() && temp.back() == '"')
    temp.pop_back();

  while ((pos = temp.find(',')) != std::string::npos) {
    token = temp.substr(0, pos);
    indices.push_back(std::stoul(token));
    temp.erase(0, pos + 1);
  }
  indices.push_back(std::stoul(temp));
  return indices;
}

int main(int argc, char* argv[])
{
  if (argc < 6) {
    printUsage(argv[0]);
    return 1;
  }

  std::string s3Path = argv[1];
  std::string awsRegion = argv[2];
  std::string objectName = argv[3];
  SizeArray start = parseIndices(argv[4]);
  SizeArray count = parseIndices(argv[5]);
  std::string driver = (argc > 6) ? argv[6] : "ros3";

  try {
    std::cout << "Benchmarking " << driver << " read process..." << std::endl;

    auto start_total = std::chrono::high_resolution_clock::now();

    // 1. read_io
    auto start_io = std::chrono::high_resolution_clock::now();
    auto readio = read_io(s3Path, awsRegion, driver);
    auto end_io = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_io = end_io - start_io;
    std::cout << "read_io took: " << elapsed_io.count() << " s" << std::endl;

    // 2. read_nwbfile
    auto start_nwb = std::chrono::high_resolution_clock::now();
    auto nwbFile = read_nwbfile(readio);
    auto end_nwb = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_nwb = end_nwb - start_nwb;
    std::cout << "read_nwbfile took: " << elapsed_nwb.count() << " s"
              << std::endl;

    // 3. find_object
    auto start_find = std::chrono::high_resolution_clock::now();
    auto object = find_object(nwbFile, objectName);
    auto end_find = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_find = end_find - start_find;
    std::cout << "find_object took: " << elapsed_find.count() << " s"
              << std::endl;

    // 4. read_slice (Assuming int16_t for benchmark, can be adjusted or made an
    // argument)
    auto start_slice = std::chrono::high_resolution_clock::now();
    auto data = read_slice<int16_t>(object, start, count);
    auto end_slice = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_slice = end_slice - start_slice;
    std::cout << "read_slice took: " << elapsed_slice.count() << " s"
              << std::endl;

    auto end_total = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_total = end_total - start_total;
    std::cout << "Total time taken: " << elapsed_total.count() << " s"
              << std::endl;
    std::cout << "Data read size: " << data.size() << " elements" << std::endl;

    readio->close();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
