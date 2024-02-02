#pragma once

#include <memory>
#include <string>

#include <H5Cpp.h>

class FileWriter
{
  /**
   *
   * Create a simple HDF5 File with a group/dataset.
   *
   */

public:
  /** Constructor */
  FileWriter();

  /** Destructor */
  ~FileWriter();

  /** Create file */
  void createFile(const std::string& filename);

  /** Create HDF5 Dataset in file */
  void createDataset(const std::string& datasetName, const int* data, int size);

private:
  std::unique_ptr<H5::H5File> file;
};
