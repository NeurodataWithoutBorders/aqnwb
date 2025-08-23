#pragma once

#include <string>

#include "io/BaseIO.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "spec/core.hpp"

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
  REGISTER_SUBCLASS_WITH_TYPENAME(ElectrodeTable,
                                  DynamicTable,
                                  AQNWB::SPEC::CORE::namespaceName,
                                  "DynamicTable")

protected:
  /**
   * @brief Constructor.
   * @param io The shared pointer to the BaseIO object.
   * extracellular electrodes").
   */
  ElectrodeTable(std::shared_ptr<IO::BaseIO> io);

  // required so we can call create
  ElectrodeTable(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /** \brief Convenience factor method since the path is fixed to
   * electrodeTablePath
   * @param io A shared pointer to the IO object.
   * @return A shared pointer to the created NWBFile object, or nullptr if
   * creation failed.
   */
  static std::shared_ptr<ElectrodeTable> create(std::shared_ptr<IO::BaseIO> io)
  {
    return RegisteredType::create<ElectrodeTable>(
        ElectrodeTable::electrodeTablePath, io);
  }

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
   * @param description The description of the table (default: "metadata about
   * extracellular electrodes")
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::string& description =
                        "metadata about extracellular electrodes");

  /**
   * @brief Finalizes the ElectrodeTable.
   *
   * Finalizes the ElectrodeTable by adding the required columns and writing
   * the data to the file.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status finalize() override;

  /**
   * @brief Sets up the ElectrodeTable by adding electrodes and their metadata.
   * @param channelsInput The vector of Channel objects to add to the table.
   */
  void addElectrodes(std::vector<Channel> channelsInput);

  /**
   * @brief The path to the ElectrodeTable.
   */
  inline const static std::string electrodeTablePath =
      "/general/extracellular_ephys/electrodes";

  DEFINE_REGISTERED_FIELD(
      readLocationColumn,
      VectorDataTyped<std::string>,
      "location",
      "the location of channel within the subject e.g. brain region")

  DEFINE_REGISTERED_FIELD(
      readGroupNameColumn,
      VectorDataTyped<std::string>,
      "group_name",
      "the name of the ElectrodeGroup this electrode is a part of")

private:
  /**
   * @brief The global indices for each added electrode.
   */
  std::vector<int> m_electrodeNumbers;

  /**
   * @brief The names of the ElectrodeGroup object for each added electrode.
   */
  std::vector<std::string> m_groupNames;

  /**
   * @brief The location names for each added electrode.
   */
  std::vector<std::string> m_locationNames;

  /**
   * @brief The references to the ElectrodeGroup object for each added
   * electrode.
   */
  std::vector<std::string> m_groupReferences;

  /**
   * @brief The references path to the ElectrodeGroup
   */
  inline const static std::string m_groupPathBase =
      "/general/extracellular_ephys";

  /**
   * @brief The group names column for write
   */
  std::shared_ptr<VectorData> m_groupNamesVectorData;

  /**
   * @brief The locations column for write
   */
  std::shared_ptr<VectorData> m_locationsVectorData;
};
}  // namespace AQNWB::NWB
