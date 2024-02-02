#include "FileWriter.hpp"

FileWriter::FileWriter() {}

FileWriter::~FileWriter() {}

void FileWriter::createFile(const std::string& fileName)
{
  file = std::make_unique<H5::H5File>(fileName, H5F_ACC_TRUNC);
}

void FileWriter::createDataset(const std::string& datasetName,
                               const int* data,
                               int size)
{
  if (!file) {
    // Handle error because file must be created first
    return;
  }

  hsize_t dims[1] = {static_cast<hsize_t>(size)};
  H5::DataSpace dataspace(1, dims);

  // Create a dataset within the file
  H5::DataSet dataset =
      file->createDataSet("test", H5::PredType::STD_I32LE, dataspace);

  // Write data to the dataset
  dataset.write(data, H5::PredType::STD_I32LE);
}

// void HDF5FileBase::close()
// {
//     file = nullptr;
//     opened = false;
// }