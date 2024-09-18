#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <H5Cpp.h>  // Include HDF5 headers

using namespace H5;

int readerFunction(const std::string& path, const std::string& dataPath)
{
  try {
    std::cout << "Opening file from path: " << path << std::endl;
    std::unique_ptr<H5File> file =
        std::make_unique<H5File>(path, H5F_ACC_RDONLY | H5F_ACC_SWMR_READ);
    std::unique_ptr<H5::DataSet> dSet =
        std::make_unique<H5::DataSet>(file->openDataSet(dataPath));

    std::vector<hsize_t> dsetSizes;
    for (int i = 0; i < 3; ++i) {
      H5Drefresh(dSet->getId());

      // Get the current size of the dataset
      DataSpace fSpace = dSet->getSpace();
      hsize_t currentSize;
      fSpace.getSimpleExtentDims(&currentSize, nullptr);

      // Update the size
      dsetSizes.push_back(currentSize);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      // Simulate real-time data streaming
    }

    // print out dataset sizes
    std::cout << "Dataset sizes: ";
    for (hsize_t val : dsetSizes) {
      std::cout << val << " ";
    }
    std::cout << std::endl;

    // check that data is being appended (last value should be greater than the
    // first)
    if (dsetSizes[0] >= dsetSizes[2]) {
      return -1;
    }
  } catch (const FileIException&) {
    return -1;
  }

  return 0;
}

int main(int /*argc*/, char* argv[])
{
  std::string path = argv[1];
  std::string dataPath = argv[2];

  return readerFunction(path, dataPath);
}