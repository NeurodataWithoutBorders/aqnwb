#pragma once

#include <string>

#include "BaseIO.hpp"

using namespace AQNWBIO;

/**

An abstract data type for a dataset.

*/
class Data
{
public:
  /** Constructor */
  Data();

  /** Destructor */
  ~Data();

  std::unique_ptr<BaseRecordingData> dataset;
};

/**

An n-dimensional dataset representing a column of a DynamicTable.

*/
class VectorData : public Data
{
public:
  std::string description;
};

/**

A list of unique identifiers for values within a dataset, e.g. rows of a
DynamicTable.

*/
class ElementIdentifiers : public Data
{
};

/**

Abstract data type for a group storing collections of data and metadata

*/
class Container
{
public:
  /** Constructor */
  Container(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  virtual ~Container();

protected:
  std::string path;
  std::shared_ptr<BaseIO> io;
};

/**

Metadata about a data acquisition device, e.g., recording system, electrode,
microscope.

*/
class Device : public Container
{
public:
  /** Constructor */
  Device(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  ~Device();

  /** Initialization function */
  void initialize();

  std::string manufacturer = "unknown";
  std::string description = "description";
};

/**

A physical grouping of electrodes, e.g. a shank of an array.

*/
class ElectrodeGroup : public Container
{
public:
  /** Constructor */
  ElectrodeGroup(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  ~ElectrodeGroup();

  /** Initialization function */
  void initialize();

  /** Add link to acquisition device */
  void linkDevice();

  std::unique_ptr<BaseRecordingData> positionDataset;
  std::string device;  // path to be linked
  std::string description = "description";
  std::string location = "unknown";
};

/**

A group containing multiple datasets that are aligned on the first dimension

*/
class DynamicTable : public Container
{
public:
  /** Constructor */
  DynamicTable(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  virtual ~DynamicTable();

  /** Initialization function */
  void initialize();

  virtual std::vector<std::string> getColNames() const = 0;

  virtual std::string getDescription() const = 0;

  /** Add a column of vector data to the table */
  void addColumn(std::string name,
                 std::string colDescription,
                 std::unique_ptr<VectorData>& vectorData,
                 std::vector<std::string> values);

  /** Add a column of element identifiers to the table */
  void addColumn(std::string name,
                 std::string colDescription,
                 std::unique_ptr<ElementIdentifiers>& elementIDs,
                 std::vector<int> values);

  /** Add a column of references */
  void addColumn(std::string name,
                 std::string colDescription,
                 std::vector<std::string> dataset);

  std::unique_ptr<BaseRecordingData> idDataset;
  std::string description;
  std::vector<std::string> colnames;
};

/**

A table containing electrode metadata

*/
class ElectrodeTable : public DynamicTable
{
public:
  /** Constructor */
  ElectrodeTable(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  ~ElectrodeTable();

  /** Initialization function */
  void initialize();

  std::vector<std::string> getColNames() const override;

  std::string getDescription() const override;

  std::vector<int> channels;
  std::unique_ptr<ElementIdentifiers> electrodeDataset =
      std::make_unique<ElementIdentifiers>();
  std::unique_ptr<VectorData> groupNamesDataset =
      std::make_unique<VectorData>();
  std::unique_ptr<VectorData> locationsDataset = std::make_unique<VectorData>();
  std::vector<int> electrodeNumbers;
  std::vector<std::string> groupNames;
  std::vector<std::string> groupReferences;
  std::vector<std::string> locationNames;
  std::vector<std::string> colnames = {"group", "group_name", "location"};
  std::string description = "metadata about extracellular electrodes";
  std::string groupPath = "/general/extracellular_ephys/array1";
};
