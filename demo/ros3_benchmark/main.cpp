#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/RegisteredType.hpp"

using namespace AQNWB;
using namespace AQNWB::IO;
using namespace AQNWB::NWB;

// Function to create the HDF5IO object and open the file via openS3
std::shared_ptr<HDF5::HDF5IO> read_io(const std::string& s3Path, const std::string& awsRegion)
{
    auto readio = std::make_shared<HDF5::HDF5IO>(s3Path);
    Status status = readio->openS3(awsRegion);
    if (status != Status::Success) {
        throw std::runtime_error("Failed to open S3 file at " + s3Path + " in region " + awsRegion);
    }
    return readio;
}

// Function to create the NWBFile object using the IO object
std::shared_ptr<NWBFile> read_nwbfile(std::shared_ptr<HDF5::HDF5IO> readio)
{
    auto nwbFile = NWBFile::create("/", readio);
    if (!nwbFile) {
        throw std::runtime_error("Failed to create NWBFile object");
    }
    return nwbFile;
}

// Function to find the object in the NWB file using the provided name
std::shared_ptr<RegisteredType> find_object(std::shared_ptr<NWBFile> nwbFile, const std::string& objectName)
{
    // Search for all typed objects in the NWB file
    auto foundObjects = nwbFile->findOwnedTypes({}, IO::SearchMode::CONTINUE_ON_TYPE);
    
    for (const auto& [objPath, type] : foundObjects) {
        // Extract object name from path
        size_t lastSlash = objPath.find_last_of('/');
        std::string currObjectName = (lastSlash == std::string::npos) ? objPath : objPath.substr(lastSlash + 1);
        
        if (currObjectName == objectName) {
            // Create the RegisteredType object using the path and the IO object from the NWBFile
            return RegisteredType::create(objPath, nwbFile->getIO());
        }
    }
    
    throw std::runtime_error("Object with name '" + objectName + "' not found in the NWB file");
}

// Function to read the data from the object using the provided slice range
// This implementation uses the generic readField method to access the "data" dataset.
template<typename T>
std::vector<T> read_slice(std::shared_ptr<RegisteredType> object, const std::vector<size_t>& start, const std::vector<size_t>& count)
{
    auto readWrapper = object->readField<DatasetField, T>("data");
    if (!readWrapper) {
        throw std::runtime_error("Failed to read 'data' field from the object. Ensure it is a data-bearing object.");
    }

    SizeArray startArray(start.begin(), start.end());
    SizeArray countArray(count.begin(), count.end());
    
    auto dataSlice = readWrapper->values(startArray, countArray);
    return dataSlice.data;
}

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " <s3_path> <aws_region> <object_name> <start_indices> <count_indices>" << std::endl;
    std::cout << "Example: " << programName 
              << " https://bucket.s3.amazonaws.com/file us-east-1 my_timeseries \"0,0\" \"10,1\"" << std::endl;
    std::cout << "Note: indices should be comma-separated strings." << std::endl;
}

// Helper to parse comma separated string to vector of size_t
std::vector<size_t> parseIndices(const std::string& s)
{
    std::vector<size_t> indices;
    size_t pos = 0;
    std::string token;
    std::string temp = s;
    // Remove quotes if present
    if (!temp.empty() && temp.front() == '"') temp.erase(0, 1);
    if (!temp.empty() && temp.back() == '"') temp.pop_back();

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
    std::vector<size_t> start = parseIndices(argv[4]);
    std::vector<size_t> count = parseIndices(argv[5]);

    try {
        std::cout << "Benchmarking ROS3 read process..." << std::endl;

        auto start_total = std::chrono::high_resolution_clock::now();

        // 1. read_io
        auto start_io = std::chrono::high_resolution_clock::now();
        auto readio = read_io(s3Path, awsRegion);
        auto end_io = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_io = end_io - start_io;
        std::cout << "read_io took: " << elapsed_io.count() << " s" << std::endl;

        // 2. read_nwbfile
        auto start_nwb = std::chrono::high_resolution_clock::now();
        auto nwbFile = read_nwbfile(readio);
        auto end_nwb = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_nwb = end_nwb - start_nwb;
        std::cout << "read_nwbfile took: " << elapsed_nwb.count() << " s" << std::endl;

        // 3. find_object
        auto start_find = std::chrono::high_resolution_clock::now();
        auto object = find_object(nwbFile, objectName);
        auto end_find = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_find = end_find - start_find;
        std::cout << "find_object took: " << elapsed_find.count() << " s" << std::endl;

        // 4. read_slice (Assuming int16_t for benchmark, can be adjusted or made an argument)
        auto start_slice = std::chrono::high_resolution_clock::now();
        auto data = read_slice<int16_t>(object, start, count);
        auto end_slice = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_slice = end_slice - start_slice;
        std::cout << "read_slice took: " << elapsed_slice.count() << " s" << std::endl;

        auto end_total = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_total = end_total - start_total;
        std::cout << "Total time taken: " << elapsed_total.count() << " s" << std::endl;
        std::cout << "Data read size: " << data.size() << " elements" << std::endl;

        readio->close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}