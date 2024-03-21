#pragma once

#include <string>
#include "io/BaseIO.hpp"
#include "hdmf/base/Container.hpp"
#include "hdmf/table/ElementIdentifiers.hpp"
#include "hdmf/table/VectorData.hpp"

using namespace AQNWBIO;

/**

A group containing multiple datasets that are aligned on the first dimension

*/
class DynamicTable : public Container
{
public:
  /** Constructor */
  DynamicTable(std::string path, std::shared_ptr<BaseIO> io, std::string description = " ");

  /** Destructor */
  virtual ~DynamicTable();

  /** Initialization function */
  void initialize();

  /** Add a column of vector string data to the table */
  void addColumn(std::string name, std::string colDescription, std::unique_ptr<VectorData>& vectorData, std::vector<std::string> values);

  /** Add a column of element identifiers to the table */
  void addColumn(std::string name, std::string colDescription, std::unique_ptr<ElementIdentifiers>& elementIDs, std::vector<int> values);

  /** Add a column of references */
  void addColumn(std::string name, std::string colDescription, std::vector<std::string> dataset);

  /** Getter for description */
  std::string getDescription() const;

  /** Getter for column names */
  virtual std::vector<std::string> getColNames() const = 0;

private:
  std::unique_ptr<BaseRecordingData> idDataset;
  std::string description;
  std::vector<std::string> colNames;
};
