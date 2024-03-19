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
  ElectrodeTable(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  ~ElectrodeTable();

  /** Initialization function */
  void initialize();

  std::vector<std::string> getColNames() const override;

  std::string getDescription() const override;

  std::vector<int> channels;
  std::unique_ptr<ElementIdentifiers> electrodeDataset = std::make_unique<ElementIdentifiers>();
  std::unique_ptr<VectorData> groupNamesDataset = std::make_unique<VectorData>();
  std::unique_ptr<VectorData> locationsDataset = std::make_unique<VectorData>();
  std::vector<int> electrodeNumbers;
  std::vector<std::string> groupNames;
  std::vector<std::string> groupReferences;
  std::vector<std::string> locationNames;
  std::vector<std::string> colnames = {"group", "group_name", "location"};
  std::string description = "metadata about extracellular electrodes";
  std::string groupPath = "/general/extracellular_ephys/array1";
};
