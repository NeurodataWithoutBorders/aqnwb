#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "aqnwb/Types.hpp"

#define DEFAULT_STR_SIZE 256
#define DEFAULT_ARRAY_SIZE 1

using Status = AQNWB::Types::Status;
using SizeArray = AQNWB::Types::SizeArray;
using SizeType = AQNWB::Types::SizeType;

/*!
 * \namespace AQNWB
 * \brief The main namespace for AqNWB
 */
namespace AQNWB
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
  BaseIO();

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
  virtual std::string getFileName() = 0;

  /**
   * @brief Opens the file for writing.
   * @return The status of the file opening operation.
   */
  virtual Status open() = 0;

  /**
   * @brief Opens an existing file or creates a new file for writing.
   * @param newfile Flag indicating whether to create a new file.
   * @return The status of the file opening operation.
   */
  virtual Status open(bool newfile) = 0;

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
   * @return The status of the attribute creation operation.
   */
  virtual Status createAttribute(const std::string& data,
                                 const std::string& path,
                                 const std::string& name) = 0;

  /**
   * @brief Creates a string array attribute at a given location in the file.
   * @param data The string array attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @return The status of the attribute creation operation.
   */
  virtual Status createAttribute(const std::vector<std::string>& data,
                                 const std::string& path,
                                 const std::string& name) = 0;

  /**
   * @brief Creates a string array attribute at a given location in the file.
   * @param data The string array attribute data.
   * @param path The location in the file to set the attribute.
   * @param name The name of the attribute.
   * @param maxSize The maximum size of the string.
   * @return The status of the attribute creation operation.
   */
  virtual Status createAttribute(const std::vector<const char*>& data,
                                 const std::string& path,
                                 const std::string& name,
                                 const SizeType& maxSize) = 0;

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
  virtual bool canModifyObjects();

  /**
   * @brief Creates an extendable dataset with a given base data type, size,
   * chunking, and path.
   * @param type The base data type of the dataset.
   * @param size The size of the dataset.
   * @param chunking The chunking size of the dataset.
   * @param path The location in the file of the new dataset.
   * @return A pointer to the created dataset.
   */
  virtual std::unique_ptr<BaseRecordingData> createArrayDataSet(
      const BaseDataType& type,
      const SizeArray& size,
      const SizeArray& chunking,
      const std::string& path) = 0;

  /**
   * @brief Returns a pointer to a dataset at a given path.
   * @param path The location in the file of the dataset.
   * @return A pointer to the dataset.
   */
  virtual std::unique_ptr<BaseRecordingData> getDataSet(
      const std::string& path) = 0;

  /**
   * @brief Convenience function for creating NWB related attributes.
   * @param path The location of the object in the file.
   * @param objectNamespace The namespace of the object.
   * @param neurodataType The neurodata type of the object.
   * @param description The description of the object (default is empty).
   * @return The status of the operation.
   */
  Status createCommonNWBAttributes(const std::string& path,
                                   const std::string& objectNamespace,
                                   const std::string& neurodataType = "",
                                   const std::string& description = "");

  /**
   * @brief Convenience function for creating data related attributes.
   * @param path The location of the object in the file.
   * @param conversion Scalar to multiply each element in data to convert it to
   * the specified ‘unit’.
   * @param resolution Smallest meaningful difference between values in data.
   * @param unit Base unit of measurement for working with the data.
   * @return The status of the operation.
   */
  Status createDataAttributes(const std::string& path,
                              const float& conversion,
                              const float& resolution,
                              const std::string& unit);

  /**
   * @brief Convenience function for creating timestamp related attributes.
   * @param path The location of the object in the file.
   * @return The status of the operation.
   */
  Status createTimestampsAttributes(const std::string& path);
  /**
   * @brief Returns true if the file is open.
   * @return True if the file is open, false otherwise.
   */
  bool isOpen() const;

  /**
   * @brief Returns true if the file is able to be opened.
   * @return True if the file is able to be opened, false otherwise.
   */
  bool isReadyToOpen() const;

  /**
   * @brief The name of the file.
   */
  const std::string filename;

protected:
  /**
   * @brief Creates a new group if it does not already exist.
   * @param path The location of the group in the file.
   * @return The status of the operation.
   */
  virtual Status createGroupIfDoesNotExist(const std::string& path) = 0;

  /**
   * @brief Whether the file is ready to be opened.
   */
  bool readyToOpen;

  /**
   * @brief Whether the file is currently open.
   */
  bool opened;
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

protected:
  /**
   * @brief The size of the dataset in each dimension.
   */
  std::vector<SizeType> size;

  /**
   * @brief The current position in the dataset.
   */
  std::vector<SizeType> position;

  /**
   * @brief The number of dimensions in the data block.
   */
  SizeType nDimensions;
};

}  // namespace AQNWB
