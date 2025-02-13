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
  // Register the ElectrodeTable as a subclass of Container
  // REGISTER_SUBCLASS(ElectrodeTable, "core")
  REGISTER_SUBCLASS_WITH_TYPENAME(ElectrodeTable, "core", "DynamicTable")

  /**
   * @brief Constructor.
   * @param io The shared pointer to the BaseIO object.
   * extracellular electrodes").
   */
  ElectrodeTable(std::shared_ptr<IO::BaseIO> io);

  // required so we can call create
  ElectrodeTable(const std::string& path, std::shared_ptr<IO::BaseIO> io);

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
   * @param channelsInput The vector of Channel objects to add to the table.
   */
  void addElectrodes(std::vector<Channel> channelsInput);

  /**
   * @brief Gets the group path of the ElectrodeTable.
   * @return The group path.
   */
  inline std::string getGroupPath() const
  {
    // all channels in ChannelVector should have the same groupName
    return m_groupReferences[0];
  }

  /**
   * @brief Sets the group path of the ElectrodeTable.
   * @param groupPath The new group path.
   */
  void setGroupPath(const std::string& groupPath);

  /**
   * @brief The path to the ElectrodeTable.
   */
  inline const static std::string electrodeTablePath =
      "/general/extracellular_ephys/electrodes";

  DEFINE_REGISTERED_FIELD(
      readLocationColumn,
      VectorData<std::string>,
      "location",
      "the location of channel within the subject e.g. brain region")

  DEFINE_REGISTERED_FIELD(
      readGroupNameColumn,
      VectorData<std::string>,
      "group_name",
      "the name of the ElectrodeGroup this electrode is a part of")

private:
  /**
   * @brief The global indices for each electrode.
   */
  std::vector<int> m_electrodeNumbers;

  /**
   * @brief The names of the ElectrodeGroup object for each electrode.
   */
  std::vector<std::string> m_groupNames;

  /**
   * @brief The location names for each electrode.
   */
  std::vector<std::string> m_locationNames;

  /**
   * @brief The references to the ElectrodeGroup object for each electrode.
   */
  std::vector<std::string> m_groupReferences;

  /**
   * @brief The references path to the ElectrodeGroup
   */
  inline const static std::string m_groupPathBase =
      "/general/extracellular_ephys";

  /**
   * @brief The row ids data object for write
   */
  std::unique_ptr<ElementIdentifiers> m_electrodeDataset;

  /**
   * @brief The group names column for write
   */
  std::unique_ptr<VectorData<std::string>> m_groupNamesDataset;

  /**
   * @brief The locations column for write
   */
  std::unique_ptr<VectorData<std::string>> m_locationsDataset;
};
}  // namespace AQNWB::NWB
