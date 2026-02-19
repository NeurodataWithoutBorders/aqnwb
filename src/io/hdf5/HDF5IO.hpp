#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <H5Opublic.h>

#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"

namespace H5
{
class DataSet;
class Attribute;
class H5File;
class DataType;
class Exception;
class CommonFG;
class PredType;
class DataSpace;
}  // namespace H5

/*!
 * \namespace AQNWB::IO::HDF5
 * \brief Namespace for all components of the HDF5 I/O backend
 */
namespace AQNWB::IO::HDF5
{

class HDF5RecordingData;  // forward declaration

/**
 * @brief The HDF5IO class provides an interface for reading and writing data to
 * HDF5 files.
 */
class HDF5IO : public BaseIO
{
public:
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
   * @brief Opens an existing file or creates a new file for writing.
   * @return The status of the file opening operation.
   */
  Status open() override;

  /**
   * @brief Opens an existing file or creates a new file for writing.
   * @param mode Access mode to use when opening the file.
   * @return The status of the file opening operation.
   */
  Status open(FileMode mode) override;

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
   * @brief  Get the storage type (Group, Dataset, Attribute) of the object at
   * path
   *
   * @param path The path of the object in the file
   * @return The StorageObjectType. May be Undefined if the object does not
   * exist.
   */
  StorageObjectType getStorageObjectType(std::string path) const override;

  /**
   * @brief Reads a dataset or attribute and determines the data type.
   *
   * @param dataPath The path to the dataset or attribute within the file.
   * @param start The starting indices for the slice (optional).
   * @param count The number of elements to read for each dimension (optional).
   * @param stride The stride for each dimension (optional).
   * @param block The block size for each dimension (optional).
   *
   * @exception May raise various H5 exceptions if read fails
   *
   * @return A DataGeneric structure containing the data and shape.
   */
  AQNWB::IO::DataBlockGeneric readDataset(
      const std::string& dataPath,
      const std::vector<SizeType>& start = {},
      const std::vector<SizeType>& count = {},
      const std::vector<SizeType>& stride = {},
      const std::vector<SizeType>& block = {}) override;

  /**
   * @brief Reads a attribute  and determines the data type
   *
   * We use IO::DataBlockGeneric here, i.e., the subclass must determine the
   * data type. The user can then convert IO::DataBlockGeneric to the
   * specific type via DataBlock::fromGeneric.
   *
   * @param dataPath The path to the attribute within the file.
   *
   * @exception May raise various H5 exceptions if read fails
   *
   * @return A DataGeneric structure containing the data and shape.
   */
  AQNWB::IO::DataBlockGeneric readAttribute(
      const std::string& dataPath) const override;

  /**
   * @brief Reads a reference attribute and returns the path to the referenced
   * object.
   * @param dataPath The path to the reference attribute within the file.
   * @return The path to the referenced object.
   */
  std::string readReferenceAttribute(
      const std::string& dataPath) const override;

  /**
   * @brief Creates an attribute at a given location in the file.
   * @param type The base data type of the attribute.
   * @param data Pointer to the attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @param size The size of the attribute (default is 1).
   * @return The status of the attribute creation operation.
   */
  Status createAttribute(const IO::BaseDataType& type,
                         const void* data,
                         const std::string& path,
                         const std::string& name,
                         const SizeType& size = 1) override;

  /**
   * @brief Creates a string attribute at a given location in the file.
   * @param data The string attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @param overwrite Overwrite the attribute if it already exists.
   * @return The status of the attribute creation operation.
   */
  Status createAttribute(const std::string& data,
                         const std::string& path,
                         const std::string& name,
                         const bool overwrite = false) override;

  /**
   * @brief Creates an array of variable length strings attribute at a given
   * location in the file.
   *
   * @param data The string array attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @param overwrite Overwrite the attribute if it already exists.
   * @return The status of the attribute creation operation.
   */
  Status createAttribute(const std::vector<std::string>& data,
                         const std::string& path,
                         const std::string& name,
                         const bool overwrite = false) override;

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
   * @brief Creates an extendable dataset with the given configuration and path.
   * @param config The configuration for the dataset, including type, shape, and
   * chunking. Can also be a LinkArrayDataSetConfig to create a soft-link.
   * @param path The location in the file of the new dataset.
   * @return A pointer to the created dataset, or nullptr for links.
   */
  std::unique_ptr<IO::BaseRecordingData> createArrayDataSet(
      const IO::BaseArrayDataSetConfig& config,
      const std::string& path) override;

  /**
   * @brief Returns a pointer to a dataset at a given path.
   * @param path The location in the file of the dataset.
   * @return A shared pointer to the dataset.
   */
  std::shared_ptr<IO::BaseRecordingData> getDataSet(
      const std::string& path) override;

  /**
   * @brief Returns the size of the dataset or attribute for each dimension.
   * @param path The location of the dataset or attribute in the file
   * @return The shape of the dataset or attribute.
   */
  SizeArray getStorageObjectShape(const std::string path) override;

  /**
   * @brief Gets the chunking configuration of a dataset.
   * @param path The path to the dataset.
   * @return The chunking configuration of the dataset, or an empty vector if
   * the dataset is not chunked or doesn't exist.
   */
  SizeArray getStorageObjectChunking(const std::string path) override;

  /**
   * @brief Gets the BaseDataType of a dataset.
   * @param path The path to the dataset.
   * @return The BaseDataType of the dataset. Returns a default T_I32 type if
   * the dataset doesn't exist or is not a dataset.
   */
  BaseDataType getStorageObjectDataType(const std::string path) override;

  /**
   * @brief Checks whether a Dataset, Group, or Link already exists at the
   * location in the file.
   * @param path The location of the object in the file.
   * @return Whether the object exists.
   */
  bool objectExists(const std::string& path) const override;

  /**
   * @brief Checks whether an Attribute exists at the
   * location in the file.
   * @param path The location of the attribute in the file. I.e.,
   *             this is a combination of that parent object's
   *             path and the name of the attribute.
   * @return Whether the attribute exists.
   */
  bool attributeExists(const std::string& path) const override;

  /**
   * @brief Gets the list of storage objects (groups, datasets, attributes)
   * inside a group.
   *
   * This function returns the relative paths and storage type of all objects
   * inside the specified group. If the input path is an attribute then an
   * empty list should be returned. If the input path is a dataset, then only
   * the attributes will be returned. Note, this function is not recursive,
   * i.e., it only looks for storage objects associated directly with the given
   * path.
   *
   * @param path The path to the group.
   * @param objectType Define which types of storage object to look for, i.e.,
   * only objects of this specified type will be returned.
   *
   * @return A vector of pairs of relative paths and their corresponding
   * storage object types.
   */
  virtual std::vector<std::pair<std::string, StorageObjectType>>
  getStorageObjects(const std::string& path,
                    const StorageObjectType& objectType =
                        StorageObjectType::Undefined) const override;

  /**
   * @brief Returns the HDF5 type of object at a given path.
   * @param path The location in the file of the object.
   * @return The type of object at the given path. H5O_TYPE_UNKNOWN indicates
   * that the object does not exist (or is of an unknown type).
   */
  H5O_type_t getH5ObjectType(const std::string& path) const;

  /**
   * @brief Returns the HDF5 native data type for a given base data type.
   *
   * Native types are platform-dependent and represent the data types as they
   * are stored in the memory of the machine where the HDF5 file is created or
   * read.
   *
   * @param type The base data type.
   * @return The HDF5 native data type.
   */
  static H5::DataType getNativeType(IO::BaseDataType type);

  /**
   * @brief Returns the BaseDataType for a given HDF5 native data type
   *
   * This function implements the opposite mapping of getNativeType.
   *
   * @param nativeType The native data type.
   * @return The corresponding BaseDataType
   */
  static IO::BaseDataType getBaseDataType(const H5::DataType& nativeType);

  /**
   * @brief Returns the HDF5 data type for a given base data type.
   *
   * Standard types are platform-independent and represent the data types
   * in a consistent format, regardless of the machine architecture.
   *
   * @param type The base data type.
   * @return The HDF5 data type.
   */
  static H5::DataType getH5Type(IO::BaseDataType type);

protected:
  /**
   * @brief Creates a new group if it does not exist.
   * @param path The location in the file of the group.
   * @return The status of the group creation operation.
   */
  Status createGroupIfDoesNotExist(const std::string& path) override;

private:
  /**
   * @brief Reads data from an HDF5 dataset or attribute into a vector of the
   * appropriate type.
   *
   * This is an internal helper function used to simplify the implementation
   * of the readData method.
   *
   * @tparam T The data type of the dataset or attribute.
   * @tparam HDF5TYPE HDF5 Dataset or Attribute type, usually determined from
   * dataSource
   * @param dataSource The HDF5 data source (dataset or attribute).
   * @param numElements The number of elements to read.
   * @param memspace The memory dataspace (optional).
   * @param dataspace The file dataspace (optional).
   *
   * @return A vector containing the data.
   */
  template<typename T, typename HDF5TYPE>
  std::vector<T> readDataHelper(const HDF5TYPE& dataSource,
                                size_t numElements,
                                const H5::DataSpace& memspace,
                                const H5::DataSpace& dataspace) const;

  /**
   * @brief Reads data from an HDF5 dataset or attribute into a vector of the
   * appropriate type.
   *
   * This is the same as readDataHelper but with default parameters defined for
   * the memspace and dataspace parameters. We overload the method here, rather
   * than defining default parameters directly, to avoid having to include
   * <H5Cpp.h> in the HDF5IO.hpp header file.
   *
   * @tparam T The data type of the dataset or attribute.
   * @tparam HDF5TYPE HDF5 Dataset or Attribute type, usually determined from
   * dataSource
   * @param dataSource The HDF5 data source (dataset or attribute).
   * @param numElements The number of elements to read.
   *
   * @return A vector containing the data.
   */
  template<typename T, typename HDF5TYPE>
  std::vector<T> readDataHelper(const HDF5TYPE& dataSource,
                                size_t numElements) const;

  /**
   * @brief Reads a variable-length string from an HDF5 dataset or attribute.
   *
   * @tparam HDF5TYPE HDF5 Dataset or Attribute type, usually determined from
   * dataSource
   * @param dataSource The HDF5 data source (dataset or attribute).
   * @param numElements The number of elements to read.
   * @param memspace The memory dataspace (optional).
   * @param dataspace The file dataspace (optional).
   *
   * @return A vector containing the data.
   */
  template<typename HDF5TYPE>
  std::vector<std::string> readStringDataHelper(
      const HDF5TYPE& dataSource,
      size_t numElements,
      const H5::DataSpace& memspace,
      const H5::DataSpace& dataspace) const;

  /**
   * @brief Reads a variable-length string from an HDF5 dataset or attribute.
   *
   * This is the same as readStringDataHelper but with default parameters
   * defined for the memspace and dataspace parameters. We overload the method
   * here, rather than defining default parameters directly, to avoid having to
   * include <H5Cpp.h> in the HDF5IO.hpp header file.
   *
   * @tparam HDF5TYPE HDF5 Dataset or Attribute type, usually determined from
   * dataSource
   * @param dataSource The HDF5 data source (dataset or attribute).
   * @param numElements The number of elements to read.
   *
   * @return A vector containing the data.
   */
  template<typename HDF5TYPE>
  std::vector<std::string> readStringDataHelper(const HDF5TYPE& dataSource,
                                                size_t numElements) const;

  std::unique_ptr<H5::Attribute> getAttribute(const std::string& path) const;

  /**
   * @brief Unique pointer to the HDF5 file for reading
   */
  std::unique_ptr<H5::H5File> m_file;

  /**
   * \brief When set true, then do not switch to SWMR mode when starting the
   * recording
   */
  bool m_disableSWMRMode;
};

}  // namespace AQNWB::IO::HDF5
