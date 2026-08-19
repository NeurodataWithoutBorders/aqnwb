#pragma once

// Common STL includes
#include <memory>
#include <optional>
#include <string>
#include <vector>
// Base AqNWB includes for IO and RegisteredType
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/RegisteredType.hpp"
// Include for parent type
#include "nwb/hdmf/table/VectorData.hpp"
// Includes for types that are referenced and used
#include "nwb/hdmf/base/Data.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
// Include for the namespace schema header
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{

/**
 * @brief Used with VectorData to encode a ragged array. An array of indices
 * into the first dimension of the target VectorData, and forming a map between
 * the rows of a DynamicTable and the indices of the VectorData. The name of the
 * VectorIndex is expected to be the name of the target VectorData object
 * followed by "_index".
 *
 * In a ragged array, each row can have a different number of elements. The
 * first vector is at VectorData[0:VectorIndex[0]]. The second vector is at
 * VectorData[VectorIndex[0]:VectorIndex[1]], and so on. This is using
 * half-open intervals, so the first index is inclusive and the second index is
 * exclusive. So for example, if VectorIndex = [3, 5, 8], then the first vector
 * is at VectorData[0:3], the second vector is at VectorData[3:5], and the third
 * vector is at VectorData[5:8], while the total number of elements in the
 * VectorData is 8.
 */
class VectorIndex : public AQNWB::NWB::VectorData
{
protected:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  VectorIndex(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io);

public:
  /**
   * @brief Runtime configuration for creating and initializing a VectorIndex
   * column.
   */
  struct DataSpec : public Data::DataSpec<VectorIndex>
  {
    DataSpec(const std::string& datasetName,
             const IO::ArrayDataSetConfig& dataConfig,
             const std::string& columnDescription,
             const std::string& columnTargetPath)
        : Data::DataSpec<VectorIndex>(datasetName, dataConfig)
        , description(columnDescription)
        , targetPath(columnTargetPath)
    {
    }

    virtual ~DataSpec() = default;

    std::string description;
    std::string targetPath;

    Status initialize(Data& data) const override
    {
      auto* vectorIndex = dynamic_cast<VectorIndex*>(&data);
      if (!vectorIndex) {
        std::cerr << "VectorIndex::DataSpec::initialize received incompatible "
                     "VectorIndex object"
                  << std::endl;
        return Status::Failure;
      }
      return vectorIndex->initialize(
          static_cast<const IO::ArrayDataSetConfig&>(*this),
          description,
          targetPath);
    }
  };

  /**
   * @brief Virtual destructor.
   */
  virtual ~VectorIndex() override {}

  /**
   *  @brief Initialize the dataset for the VectorIndex object
   *
   *  This function creates a dataset using the provided configuration
   *
   * @param dataConfig The configuration for the dataset. Must use an
   * unsigned int data type.
   * @param description The description of the VectorIndex
   * @param targetPath The path to the target VectorData
   * @throw std::invalid_argument if the provided dataSpecs are invalid.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                    const std::string& description,
                    const std::string& targetPath);

  /**
   * @brief Appends data to the target VectorData and updates this
   * index.
   *
   * This method delegates the actual appending of values to the target
   * VectorData, determines how many elements were appended, increments its
   * internal current index, and writes the new index value to its own dataset.
   *
   * @param targetValues The values to append to the target VectorData.
   * @param elementsAppended Output parameter that will be set to the number of
   * elements appended.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status appendData(const AQNWB::Types::CellValue& targetValues,
                    size_t& elementsAppended) override;

  /**
   * @brief Sets the cached target VectorData column.
   *
   * This allows DynamicTable to provide the already-configured target column,
   * avoiding the need to read it from the file on every append.
   *
   * @param target The target VectorData column.
   */
  void setTargetColumn(std::shared_ptr<VectorData> target)
  {
    m_targetColumn = target;
  }

  /**
   * @brief Gets the cached target VectorData column, or reads it if not cached.
   *
   * @return The target VectorData column, or nullptr if not available.
   */
  std::shared_ptr<VectorData> getTargetColumn();

  /**
   * @brief Reads a range of ragged array vectors from the target VectorData.
   *
   * For VectorIndex, this reads the indices and then uses them to read the
   * corresponding ragged array vectors from the target VectorData.
   *
   * @param start The starting index for the slice (optional).
   * @param count The number of elements to read (optional).
   * @param stride The stride (optional).
   * @param block The block size (optional).
   * @return A vector of CellValue, where each CellValue contains the data for a
   * cell.
   * @throws std::out_of_range if the requested range is invalid (e.g., start +
   * count exceeds the number of rows), or if the indices point outside the
   * target dataset.
   * @throws std::runtime_error if the VectorData cannot be read, if the target
   * column is not available, or if the IO object is closed.
   * @throws std::invalid_argument if the indices stored in the VectorIndex are
   * invalid (e.g., not monotonically increasing).
   */
  std::vector<AQNWB::Types::CellValue> readIndexedCellValues(
      SizeType start = 0,
      SizeType count = 0,
      SizeType stride = 1,
      SizeType block = 1);

  // Define read methods
  DEFINE_REFERENCED_REGISTERED_FIELD(
      readTarget,
      VectorData,
      "target",
      "Reference to the target dataset that this index applies to.")

  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         "Description of what these vectors represent.")

  DEFINE_DATASET_FIELD(
      readData,
      recordData,
      uint32_t,
      "",
      "Used with VectorData to encode a ragged array. An array of indices into "
      "the first dimension of the target VectorData - and forming a map "
      "between the rows of a DynamicTable and the indices of the VectorData. "
      "The name of the VectorIndex is expected to be the name of the target "
      "VectorData object followed by _index.")

  REGISTER_SUBCLASS(VectorIndex,
                    VectorData,
                    AQNWB::SPEC::HDMF_COMMON::namespaceName)

private:
  /**
   * @brief The current index value, representing the total number of elements
   * currently in the target VectorData. This is the same as the last value in
   * this VectorIndex.
   */
  uint64_t m_currentIndex = 0;

  /**
   * @brief Flag indicating whether m_currentIndex has been initialized from the
   * file.
   */
  bool m_currentIndexInitialized = false;

  /**
   * @brief Cached target VectorData column to avoid reading it from the file
   * repeatedly.
   */
  std::shared_ptr<VectorData> m_targetColumn;

  /**
   * @brief Cached data type of the VectorIndex dataset.
   */
  IO::BaseDataType m_dataType;

  /**
   * @brief Flag indicating whether m_dataType has been initialized.
   */
  bool m_dataTypeInitialized = false;

  /**
   * @brief Initializes m_currentIndex and m_dataType by reading the last value
   * from the dataset, or setting it to 0 if the dataset is empty. This is used
   * for resuming appending of index values to the VectorIndex.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initializeAppendState();

  /**
   * @brief Extracts the integer index value from a CellValue variant.
   *
   * This helper function safely extracts the underlying integer value from a
   * CellValue that is expected to contain a scalar integer.
   *
   * @param cell The CellValue containing the index.
   * @return The extracted index as a uint64_t. Returns 0 if the cell does not
   * contain a valid integer.
   */
  uint64_t extractIndexValue(const AQNWB::Types::CellValue& cell) const;

  /**
   * @brief Combines multiple scalar CellValues into a single vector CellValue.
   *
   * This helper function takes a slice of scalar CellValues (representing
   * individual elements of a ragged array vector) and combines them into a
   * single CellValue containing a std::vector of the appropriate type.
   *
   * @param cells The vector of scalar CellValues to combine.
   * @param offset The starting index in the cells vector.
   * @param count The number of elements to combine.
   * @return A CellValue containing a vector of the combined elements. Returns
   * an empty vector CellValue if count is 0 or offset is out of bounds.
   */
  AQNWB::Types::CellValue combineCellsToVector(
      const std::vector<AQNWB::Types::CellValue>& cells,
      size_t offset,
      size_t count) const;
};

}  // namespace AQNWB::NWB
