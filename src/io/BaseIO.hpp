#pragma once

#include <iostream>
#include <string>

#define DEFAULT_STR_SIZE 256
#define DEFAULT_ARRAY_SIZE 1

namespace AQNWBIO
{

class BaseRecordingData;

class BaseDataType
{
public:
  enum Type
  {
    T_U8,
    T_U16,
    T_U32,
    T_U64,
    T_I8,
    T_I16,
    T_I32,
    T_I64,
    T_F32,
    T_F64,
    T_STR,
  };
  BaseDataType(Type t = T_I32, size_t s = 1);
  Type type;
  size_t typeSize;

  // handy accessors
  static const BaseDataType U8;
  static const BaseDataType U16;
  static const BaseDataType U32;
  static const BaseDataType U64;
  static const BaseDataType I8;
  static const BaseDataType I16;
  static const BaseDataType I32;
  static const BaseDataType I64;
  static const BaseDataType F32;
  static const BaseDataType F64;
  static const BaseDataType DSTR;
  static BaseDataType STR(size_t size);
};

/**

        Sets up the BaseIO class as an IO framework

*/
class BaseIO
{
public:
  /** Constructor */
  BaseIO();
  BaseIO(const BaseIO&) = delete;  // non construction-copyable
  BaseIO& operator=(const BaseIO&) = delete;  // non copiable

  /** Destructor */
  virtual ~BaseIO();

  // ------------------------------------------------------------
  //                  PURE VIRTUAL METHODS
  //         (must be implemented by all IO classes)
  // ------------------------------------------------------------

  /** Returns the full path to the file */
  virtual std::string getFileName() = 0;

  /** Opens the file for writing */
  virtual int open() = 0;

  /** Opens the file for writing */
  virtual int open(bool newfile) = 0;

  /** Closes the file */
  virtual void close() = 0;

  /** Sets an attribute at a given location in the file */
  virtual int setAttribute(BaseDataType type,
                           const void* data,
                           std::string path,
                           std::string name,
                           size_t size = 1) = 0;

  // /** Sets a string attribute at a given location in the file */
  virtual int setAttribute(const std::string& data,
                           std::string path,
                           std::string name) = 0;

  /** Sets a std::string array attribute at a given location in the file */
  virtual int setAttribute(const std::vector<std::string>& data,
                           std::string path,
                           std::string name) = 0;

  /** Sets a std::string array attribute at a given location in the file */
  virtual int setAttribute(const std::vector<const char*>& data,
                           std::string path,
                           std::string name,
                           size_t maxSize) = 0;

  /** Sets an object reference attribute for a given location in the file */
  virtual int setAttributeRef(std::string referencePath,
                              std::string path,
                              std::string name) = 0;

  /** Creates a new group */
  virtual int createGroup(std::string path) = 0;

  /** Creates a link to another location in the file */
  virtual void createLink(std::string path, std::string reference) = 0;

  /** Creates a non-modifiable dataset with a string value */
  virtual void createStringDataSet(std::string path, std::string value) = 0;

  /** Creates a dataset that holds an array of references to groups / datasets
   *  within the file */
  virtual void createReferenceDataSet(std::string path,
                                      std::vector<std::string> references) = 0;

  /** Create an extendable dataset */
  virtual BaseRecordingData* createDataSet(BaseDataType type,
                                           const std::vector<size_t>& size,
                                           const std::vector<size_t>& chunking,
                                           const std::string path) = 0;

  // ------------------------------------------------------------
  //                    OTHER METHODS
  // ------------------------------------------------------------

  /** Sets up the attributes for a group */
  int setCommonNWBAttributes(std::string path,
                             std::string group_namespace,
                             std::string neurodata_type,
                             std::string description = "");

  /** Returns true if the file is open */
  bool isOpen() const;

  /** Returns true if the file is able to be opened */
  bool isReadyToOpen() const;

  /** Returns name of the file */
  const std::string filename;

protected:
  /** Creates a new group (ignores if it exists) */
  virtual int createGroupIfDoesNotExist(std::string path) = 0;

  bool readyToOpen;
  bool opened;
};

/**

        Sets up the BaseRecording class for extendable datasets

*/
class BaseRecordingData
{
public:
  /** Constructor */
  BaseRecordingData();
  BaseRecordingData(const BaseRecordingData&) =
      delete;  // non construction-copyable
  BaseRecordingData& operator=(const BaseRecordingData&) =
      delete;  // non copiable

  /** Destructor */
  virtual ~BaseRecordingData();

  /** Writes a 1D block of data (samples) */
  int writeDataBlock(size_t xDataSize, BaseDataType type, const void* data);

  /** Writes a 2D block of data (samples x channels) */
  virtual int writeDataBlock(size_t xDataSize,
                             size_t yDataSize,
                             BaseDataType type,
                             const void* data) = 0;

protected:
  int xPos;
  size_t xChunkSize;
  size_t size[3];
  size_t dimension;
  std::vector<uint32_t> rowXPos;
};

}  // namespace AQNWBIO
