#pragma once

#include <optional>

#include "io/BaseIO.hpp"

namespace H5
{
class DataSet;
class DataType;
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
  explicit HDF5RecordingData(std::unique_ptr<H5::DataSet> data);

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
  ~HDF5RecordingData() override;

  /**
   * @brief Writes a block of data to the HDF5 dataset.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param type The data type of the elements in the data block.
   * @param data A pointer to the data block.
   * @return The status of the write operation.
   */
  Status writeDataBlock(const SizeArray& dataShape,
                        const SizeArray& positionOffset,
                        const AQNWB::IO::BaseDataType& type,
                        const void* data) override;

  /**
   * @brief Writes a block of string data (any number of dimensions).
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param type The data type of the elements in the data block. Either
   *             BaseDataType::Type::V_STR or BaseDataType::Type::T_STR
   *             for variable and fixed-length strings repsetively.
   * @param data Vector with the string data
   * @return The status of the write operation.
   */
  Status writeDataBlock(const SizeArray& dataShape,
                        const SizeArray& positionOffset,
                        const AQNWB::IO::BaseDataType& type,
                        const std::vector<std::string>& data) override;

  /**
   * @brief Gets a const pointer to the HDF5 dataset.
   * @return A const pointer to the HDF5 dataset.
   */
  inline const H5::DataSet* getDataSet() const { return m_dataset.get(); }

private:
  /**
   * @brief Checks the shape and position of a data block and extends the
   * dataset if the block doesn't fit.
   *
   * Used by both write paths, writeBlock and writeNumericBlock.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param offset The position of the block in the dataset (output; empty for
   * a scalar dataset)
   * @param count The size of the block in the dataset, with dimensions of size
   * 0 written as 1 (output; empty for a scalar dataset)
   * @return The status of the operation.
   */
  Status prepareWrite(const SizeArray& dataShape,
                      const SizeArray& positionOffset,
                      std::vector<uint64_t>& offset,
                      std::vector<uint64_t>& count);

  /**
   * @brief Write a block of data with the HDF5 C++ API (see writeDataBlock).
   *
   * Used for strings, scalar datasets and array types.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param memType The HDF5 type of the data in memory.
   * @param data A pointer to the data block.
   * @return The status of the write operation.
   */
  Status writeBlock(const SizeArray& dataShape,
                    const SizeArray& positionOffset,
                    const H5::DataType& memType,
                    const void* data);

  /**
   * @brief Write a block of numeric data with the HDF5 C API (see
   * writeDataBlock). The dataset must have at least one dimension.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param memType The HDF5 native type (hid_t) of the data in memory, from
   * HDF5IO::getNativeNumericTypeId
   * @param data A pointer to the data block.
   * @return The status of the write operation.
   */
  Status writeNumericBlock(const SizeArray& dataShape,
                           const SizeArray& positionOffset,
                           int64_t memType,
                           const void* data);

  /**
   * @brief Pointer to an extendable HDF5 dataset
   */
  std::unique_ptr<H5::DataSet> m_dataset;

  /**
   * @brief The HDF5 type of the last string write and its base data type.
   *
   * Creating a string type (H5Tcopy, H5Tset_size, H5Tset_cset) for each
   * write is a measurable part of the cost of small string writes (e.g., one
   * row of a table with a text column), so the type is reused while the base
   * data type of the writes stays the same.
   */
  std::unique_ptr<H5::DataType> m_stringType;
  std::optional<IO::BaseDataType> m_stringBaseType;
};
}  // namespace AQNWB::IO::HDF5
