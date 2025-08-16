#pragma once

#include <any>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include <boost/multi_array.hpp>  // TODO move this and function def to the cpp file

#include "Types.hpp"

#define DEFAULT_STR_SIZE 256
#define DEFAULT_ARRAY_SIZE 1

using StorageObjectType = AQNWB::Types::StorageObjectType;
using Status = AQNWB::Types::Status;
using SizeArray = AQNWB::Types::SizeArray;
using SizeType = AQNWB::Types::SizeType;

/*!
 * \namespace AQNWB
 * \brief The main namespace for AqNWB
 */
namespace AQNWB::IO
{

class BaseRecordingData;

/**
 * @brief Represents a base data type.
 *
 * This class provides an enumeration of different data types and their
 * corresponding sizes. It also includes handy accessors for commonly used data
 * types.
 */
class BaseDataType
{
public:
  /**
   * @brief Enumeration of different data types.
   */
  enum Type
  {
    T_U8,  ///< Unsigned 8-bit integer
    T_U16,  ///< Unsigned 16-bit integer
    T_U32,  ///< Unsigned 32-bit integer
    T_U64,  ///< Unsigned 64-bit integer
    T_I8,  ///< Signed 8-bit integer
    T_I16,  ///< Signed 16-bit integer
    T_I32,  ///< Signed 32-bit integer
    T_I64,  ///< Signed 64-bit integer
    T_F32,  ///< 32-bit floating point
    T_F64,  ///< 64-bit floating point
    T_STR,  ///< String
    V_STR,  ///< Variable length string
  };

  /**
   * @brief Constructs a BaseDataType object with the specified type and size.
   * @param t The data type.
   * @param s The size of the data type.
   */
  BaseDataType(Type t = T_I32, SizeType s = 1);

  Type type;  ///< The data type.
  SizeType typeSize;  ///< The size of the data type.

  // handy accessors
  static const BaseDataType U8;  ///< Accessor for unsigned 8-bit integer.
  static const BaseDataType U16;  ///< Accessor for unsigned 16-bit integer.
  static const BaseDataType U32;  ///< Accessor for unsigned 32-bit integer.
  static const BaseDataType U64;  ///< Accessor for unsigned 64-bit integer.
  static const BaseDataType I8;  ///< Accessor for signed 8-bit integer.
  static const BaseDataType I16;  ///< Accessor for signed 16-bit integer.
  static const BaseDataType I32;  ///< Accessor for signed 32-bit integer.
  static const BaseDataType I64;  ///< Accessor for signed 64-bit integer.
  static const BaseDataType F32;  ///< Accessor for 32-bit floating point.
  static const BaseDataType F64;  ///< Accessor for 64-bit floating point.
  static const BaseDataType DSTR;  ///< Accessor for dynamic string.
  static BaseDataType STR(
      SizeType size);  ///< Accessor for string with specified size.

  // Define the equality operator
  bool operator==(const BaseDataType& other) const
  {
    return type == other.type && typeSize == other.typeSize;
  }

  // Variant data type for representing any 1D vector with BaseDataType values
  using BaseDataVectorVariant = std::variant<std::monostate,
                                             std::vector<uint8_t>,
                                             std::vector<uint16_t>,
                                             std::vector<uint32_t>,
                                             std::vector<uint64_t>,
                                             std::vector<int8_t>,
                                             std::vector<int16_t>,
                                             std::vector<int32_t>,
                                             std::vector<int64_t>,
                                             std::vector<float>,
                                             std::vector<double>,
                                             std::vector<std::string>>;

  /**
   * @brief Get the BaseDataType from a std::type_index
   *
   * @param typeIndex The type index for which to determine the BaseDataType
   * @return The corresponding BaseDataType
   * @throws std::runtime_error if the typeIndex does not correspond to a
   * supported data type.
   */
  static BaseDataType fromTypeId(const std::type_index& typeIndex)
  {
    if (typeIndex == typeid(uint8_t)) {
      return BaseDataType(U8);
    } else if (typeIndex == typeid(uint16_t)) {
      return BaseDataType(U16);
    } else if (typeIndex == typeid(uint32_t)) {
      return BaseDataType(U32);
    } else if (typeIndex == typeid(uint64_t)) {
      return BaseDataType(U64);
    } else if (typeIndex == typeid(int8_t)) {
      return BaseDataType(I8);
    } else if (typeIndex == typeid(int16_t)) {
      return BaseDataType(I16);
    } else if (typeIndex == typeid(int32_t)) {
      return BaseDataType(I32);
    } else if (typeIndex == typeid(int64_t)) {
      return BaseDataType(I64);
    } else if (typeIndex == typeid(float)) {
      return BaseDataType(F32);
    } else if (typeIndex == typeid(double)) {
      return BaseDataType(F64);
    } else {
      throw std::runtime_error("Unsupported data type");
    }
  }
};

class DataBlockGeneric;

/**
 * @enum SearchMode
 * @brief Enum class for specifying the search mode for findTypes
 */
enum class SearchMode
{
  /**
   * @brief Stop searching inside an object once a matching type is found.
   */
  STOP_ON_TYPE = 1,
  /**
   * @brief Continue searching inside an object even after a matching type is
   * found.
   */
  CONTINUE_ON_TYPE = 2,
};

/**
 * @brief The access mode for the file.
 */
enum class FileMode
{
  /**
   * @brief Opens the file and overwrites any existing file.
   */
  Overwrite,

  /**
   * @brief Opens the file with both read and write access.
   *
   * Note: This is similar to r+ mode, so the file will not be created if it
   * does not exist.
   */
  ReadWrite,

  /**
   * @brief Opens the file in read only mode.
   *
   * Note: This is similar to r+ mode, so the file will not be created if it
   * does not exist.
   */
  ReadOnly
};

/**
 * @brief The configuration for an array dataset
 *
 * This class defines basic properties of an n-Dimensional array dataset, e.g.,
 * to configure how the dataset should be created in the file. IO backends may
 * create their own subclass to add additional configuration options, e.g.,
 * compression, chunking, etc.
 */
class ArrayDataSetConfig
{
public:
  /**
   * @brief Constructs an ArrayDataSetConfig object with the specified type,
   * shape, and chunking.
   * @param type The data type of the dataset.
   * @param shape The shape of the dataset.
   * @param chunking The chunking of the dataset.
   */
  ArrayDataSetConfig(const BaseDataType& type,
                     const SizeArray& shape,
                     const SizeArray& chunking);

  /**
   * @brief Virtual destructor to ensure proper cleanup in derived classes.
   */
  virtual ~ArrayDataSetConfig() = default;

  /**
   * @brief Returns the data type of the dataset.
   * @return The data type of the dataset.
   */
  inline BaseDataType getType() const { return m_type; }

  /**
   * @brief Returns the shape of the dataset.
   * @return The shape of the dataset.
   */
  inline SizeArray getShape() const { return m_shape; }

  /**
   * @brief Returns the chunking of the dataset.
   * @return The chunking of the dataset.
   */
  inline SizeArray getChunking() const { return m_chunking; }

protected:
  // The data type of the dataset
  BaseDataType m_type;
  // The shape of the dataset
  SizeArray m_shape;
  // The chunking of the dataset
  SizeArray m_chunking;
};

/**
 * @brief The BaseIO class is an abstract base class that defines the interface
 * for input/output (IO) operations on a file.
 *
 * This class provides pure virtual methods that must be implemented by all IO
 * classes. It also includes other methods for common IO operations.
 *
 * @note This class cannot be instantiated directly as it is an abstract class.
 */
class BaseIO
{
public:
  /**
   * @brief Constructor for the BaseIO class.
   */
  BaseIO(const std::string& filename);

  /**
   * @brief Copy constructor is deleted to prevent construction-copying.
   */
  BaseIO(const BaseIO&) = delete;

  /**
   * @brief Assignment operator is deleted to prevent copying.
   */
  BaseIO& operator=(const BaseIO&) = delete;

  /**
   * @brief Destructor the BaseIO class.
   */
  virtual ~BaseIO();

  /**
   * @brief Returns the full path to the file.
   * @return The full path to the file.
   */
  virtual std::string getFileName() const { return m_filename; }

  /**
   * @brief  Get the storage type (Group, Dataset, Attribute) of the object at
   * path
   *
   * @param path The path of the object in the file
   * @return The StorageObjectType. May be Undefined if the object does not
   * exist.
   */
  virtual StorageObjectType getStorageObjectType(std::string path) const = 0;

  /**
   * @brief Opens the file for writing.
   * @return The status of the file opening operation.
   */
  virtual Status open() = 0;

  /**
   * @brief Opens an existing file or creates a new file for writing.
   * @param mode Access mode to use when opening the file.
   * @return The status of the file opening operation.
   */
  virtual Status open(FileMode mode) = 0;

  /**
   * @brief Closes the file.
   * @return The status of the file closing operation.
   */
  virtual Status close() = 0;

  /**
   * @brief Flush data to disk
   * @return The status of the flush operation.
   */
  virtual Status flush() = 0;

  /**
   * @brief Checks whether a Dataset, Group, or Link already exists at the
   * location in the file.
   * @param path The location of the object in the file.
   * @return Whether the object exists.
   */
  virtual bool objectExists(const std::string& path) const = 0;

  /**
   * @brief Checks whether an Attribute exists at the
   * location in the file.
   * @param path The location of the attribute in the file. I.e.,
   *             this is a combination of that parent object's
   *             path and the name of the attribute.
   * @return Whether the attribute exists.
   */
  virtual bool attributeExists(const std::string& path) const = 0;

  /**
   * @brief Gets the list of storage objects (groups, datasets, attributes)
   * inside a group.
   *
   * This function returns the relative paths and storage type of all objects
   * inside the specified group. If the input path is an attribute then an empty
   * list should be returned. If the input path is a dataset, then only the
   * attributes will be returned. Note, this function is not recursive, i.e.,
   * it only looks for storage objects associated directly with the given path.
   *
   * @param path The path to the group.
   * @param objectType Define which types of storage object to look for, i.e.,
   * only objects of this specified type will be returned.
   *
   * @return A vector of pairs of relative paths and their corresponding storage
   * object types.
   */
  virtual std::vector<std::pair<std::string, StorageObjectType>>
  getStorageObjects(const std::string& path,
                    const StorageObjectType& objectType =
                        StorageObjectType::Undefined) const = 0;

  /**
   * @brief Finds all datasets and groups of the given types in the HDF5 file.
   *
   * This function recursively searches for the given types in the HDF5 file,
   * starting from the specified path. It checks each object's attributes to
   * determine its type and matches it against the given types.
   *
   * @param starting_path The path in the HDF5 file to start the search from.
   * @param types The set of types to search for. If an empty set is provided,
   *              then all objects with an assigned type (i.e., object that have
   *              a neurodata_type and namespace attributed) will be returned.
   * @param search_mode The search mode to use.
   * @param exclude_starting_path If true, the starting path will not be
   * included in the search and the resutlting output, but only its children
   * will be searched. This also means, if the starting path is a typed object,
   * then STOP_ON_TYPE will stop if exclude_starting_path=false but if
   * exclude_starting_path=true then it will not stop the search at the
   * starting_path but will continue until the next matching typed object is
   * found. This is useful when we want to find all objects with a neurodata
   * type object, but we are not interested in the object itself (e.g., when we
   * have an unknown Container type and we want to find all registered fields
   * that is owns)
   * @return An unordered map where each key is the path to an object and its
   * corresponding value is the type of the object.
   */
  virtual std::unordered_map<std::string, std::string> findTypes(
      const std::string& starting_path,
      const std::unordered_set<std::string>& types,
      SearchMode search_mode,
      bool exclude_starting_path = false) const;

  /**
   * @brief Reads a dataset and determines the data type
   *
   * We use DataBlockGeneric here, i.e., the subclass must determine the
   * data type. The user can then convert DataBlockGeneric to the
   * specific type via DataBlock::fromGeneric.
   *
   * @param dataPath The path to the dataset within the file.
   * @param start The starting indices for the slice (optional).
   * @param count The number of elements to read for each dimension (optional).
   * @param stride The stride for each dimension (optional).
   * @param block The block size for each dimension (optional).
   *
   * @return A DataGeneric structure containing the data and shape.
   */
  virtual DataBlockGeneric readDataset(
      const std::string& dataPath,
      const std::vector<SizeType>& start = {},
      const std::vector<SizeType>& count = {},
      const std::vector<SizeType>& stride = {},
      const std::vector<SizeType>& block = {}) = 0;

  /**
   * @brief Reads a attribute  and determines the data type
   *
   * We use DataBlockGeneric here, i.e., the subclass must determine the
   * data type. The user can then convert DataBlockGeneric to the
   * specific type via DataBlock::fromGeneric.
   *
   * @param dataPath The path to the attribute within the file.
   *
   * @return A DataGeneric structure containing the data and shape.
   */
  virtual DataBlockGeneric readAttribute(const std::string& dataPath) const = 0;

  /**
   * @brief Reads a reference attribute and returns the path to the referenced
   * object.
   * @param dataPath The path to the reference attribute within the file.
   * @return The path to the referenced object.
   */
  virtual std::string readReferenceAttribute(
      const std::string& dataPath) const = 0;

  /**
   * @brief Creates an attribute at a given location in the file.
   * @param type The base data type of the attribute.
   * @param data Pointer to the attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @param size The size of the attribute (default is 1).
   * @return The status of the attribute creation operation.
   */
  virtual Status createAttribute(const BaseDataType& type,
                                 const void* data,
                                 const std::string& path,
                                 const std::string& name,
                                 const SizeType& size = 1) = 0;

  /**
   * @brief Creates a string attribute at a given location in the file.
   * @param data The string attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @param overwrite Overwrite the attribute if it already exists.
   * @return The status of the attribute creation operation.
   */
  virtual Status createAttribute(const std::string& data,
                                 const std::string& path,
                                 const std::string& name,
                                 const bool overwrite = false) = 0;

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
  virtual Status createAttribute(const std::vector<std::string>& data,
                                 const std::string& path,
                                 const std::string& name,
                                 const bool overwrite = false) = 0;

  /**
   * @brief Sets an object reference attribute for a given location in the file.
   * @param referencePath The full path to the referenced group / dataset.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @return The status of the attribute creation operation.
   */
  virtual Status createReferenceAttribute(const std::string& referencePath,
                                          const std::string& path,
                                          const std::string& name) = 0;

  /**
   * @brief Creates a new group in the file.
   * @param path The location in the file of the new group.
   * @return The status of the group creation operation.
   */
  virtual Status createGroup(const std::string& path) = 0;

  /**
   * @brief Creates a soft link to another location in the file.
   * @param path The location in the file to the new link.
   * @param reference The location in the file of the object that is being
   * linked to.
   * @return The status of the link creation operation.
   */
  virtual Status createLink(const std::string& path,
                            const std::string& reference) = 0;

  /**
   * @brief Creates a non-modifiable dataset with a string value.
   * @param path The location in the file of the dataset.
   * @param value The string value of the dataset.
   * @return The status of the dataset creation operation.
   */
  virtual Status createStringDataSet(const std::string& path,
                                     const std::string& value) = 0;

  /**
   * @brief Creates a dataset that holds an array of string values.
   * @param path The location in the file of the dataset.
   * @param values The vector of string values of the dataset.
   * @return The status of the dataset creation operation.
   */
  virtual Status createStringDataSet(
      const std::string& path, const std::vector<std::string>& values) = 0;

  /**
   * @brief Creates a dataset that holds an array of references to groups within
   * the file.
   * @param path The location in the file of the new dataset.
   * @param references The array of references.
   * @return The status of the dataset creation operation.
   */
  virtual Status createReferenceDataSet(
      const std::string& path, const std::vector<std::string>& references) = 0;

  /**
   * @brief Starts the recording process.
   * @return The status of the operation.
   */
  virtual Status startRecording() = 0;

  /**
   * @brief Stops the recording process.
   * @return The status of the operation.
   */
  virtual Status stopRecording() = 0;

  /**
   * @brief Returns true if the file is in a mode where objects can
   * be added or deleted. Note, this does not apply to the modification
   * of raw data on already existing objects. Derived classes should
   * override this function to check if objects can be modified.
   * @return True if the file is in a modification mode, false otherwise.
   */
  virtual bool canModifyObjects() { return true; }

  /**
   * @brief Creates an extendable dataset with the given configuration and path.
   * @param config The configuration for the dataset, including type, shape, and
   * chunking.
   * @param path The location in the file of the new dataset.
   * @return A pointer to the created dataset.
   */
  virtual std::unique_ptr<BaseRecordingData> createArrayDataSet(
      const ArrayDataSetConfig& config, const std::string& path) = 0;

  /**
   * @brief Returns a pointer to a dataset at a given path.
   * @param path The location in the file of the dataset.
   * @return A shared pointer to the dataset.
   */
  virtual std::shared_ptr<BaseRecordingData> getDataSet(
      const std::string& path) = 0;

  /**
   * @brief Returns the size of the dataset or attribute for each dimension.
   * @param path The location of the dataset or attribute in the file
   * @return The shape of the dataset or attribute.
   */
  virtual std::vector<SizeType> getStorageObjectShape(
      const std::string path) = 0;

  /**
   * @brief Convenience function for creating NWB related attributes.
   * @param path The location of the object in the file.
   * @param objectNamespace The namespace of the object.
   * @param neurodataType The neurodata type of the object.
   * @return The status of the operation.
   */
  Status createCommonNWBAttributes(const std::string& path,
                                   const std::string& objectNamespace,
                                   const std::string& neurodataType = "");

  /**
   * @brief Returns true if the file is open.
   * @return True if the file is open, false otherwise.
   */
  inline bool isOpen() const { return m_opened; }

  /**
   * @brief Returns true if the file is able to be opened.
   * @return True if the file is able to be opened, false otherwise.
   */
  inline bool isReadyToOpen() const { return m_readyToOpen; }

protected:
  /**
   * @brief The name of the file.
   */
  const std::string m_filename;

  /**
   * @brief Creates a new group if it does not already exist.
   * @param path The location of the group in the file.
   * @return The status of the operation.
   */
  virtual Status createGroupIfDoesNotExist(const std::string& path) = 0;

  /**
   * @brief Whether the file is ready to be opened.
   */
  bool m_readyToOpen;

  /**
   * @brief Whether the file is currently open.
   */
  bool m_opened;
};

/**
 * @brief The base class to represent recording data that can be extended.
 *
 * This class provides functionality for writing 1D and 2D blocks of data.
 */
class BaseRecordingData
{
public:
  /**
   * @brief Default constructor.
   */
  BaseRecordingData();

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  BaseRecordingData(const BaseRecordingData&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  BaseRecordingData& operator=(const BaseRecordingData&) = delete;

  /**
   * @brief Destructor.
   */
  virtual ~BaseRecordingData();

  /**
   * @brief Writes a block of data using the stored position information.
   * This is not intended to be overwritten by derived classes, but is a
   * convenience function for writing data using the last recorded position.
   * @param dataShape The size of the data block.
   * @param type The data type of the elements in the data block.
   * @param data A pointer to the data block.
   * @return The status of the write operation.
   */
  Status writeDataBlock(const std::vector<SizeType>& dataShape,
                        const BaseDataType& type,
                        const void* data);

  /**
   * @brief Writes a block of data (any number of dimensions).
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param type The data type of the elements in the data block.
   * @param data A pointer to the data block.
   * @return The status of the write operation.
   */
  virtual Status writeDataBlock(const std::vector<SizeType>& dataShape,
                                const std::vector<SizeType>& positionOffset,
                                const BaseDataType& type,
                                const void* data) = 0;

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
  virtual Status writeDataBlock(const std::vector<SizeType>& dataShape,
                                const std::vector<SizeType>& positionOffset,
                                const BaseDataType& type,
                                const std::vector<std::string>& data) = 0;

  /**
   * @brief Get the number of dimensions in the dataset.
   * @return The number of dimensions.
   */
  inline SizeType getNumDimensions() const { return m_shape.size(); }

  /**
   * @brief Get the size of the dataset.
   * @return Vector containing the size in each dimension.
   */
  inline const std::vector<SizeType>& getShape() const { return m_shape; }

  /**
   * @brief Get the current position in the dataset.
   * @return Vector containing the position in each dimension.
   */
  inline const std::vector<SizeType>& getPosition() const { return m_position; }

protected:
  /**
   * @brief The size of the dataset in each dimension.
   */
  std::vector<SizeType> m_shape;

  /**
   * @brief The current position in the dataset.
   */
  std::vector<SizeType> m_position;
};

}  // namespace AQNWB::IO
