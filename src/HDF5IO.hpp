#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "BaseIO.hpp"

namespace H5
{
class DataSet;
class H5File;
class DataType;
class Exception;
}  // namespace H5

using namespace AQNWBIO;

class HDF5RecordingData;  // declare here because gets used in HDF5IO class

class HDF5IO : public BaseIO
{
public:
  /** Constructor */
  HDF5IO();

  /** Destructor */
  ~HDF5IO();

  /** Returns the full path to the HDF5 file */
  std::string getFileName() override;

  /** Opens the file for writing */
  int open(std::string fileName) override;

  /** Closes the file */
  void close() override;

  static H5::DataType getNativeType(BaseDataType type);
  static H5::DataType getH5Type(BaseDataType type);

  /** Sets an attribute at a given location in the file */
  int setAttribute(BaseDataType type,
                   const void* data,
                   std::string path,
                   std::string name,
                   int size = 1) override;

  // /** Sets a string attribute at a given location in the file */
  int setAttribute(const std::string& data,
                   std::string path,
                   std::string name) override;

  /** Sets a std::string array attribute at a given location in the file */
  int setAttribute(const std::vector<std::string>& data,
                   std::string path,
                   std::string name) override;

  /** Sets a std::string array attribute at a given location in the file */
  int setAttribute(const std::vector<const char*>& data,
                   std::string path,
                   std::string name,
                   size_t maxSize) override;

  // /** Sets an object reference attribute for a given location in the file */
  // int setAttributeRef(std::string referencePath, std::string attributePath,
  // std::string attributeName);

  /** Creates a new group (throws an exception if it exists) */
  int createGroup(std::string path) override;

  /** Creates the basic file structure upon opening */
  int createFileStructure() override;

  // /** Returns a pointer to a dataset at a given path*/
  HDF5RecordingData* getDataSet(std::string path);

  /** aliases for   createDataSet */
  HDF5RecordingData* createDataSet(BaseDataType type,
                                   int sizeX,
                                   int chunkX,
                                   std::string path);
  HDF5RecordingData* createDataSet(
      BaseDataType type, int sizeX, int sizeY, int chunkX, std::string path);
  HDF5RecordingData* createDataSet(BaseDataType type,
                                   int sizeX,
                                   int sizeY,
                                   int sizeZ,
                                   int chunkX,
                                   std::string path);
  HDF5RecordingData* createDataSet(BaseDataType type,
                                   int sizeX,
                                   int sizeY,
                                   int sizeZ,
                                   int chunkX,
                                   int chunkY,
                                   std::string path);

  inline int showError(const char* error)
  {
    std::cerr << error << std::endl;
    return -1;
  }

protected:
  std::string filename;

  /** Creates a new group (ignores if it exists) */
  int createGroupIfDoesNotExist(std::string path) override;

  /** Opens existing file or creates new file  */
  int open(bool newfile);

private:
  std::unique_ptr<H5::H5File> file;

  HDF5RecordingData* createDataSet(
      BaseDataType type,  // TODO - Is there a specific reason this is private?
      int dimension,
      int* size,
      int* chunking,
      std::string path);
};

/**

        Represents an HDF5 Dataset that can be extended indefinitely
        in blocks.

*/
class HDF5RecordingData : public BaseRecordingData
{
public:
  /** Constructor */
  HDF5RecordingData(H5::DataSet* data);
  HDF5RecordingData(const HDF5RecordingData&) =
      delete;  // non construction-copyable
  HDF5RecordingData& operator=(const HDF5RecordingData&) =
      delete;  // non copiable

  /** Destructor */
  ~HDF5RecordingData();

  /** Writes a 2D block of data (samples x channels) */
  int writeDataBlock(int xDataSize,
                     int yDataSize,
                     BaseDataType type,
                     const void* data);

private:
  std::unique_ptr<H5::DataSet> dSet;
};
