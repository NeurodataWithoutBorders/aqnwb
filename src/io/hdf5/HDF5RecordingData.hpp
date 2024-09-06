#pragma once

#include "io/BaseIO.hpp"

namespace H5
{
class DataSet;
}  // namespace H5

namespace AQNWB::IO::HDF5
{
/**
 * @brief Represents an HDF5 Dataset that can be extended indefinitely
 *        in blocks.
 *
 * This class provides functionality for reading and writing blocks of data
 * to an HDF5 dataset.
 */
class HDF5RecordingData : public AQNWB::IO::BaseRecordingData
{
public:
  /**
   * @brief Constructs an HDF5RecordingData object.
   * @param data A pointer to the HDF5 dataset.
   */
  HDF5RecordingData(std::unique_ptr<H5::DataSet> data);

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  HDF5RecordingData(const HDF5RecordingData&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  HDF5RecordingData& operator=(const HDF5RecordingData&) = delete;

  /**
   * @brief Destroys the HDF5RecordingData object.
   */
  ~HDF5RecordingData();

  /**
   * @brief Writes a block of data to the HDF5 dataset.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param type The data type of the elements in the data block.
   * @param data A pointer to the data block.
   * @return The status of the write operation.
   */
  Status writeDataBlock(const std::vector<SizeType>& dataShape,
                        const std::vector<SizeType>& positionOffset,
                        const AQNWB::IO::BaseDataType& type,
                        const void* data);

  /**
   * @brief Gets a const pointer to the HDF5 dataset.
   * @return A const pointer to the HDF5 dataset.
   */
  const H5::DataSet* getDataSet();

private:
  /**
   * @brief Pointer to an extendable HDF5 dataset
   */
  std::unique_ptr<H5::DataSet> dSet;

  /**
   * @brief Return status of HDF5 operations.
   */
  Status checkStatus(int status);
};
}  // namespace AQNWB::IO::HDF5
