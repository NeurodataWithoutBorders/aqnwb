#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/RegisteredType.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"

using namespace AQNWB;
using namespace AQNWB::IO;
using namespace AQNWB::NWB;

// Function to calculate mean of a vector
template<typename T>
double calculateMean(const std::vector<T>& data)
{
  if (data.empty()) {
    return 0.0;
  }
  double sum = std::accumulate(data.begin(), data.end(), 0.0);
  return sum / data.size();
}

// Function to calculate standard deviation of a vector
template<typename T>
double calculateStdDev(const std::vector<T>& data, double mean)
{
  if (data.empty()) {
    return 0.0;
  }
  double variance = 0.0;
  for (const auto& value : data) {
    variance += std::pow(value - mean, 2);
  }
  variance /= (data.size() - 1);
  return std::sqrt(variance);
}

// Function to compute the mean using std::visit. This allows us to
// compute the mean for a wide range of different data types without
// having to specify the data type ourselves. The data is encapsulated
// in a BaseDataType::BaseDataVectorVariant, which is a std::variant that
// can represent any std::vector of BaseDataType values.
inline double calculateMeanFromVariant(const BaseDataType::BaseDataVectorVariant& variant)
{
  return std::visit(
      [](auto&& arg) -> double
      {
        using T = std::decay_t<decltype(arg)>;
        // Check that the variant represents a BaseDataType we can compute on
        if constexpr (std::is_same_v<T, std::monostate>) {
          throw std::runtime_error("Invalid data type");
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
          throw std::runtime_error("Cannot compute mean of string data");
        } else {
          return calculateMean(arg);  // Compute the mean
        }
      },
      variant);
}

// Simple helper function for marking text to print in bold
inline std::string bold(const std::string& str)
{
  return "\033[1m" + str + "\033[0m";
}

// Helper function to print usage instructions for this demo application
void printUsage(const char* programName)
{
  std::cout << "Usage: " << programName << " <path_to_nwb_file>" << std::endl;
  std::cout << "Example: " << programName
            << " ../../sub-EF0147_ses-20190204T144339_behavior+ecephys.nwb"
            << std::endl;
  std::cout << std::endl;
  std::cout << "This program reads an NWB file and performs basic statistical "
               "analysis on the ElectricalSeries data."
            << std::endl;
  std::cout << "It displays channel statistics and basic metadata."
            << std::endl;
}

// The main routine implementing the main workflow of this demo application
int main(int argc, char* argv[])
{
  // Check if a file path was provided
  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }

  // Path to the NWB file from the command line arguments
  std::string filePath = argv[1];

  // Check if the file exists
  if (!std::filesystem::exists(filePath)) {
    std::cerr << "Error: File not found: " << filePath << std::endl;
    std::cerr << "Please provide a valid path to an NWB file." << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // Print progress status
  std::cout << std::endl << bold("Opening NWB file: ") << filePath << std::endl;
 
  // Create an IO object for reading the NWB file
  std::shared_ptr<BaseIO> io = createIO("HDF5", filePath);
  io->open(FileMode::ReadOnly);

  // Read the NWB file
  auto nwbfile = AQNWB::NWB::NWBFile::create("/", io);

  // Search for ElectricalSeries objects in the file
  // Use search mode to CONTINUE_ON_TYPE to also search inside processing modules
  std::cout << bold("Searching for ElectricalSeries objects...") << std::endl;
  std::unordered_set<std::string> typesToSearch = {"core::ElectricalSeries"};
  std::unordered_map<std::string, std::string> foundElectricalSeries =
      nwbfile->findOwnedTypes(typesToSearch, IO::SearchMode::CONTINUE_ON_TYPE);
  if (foundElectricalSeries.empty()) {
    std::cout << "No ElectricalSeries found in the file." << std::endl;
    io->close();
    return 1;
  }

  // Print appreviated list of the paths of found ElectricalSeries by printing
  // the first 3 paths and the last path
  std::cout << bold("Number of ElectricalSeries: ")
            << foundElectricalSeries.size() << std::endl;
  size_t count = 0;
  size_t max_count = 3;
  for (const auto& pair : foundElectricalSeries) {
    if (count < max_count) {
      std::cout << "    " << pair.first << std::endl;
    } else if (count == max_count && foundElectricalSeries.size() > max_count)
    {
      std::cout << "     ..." << std::endl;
    } else if (count == foundElectricalSeries.size() - 1) {
      std::cout << "    " << pair.first << std::endl;
    }
    ++count;
  }
  std::cout << std::endl;

  // Read the first ElectricalSeries found
  std::string esPath = foundElectricalSeries.begin()->first;
  std::cout << bold("Analyzing ElectricalSeries at: ") << esPath << std::endl;

  auto electricalSeries = AQNWB::NWB::ElectricalSeries::create(esPath, io);

  // Read the data lazily. Here we only create the wrapper for reading 
  // but we do not yet load any actual data from disk here
  auto dataWrapper = electricalSeries->readData();

  // We can now inspect the data shape before loading the data
  auto fullDataShape = dataWrapper->getShape();

  // Get the number of time points and channels from the shape
  SizeType numTimePoints = fullDataShape[0];
  SizeType numChannels = fullDataShape.size() > 1 ? fullDataShape[1] : 1;
  std::cout << bold("Number of time points: ") << numTimePoints << std::endl;
  std::cout << bold("Number of channels: ") << numChannels << std::endl;

  // Read the unit of the data
  std::string unit = electricalSeries->readDataUnit()->values().data[0];
  std::cout << bold("Data unit: ") << unit << std::endl;

  // Read the description of the data
  std::string description =
      electricalSeries->readDescription()->values().data[0];
  std::cout << bold("Data description: ") << description << std::endl;

  // Calculate the global mean. Here we DataBlockGeneric::as_variant() 
  // to compute on the data as a std::variant so that we do not need to 
  // determine the exact numeric type that our ElectricalSeries is using
  auto genericData = dataWrapper->valuesGeneric();
  auto variantData = genericData.as_variant();
  double meanFromVariant = calculateMeanFromVariant(variantData);
  std::cout << bold("Global mean: ") << meanFromVariant << " " << unit << std::endl;

  // In the following part we use the typed multi-array view and `DataBlock`
  // to compute statistics on a per-channel basis. These classes require that
  // the data type is specified. We here use the default behavior for
  // `ElectricalSeries.readData`, which defaults to float data consistent with
  // the NWB schema. To generalize this example to work with a wider range
  // of data types, we could use the same std::variant approach as shown for the
  // global mean, or use a switch/case approach to call the analysis with
  // the approbriate type depending on the value of either genericData.typeIndex
  // or genericData.baseDataType.
  if (genericData.typeIndex == typeid(float))
  {
    // Load data as DataBlock<float>
    auto dataValues = dataWrapper->values();

    // Convert to a multi-array view for easier access
    auto dataArray = dataValues.as_multi_array<2>();

    // Print status update
    std::cout << std::endl << bold("Channel Analysis:") << std::endl;

    // Store statistics for all channels
    std::vector<double> means(numChannels);
    std::vector<double> stdDevs(numChannels);
    std::vector<float> minVals(numChannels);
    std::vector<float> maxVals(numChannels);
    std::vector<float> ranges(numChannels);

    // Calculate statistics for all channels
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      // Extract data for this channel
      std::vector<float> channelData(numTimePoints);
      for (SizeType t = 0; t < numTimePoints; ++t) {
        channelData[t] = dataArray[t][ch];
      }

      // Calculate statistics
      means[ch] = calculateMean(channelData);
      stdDevs[ch] = calculateStdDev(channelData, means[ch]);

      // Find min and max values and range
      auto minmax = std::minmax_element(channelData.begin(), channelData.end());
      minVals[ch] = *minmax.first;
      maxVals[ch] = *minmax.second;
      ranges[ch] = maxVals[ch] - minVals[ch];
    }

    // Print the table header
    std::cout << std::setw(10) << "Channel" << std::setw(15)
              << "Mean" << std::setw(15) << "StdDev"
              << std::setw(15) << "Min" << std::setw(15) << "Max"
              << std::setw(15) << "Range" << std::endl;

    std::cout << std::string(85, '-') << std::endl;

    // Print the table rows
    for (SizeType ch = 0; ch < numChannels; ++ch) {
      std::cout << std::setw(10) << ch << std::setw(15) << std::fixed
                << std::setprecision(4) << means[ch] << std::setw(15)
                << std::fixed << std::setprecision(4) << stdDevs[ch]
                << std::setw(15) << std::fixed << std::setprecision(4)
                << minVals[ch] << std::setw(15) << std::fixed
                << std::setprecision(4) << maxVals[ch] << std::setw(15)
                << std::fixed << std::setprecision(4) << ranges[ch]
                << std::endl;
    }
  }
  else
  {
    // Print status update indicating that per-channel analysi was skipped
    std::cout << bold("Skipping Channel Analysis:")
              << " The per channel analysis assumes float data found "
              << genericData.typeIndex.name() << std::endl;
  }
  // Close the file
  io->close();
  // Final progress status
  std::cout << std::endl << bold("Analysis complete.") << std::endl;

  return 0;
}
