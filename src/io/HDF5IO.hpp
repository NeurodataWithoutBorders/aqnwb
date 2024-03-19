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

  /** Constructor */
  HDF5IO(std::string fileName);

  /** Destructor */
  ~HDF5IO();

  /** Returns the full path to the HDF5 file */
  std::string getFileName() override;

  /** Opens existing file or creates new file for writing */
  int open() override;

  /** Opens existing file or creates new file for writing */
  int open(bool newfile) override;

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

  /** Sets an object reference attribute for a given location in the file */
  int setAttributeRef(std::string referencePath,
                      std::string path,
                      std::string name) override;

  /** Creates a new group (throws an exception if it exists) */
  int createGroup(std::string path) override;

  // /** Returns a pointer to a dataset at a given path*/
  HDF5RecordingData* getDataSet(std::string path);

  /** Creates a link to another location in the file */
  void createLink(std::string path, std::string reference) override;

  /** Creates a non-modifiable dataset with a string value */
  void createStringDataSet(std::string path, std::string value) override;

  /** Creates a dataset that holds an array of references to groups within the
   * file */
  void createReferenceDataSet(std::string path,
                              std::vector<std::string> references) override;

  /** aliases for createDataSet */
  BaseRecordingData* createDataSet(BaseDataType type,
                                   const std::vector<int>& size,
                                   const std::vector<int>& chunking,
                                   const std::string path) override;

protected:
  std::string filename;

  /** Creates a new group (ignores if it exists) */
  int createGroupIfDoesNotExist(std::string path) override;

private:
  std::unique_ptr<H5::H5File> file;

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
