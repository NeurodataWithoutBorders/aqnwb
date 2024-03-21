#pragma once

#include <string>
#include "hdmf/table/DynamicTable.hpp"
#include "hdmf/table/VectorData.hpp"
#include "hdmf/table/ElementIdentifiers.hpp"
#include "io/BaseIO.hpp"

using namespace AQNWBIO;

/**

A table containing electrode metadata

*/
class ElectrodeTable : public DynamicTable
{
public:
  /** Constructor */
  ElectrodeTable(std::string path, std::shared_ptr<BaseIO> io, std::vector<int> channels, std::string description = "metadata about extracellular electrodes");

  /** Destructor */
  ~ElectrodeTable();

  /** Initialization function */
  void initialize();

  /** Getter for colNames */
  std::vector<std::string> getColNames() const override;

  /** Setter for colNames */
  void setColNames(const std::vector<std::string>& newColNames);

  /** Getter for groupPath */
  std::string getGroupPath() const;

  /** Setter for groupPath */
  void setGroupPath(const std::string& groupPath);

  std::unique_ptr<ElementIdentifiers> electrodeDataset = std::make_unique<ElementIdentifiers>();
  std::unique_ptr<VectorData> groupNamesDataset = std::make_unique<VectorData>();
  std::unique_ptr<VectorData> locationsDataset = std::make_unique<VectorData>();

private:
  std::vector<int> channels;
  std::vector<int> electrodeNumbers;
  std::vector<std::string> groupNames;
  std::vector<std::string> locationNames;
  std::vector<std::string> groupReferences;
  std::vector<std::string> colNames = {"group", "group_name", "location"};
  std::string groupPath = "/general/extracellular_ephys/array1";
};
