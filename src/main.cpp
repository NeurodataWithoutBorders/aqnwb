#include <iostream>

#include "FileWriter.hpp"

auto main() -> int
{
  const std::string fileName = "test.h5";

  // Create instance of filewriter
  FileWriter fileWriter;

  // Create new HDF5 file
  fileWriter.createFile(fileName);

  const int data[] = {1, 2, 3, 4, 5};
  const int dataSize = sizeof(data) / sizeof(data[0]);

  fileWriter.createDataset("test_dataset", data, dataSize);

  std::cout << "Writing data to HDF5 File..." << '\n';

  return 0;
}
