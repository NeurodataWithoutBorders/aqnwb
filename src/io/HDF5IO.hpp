#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "BaseIO.hpp"
#include "Types.hpp"
namespace H5
{
class DataSet;
class H5File;
class DataType;
class Exception;
}  // namespace H5

using namespace AQNWBIO;

class HDF5RecordingData;  // declare here because gets used in HDF5IO class

/**
 * @brief The HDF5IO class provides an interface for reading and writing data to HDF5 files.
 */
class HDF5IO : public BaseIO
{
public:
  /**
   * @brief Default constructor for the HDF5IO class.
   */
  HDF5IO();

  /**
   * @brief Constructor for the HDF5IO class that takes a file name as input.
   * @param fileName The name of the HDF5 file.
   */
  HDF5IO(const std::string& fileName);

  /**
   * @brief Destructor for the HDF5IO class.
   */
  ~HDF5IO();

  /**
   * @brief Returns the full path to the HDF5 file.
   * @return The full path to the HDF5 file.
   */
  std::string getFileName() override;

  /**
   * @brief Opens an existing file or creates a new file for writing.
   * @return The status of the file opening operation.
   */
  Status open() override;

  /**
   * @brief Opens an existing file or creates a new file for writing.
   * @param newfile Flag indicating whether to create a new file.
   * @return The status of the file opening operation.
   */
  Status open(bool newfile) override;

  /**
   * @brief Closes the file.
   */
  void close() override;

  /**
   * @brief Creates an attribute at a given location in the file.
   * @param type The base data type of the attribute.
   * @param data Pointer to the attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @param size The size of the attribute (default is 1).
   * @return The status of the attribute creation operation.
   */
  Status createAttribute(const BaseDataType& type,
                   const void* data,
                   const std::string& path,
                   const std::string& name,
                   const SizeType& size = 1) override;

  /**
   * @brief Creates a string attribute at a given location in the file.
   * @param data The string attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @return The status of the attribute creation operation.
   */
  Status createAttribute(const std::string& data,
                   const std::string& path,
                   const std::string& name) override;

  /**
   * @brief Creates a string array attribute at a given location in the file.
   * @param data The string array attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @return The status of the attribute creation operation.
   */
  Status createAttribute(const std::vector<std::string>& data,
                   const std::string& path,
                   const std::string& name) override;

  /**
   * @brief Creates a string array attribute at a given location in the file.
   * @param data The string array attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @param maxSize The maximum size of the string.
   * @return The status of the attribute creation operation.
   */
  Status createAttribute(const std::vector<const char*>& data,
                   const std::string& path,
                   const std::string& name,
                   const SizeType& maxSize) override;

  /**
   * @brief Sets an object reference attribute for a given location in the file.
   * @param referencePath The full path to the referenced group / dataset.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @return The status of the attribute creation operation.
   */
  Status createAttributeRef(const std::string& referencePath,
                      const std::string& path,
                      const std::string& name) override;

  /**
   * @brief Creates a new group in the file.
   * @param path The location in the file of the new group.
   * @return The status of the group creation operation.
   */
  Status createGroup(const std::string& path) override;

  /**
   * @brief Creates a soft link to another location in the file.
   * @param path The location in the file to the new link.
   * @param reference The location in the file of the object that is being linked to.
   */
  void createLink(const std::string& path, const std::string& reference) override;

  /**
   * @brief Creates a non-modifiable dataset with a string value.
   * @param path The location in the file of the dataset.
   * @param value The string value of the dataset.
   */
  void createStringDataSet(const std::string& path, const std::string& value) override;

  /**
   * @brief Creates a dataset that holds an array of references to groups within the file.
   * @param path The location in the file of the new dataset.
   * @param references The array of references.
   */
  void createDataSetOfReferences(const std::string& path,
                              const std::vector<std::string>& references) override;

  /**
   * @brief Creates an extendable dataset with a given base data type, size, chunking, and path.
   * @param type The base data type of the dataset.
   * @param size The size of the dataset.
   * @param chunking The chunking size of the dataset.
   * @param path The location in the file of the new dataset.
   * @return A pointer to the created dataset.
   */
  BaseRecordingData* createDataSet(const BaseDataType& type,
                                   const SizeArray& size,
                                   const SizeArray& chunking,
                                   const std::string& path) override;

  /**
   * @brief Returns a pointer to a dataset at a given path.
   * @param path The location in the file of the dataset.
   * @return A pointer to the dataset.
   */
  BaseRecordingData* getDataSet(const std::string& path) override;

  /**
   * @brief Returns the HDF5 native data type for a given base data type.
   * @param type The base data type.
   * @return The HDF5 native data type.
   */
  static H5::DataType getNativeType(BaseDataType type);

  /**
   * @brief Returns the HDF5 data type for a given base data type.
   * @param type The base data type.
   * @return The HDF5 data type.
   */
  static H5::DataType getH5Type(BaseDataType type);

protected:
  std::string filename;

  /**
   * @brief Creates a new group if it does not exist.
   * @param path The location in the file of the group.
   * @return The status of the group creation operation.
   */
  Status createGroupIfDoesNotExist(const std::string& path) override;

private:
  std::unique_ptr<H5::H5File> file;

};


/**
 * @brief Represents an HDF5 Dataset that can be extended indefinitely
        in blocks.
 *
 * This class provides functionality for reading and writing 2D blocks of data
 * (samples x channels) to an HDF5 dataset.
 */
class HDF5RecordingData : public BaseRecordingData
{
public:
  /**
   * @brief Constructs an HDF5RecordingData object.
   * @param data A pointer to the HDF5 dataset.
   */
  HDF5RecordingData(H5::DataSet* data);

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
   * @brief Writes a 2D block of data to the HDF5 dataset.
   * @param xDataSize The size of the data block in the x dimension (samples).
   * @param yDataSize The size of the data block in the y dimension (channels).
   * @param type The data type of the elements in the data block.
   * @param data A pointer to the data block.
   * @return The status of the write operation.
   */
  Status writeDataBlock(const SizeType& xDataSize,
                        const SizeType& yDataSize,
                        const BaseDataType& type,
                        const void* data);

  /**
   * @brief Reads a block of data from the HDF5 dataset.
   * @param type The data type of the data block.
   * @param buffer A pointer to the buffer to store the read data.
   */
  void readDataBlock(const BaseDataType& type, void* buffer);

private:
  /**
   * @brief Pointer to an extendable HDF5 dataset
   */
  std::unique_ptr<H5::DataSet> dSet;
};
