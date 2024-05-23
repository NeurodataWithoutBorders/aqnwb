
#pragma once

#include <filesystem>
#include <random>

#include <catch2/catch_test_macros.hpp>

#include "Channel.hpp"
#include "Types.hpp"

using namespace AQNWB;
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

inline std::vector<Types::ChannelGroup> getMockChannelArrays()
{
  Channel ch0 = Channel("ch0", "array1", 0, 0);
  Channel ch1 = Channel("ch1", "array1", 1, 1);
  Channel ch2 = Channel("ch2", "array2", 0, 2);
  Channel ch3 = Channel("ch3", "array2", 1, 3);
  // std::vector<Types::ChannelGroup> arrays = {Types::ChannelGroup {ch0, ch1, ch2, ch3}}; // TODO - make this work for multiple channel groups
  std::vector<Types::ChannelGroup> arrays = {Types::ChannelGroup {ch0, ch1},
                                             Types::ChannelGroup {ch2, ch3}}; // TODO - make this work for multiple channel groups

  return arrays;
}

inline std::vector<std::vector<float>> getMockData(int numChannels = 4, int numSamples = 1000)
{
  std::vector<std::vector<float>> mockData(numChannels, std::vector<float>(numSamples));

  std::random_device rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> dis(0.0, 1.0); // Define the range for your floats here

  for(auto& channelData : mockData) {
      for(auto& data : channelData) {
          data = dis(gen); // Generate a random float and assign it to the current element
      }
  }

  return mockData;
}

inline std::vector<double> getMockTimestamps(int numSamples = 1000, int samplingRate = 30000)
{
  std::vector<double> mockTimestamps(numSamples);
  double samplingPeriod = 1.0 / samplingRate;

  for(int i = 0; i < numSamples; ++i) {
      mockTimestamps[i] = i * samplingPeriod; // Each timestamp is the sample number times the sampling period
  }

  return mockTimestamps;
}