#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <H5Opublic.h>

#include "BaseIO.hpp"
#include "Types.hpp"

namespace H5
{
class DataSet;
class H5File;
class DataType;
class Exception;
}  // namespace H5

/*!
 * \namespace AQNWB::HDF5
 * \brief Namespace for all components of the HDF5 I/O backend
 */
namespace AQNWB::HDF5
{
class HDF5RecordingData;  // declare here because gets used in HDF5IO class

/**
 * @brief The HDF5IO class provides an interface for reading and writing data to
 * HDF5 files.
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
   * @param disableSWMRMode Disable recording of data in Single Writer
   *                 Multiple Reader (SWMR) mode. Using SWMR ensures that the
   *                 HDF5 file remains valid and readable at all times during
   *                 the recording process (but does not allow for new objects
   *                 (Groups or Datasets) to be created.
   */
  HDF5IO(const std::string& fileName, const bool disableSWMRMode = false);

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
   * @return The status of the file closing operation.
   */
  Status close() override;

  /**
   * @brief Flush data to disk
   * @return The status of the flush operation.
   */
  Status flush() override;

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
  Status createReferenceAttribute(const std::string& referencePath,
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
   * @param reference The location in the file of the object that is being
   * linked to.
   * @return The status of the link creation operation.
   */
  Status createLink(const std::string& path,
                    const std::string& reference) override;

  /**
   * @brief Creates a non-modifiable dataset with a string value.
   * @param path The location in the file of the dataset.
   * @param value The string value of the dataset.
   * @return The status of the dataset creation operation.
   */
  Status createStringDataSet(const std::string& path,
                             const std::string& value) override;

  /**
   * @brief Creates a dataset that holds an array of string values.
   * @param path The location in the file of the dataset.
   * @param values The vector of string values of the dataset.
   * @return The status of the dataset creation operation.
   */
  Status createStringDataSet(const std::string& path,
                             const std::vector<std::string>& values) override;

  /**
   * @brief Creates a dataset that holds an array of references to groups within
   * the file.
   * @param path The location in the file of the new dataset.
   * @param references The array of references.
   * @return The status of the dataset creation operation.
   */
  Status createReferenceDataSet(
      const std::string& path,
      const std::vector<std::string>& references) override;

  /**
   * @brief Start SWMR write to start recording process
   * @return The status of the start recording operation.
   */
  Status startRecording() override;

  /**
   * @brief Stops the recording process.
   * @return The status of the stop recording operation.
   */
  Status stopRecording() override;

  /**
   * @brief Checks whether the file is in a mode where objects
   * can be added or deleted. Note, this does not apply to the modification
   * of raw data on already existing objects.
   * @return Whether objects can be modified.
   */
  bool canModifyObjects() override;

  /**
   * @brief Creates an extendable dataset with a given base data type, size,
   * chunking, and path.
   * @param type The base data type of the dataset.
   * @param size The size of the dataset.
   * @param chunking The chunking size of the dataset.
   * @param path The location in the file of the new dataset.
   * @return A pointer to the created dataset.
   */
  std::unique_ptr<BaseRecordingData> createArrayDataSet(
      const BaseDataType& type,
      const SizeArray& size,
      const SizeArray& chunking,
      const std::string& path) override;

  /**
   * @brief Returns a pointer to a dataset at a given path.
   * @param path The location in the file of the dataset.
   * @return A pointer to the dataset.
   */
  std::unique_ptr<BaseRecordingData> getDataSet(
      const std::string& path) override;

  /**
   * @brief Checks whether a Dataset, Group, or Link already exists at the
   * location in the file.
   * @param path The location of the object in the file.
   * @return Whether the object exists.
   */
  bool objectExists(const std::string& path) override;

  /**
   * @brief Returns the HDF5 type of object at a given path.
   * @param path The location in the file of the object.
   * @return The type of object at the given path.
   */
  H5O_type_t getObjectType(const std::string& path);

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
  bool disableSWMRMode;  // when set do not use SWMR mode when opening the HDF5
                         // file
};

/**
 * @brief Represents an HDF5 Dataset that can be extended indefinitely
        in blocks.
*
* This class provides functionality for reading and writing blocks of data
* to an HDF5 dataset.
*/
class HDF5RecordingData : public BaseRecordingData
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
                        const BaseDataType& type,
                        const void* data);

  /**
   * @brief Gets a const pointer to the HDF5 dataset.
   * @return A const pointer to the HDF5 dataset.
   */
  const H5::DataSet* getDataSet();

private:
  /**
   * @brief Return status of HDF5 operations.
   */
  Status checkStatus(int status);

  /**
   * @brief Pointer to an extendable HDF5 dataset
   */
  std::unique_ptr<H5::DataSet> m_dataset;
};
}  // namespace AQNWB::HDF5
