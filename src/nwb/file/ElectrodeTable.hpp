#pragma once

#include <string>

#include "io/BaseIO.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/VectorData.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Represents a table containing electrode metadata.
 */
class ElectrodeTable : public DynamicTable
{
public:
  /**
   * @brief Constructor.
   * @param io The shared pointer to the BaseIO object.
   * extracellular electrodes").
   */
  ElectrodeTable(std::shared_ptr<BaseIO> io);

  // required so we can call create
  ElectrodeTable(const std::string& path, std::shared_ptr<BaseIO> io);

  /**
   * @brief Destructor.
   */
  ~ElectrodeTable();

  /**
   * @brief Initializes the ElectrodeTable.
   *
   * Initializes the ElectrodeTable by creating NWB related attributes and
   * adding required columns.
   *
   *  @param description The description of the table (default: "metadata about
   */
  void initialize(const std::string& description =
                      "metadata about extracellular electrodes");

  /**
   * @brief Finalizes the ElectrodeTable.
   *
   * Finalizes the ElectrodeTable by adding the required columns and writing
   * the data to the file.
   */
  void finalize();

  /**
   * @brief Sets up the ElectrodeTable by adding electrodes and their metadata.
   *
   */
  void addElectrodes(std::vector<Channel> channels);

  /**
   * @brief Gets the column names of the ElectrodeTable.
   * @return The vector of column names.
   */
  const std::vector<std::string>& getColNames() override;

  /**
   * @brief Sets the column names of the ElectrodeTable.
   * @param newColNames The vector of new column names.
   */
  void setColNames(const std::vector<std::string>& newColNames);

  /**
   * @brief Gets the group path of the ElectrodeTable.
   * @return The group path.
   */
  std::string getGroupPath() const;

  /**
   * @brief Sets the group path of the ElectrodeTable.
   * @param groupPath The new group path.
   */
  void setGroupPath(const std::string& groupPath);

  std::unique_ptr<ElementIdentifiers> electrodeDataset =
      std::make_unique<ElementIdentifiers>(); /**< The electrode dataset. */
  std::unique_ptr<VectorData> groupNamesDataset =
      std::make_unique<VectorData>(); /**< The group names dataset. */
  std::unique_ptr<VectorData> locationsDataset =
      std::make_unique<VectorData>(); /**< The locations dataset. */

  /**
   * @brief The path to the ElectrodeTable.
   */
  inline const static std::string electrodeTablePath =
      "/general/extracellular_ephys/electrodes/";

private:
  /**
   * @brief The channel information from the acquisition system.
   */
  std::vector<Channel> channels;

  /**
   * @brief The global indices for each electrode.
   */
  std::vector<int> electrodeNumbers;

  /**
   * @brief The names of the ElectrodeGroup object for each electrode.
   */
  std::vector<std::string> groupNames;

  /**
   * @brief The location names for each electrode.
   */
  std::vector<std::string> locationNames;

  /**
   * @brief The references to the ElectrodeGroup object for each electrode.
   */
  std::vector<std::string> groupReferences;

  /**
   * @brief The vector of column names for the table.
   */
  std::vector<std::string> colNames = {"group", "group_name", "location"};

  /**
   * @brief The references path to the ElectrodeGroup
   */
  std::string groupPathBase = "/general/extracellular_ephys/";
};
}  // namespace AQNWB::NWB
