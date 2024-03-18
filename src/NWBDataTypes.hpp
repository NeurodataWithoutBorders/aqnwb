#pragma once

#include <string>
#include "BaseIO.hpp"

using namespace AQNWBIO;


/**

Abstract data type for a group storing collections of data and metadata

*/
class Container 
{
public:
  /** Constructor */
  Container(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  ~Container();

protected:
  std::string path;
  std::shared_ptr<BaseIO> io;
};

/**

Metadata about a data acquisition device, e.g., recording system, electrode, microscope.

*/
class Device : public Container
{
public:
  /** Constructor */
  Device(std::string path, std::shared_ptr<BaseIO> io) : Container(path, io) {}

  /** Initialization function */
  void initialize();
};

/**

A group containing multiple datasets that are aligned on the first dimension

*/
class DynamicTable : public Container
{
public:
  /** Constructor */
  DynamicTable(std::string path, std::shared_ptr<BaseIO> io) : Container(path, io) {}

  /** Initialization function */
  void initialize();

  /** Add a column to the table */
  void addColumn(std::string name, std::string description);

  void addIdentifiers(std::string name, std::string description);
};

/**

A table containing electrode metadata

*/
class ElectrodeTable : public DynamicTable
{
public:
  /** Constructor */
  ElectrodeTable(std::string path, std::shared_ptr<BaseIO> io) : DynamicTable(path, io) {}

  /** Initialization function */
  void initialize(std::vector<int> channels);

  std::unique_ptr<BaseRecordingData> electrodeDataset;

  std::unique_ptr<BaseRecordingData> groupNamesDataset;

  std::unique_ptr<BaseRecordingData> locationsDataset;

  std::vector<int> electrodeNumbers;

  std::vector<std::string> groupNames;

  std::vector<std::string> groupReferences;
};


class Data
{
public:
  /** Constructor */
  Data();

  /** Destructor */
  ~Data();
};

class VectorData : Data
{
public:
  /** Constructor */
  VectorData();

  /** Destructor */
  ~VectorData();
};
