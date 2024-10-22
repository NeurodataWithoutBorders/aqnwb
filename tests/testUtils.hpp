
#pragma once

#include <filesystem>
#include <random>

#include <H5Cpp.h>
#include <catch2/catch_test_macros.hpp>

#include "Channel.hpp"
#include "Types.hpp"
#include "io/hdf5/HDF5IO.hpp"

using namespace AQNWB;
using namespace AQNWB::IO;
namespace fs = std::filesystem;

inline std::string getTestFilePath(std::string filename)
{
  // create data directory if it doesn't exist
  fs::path dirPath = fs::current_path() / "data";
  fs::directory_entry dir(dirPath);
  if (!dir.exists()) {
    fs::create_directory(dir);
  }

  // get filename and remove old file
  fs::path filepath = dirPath / filename;
  if (fs::exists(filepath)) {
    fs::remove(filepath);
  }

  return filepath.u8string();
}

inline std::vector<Types::ChannelVector> getMockChannelArrays(
    SizeType numChannels = 2,
    SizeType numArrays = 2,
    std::string groupName = "array")
{
  std::vector<Types::ChannelVector> arrays(numArrays);
  for (SizeType i = 0; i < numArrays; i++) {
    std::vector<Channel> chGroup;
    for (SizeType j = 0; j < numChannels; j++) {
      Channel ch("ch" + std::to_string(j),
                 groupName + std::to_string(i),
                 i,
                 j,
                 i * numArrays + j);
      chGroup.push_back(ch);
    }
    arrays[i] = chGroup;
  }
  return arrays;
}

inline std::vector<std::string> getMockChannelArrayNames(
    std::string baseName = "esdata", SizeType numArrays = 2)
{
  std::vector<std::string> arrayNames(numArrays);
  for (SizeType i = 0; i < numArrays; i++) {
    arrayNames[i] = baseName + std::to_string(i);
  }
  return arrayNames;
}

inline std::vector<float> getMockData1D(SizeType numSamples = 1000)
{
  std::vector<float> mockData(numSamples);

  std::random_device rd;
  std::mt19937 rng(rd());  // random number generator
  std::uniform_real_distribution<> dis(-1.0, 1.0);  // range of floats

  for (auto& data : mockData) {
    data = static_cast<float>(dis(rng))
        * 1000.f;  // approximate microvolt unit range
  }

  return mockData;
}

inline std::vector<std::vector<float>> getMockData2D(SizeType numSamples = 1000,
                                                     SizeType numChannels = 4)
{
  std::vector<std::vector<float>> mockData(numChannels,
                                           std::vector<float>(numSamples));

  std::random_device rd;
  std::mt19937 rng(rd());  // random number generator
  std::uniform_real_distribution<> dis(-1.0, 1.0);  // range of floats

  for (auto& channelData : mockData) {
    for (auto& data : channelData) {
      data = static_cast<float>(dis(rng))
          * 100.f;  // approximate microvolt unit range
    }
  }

  return mockData;
}

inline std::vector<double> getMockTimestamps(SizeType numSamples = 1000,
                                             SizeType samplingRate = 30000)
{
  std::vector<double> mockTimestamps(numSamples);
  double samplingPeriod = 1.0 / samplingRate;

  for (SizeType i = 0; i < numSamples; ++i) {
    mockTimestamps[i] = i * samplingPeriod;  // Each timestamp is the sample
                                             // number times the sampling period
  }

  return mockTimestamps;
}

inline void readH5DataBlock(const H5::DataSet* dSet,
                            const BaseDataType& type,
                            void* buffer)
{
  H5::DataSpace fSpace = dSet->getSpace();
  H5::DataType nativeType = IO::HDF5::HDF5IO::getNativeType(type);
  dSet->read(buffer, nativeType, fSpace, fSpace);
}
