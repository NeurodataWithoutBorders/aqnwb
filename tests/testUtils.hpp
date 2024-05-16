
#pragma once

#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "Types.hpp"
#include "Channel.hpp"

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

inline std::vector<Types::ChannelGroup> getMockTestArrays()
{
  Channel ch0 = Channel("ch0", "array1", 0, 0);
  Channel ch1 = Channel("ch1", "array1", 1, 1);
  Channel ch3 = Channel("ch3", "array2", 0, 2);
  Channel ch4 = Channel("ch4", "array2", 1, 3);
  std::vector<Types::ChannelGroup> arrays = {Types::ChannelGroup{ch0, ch1}};

  return arrays;
}