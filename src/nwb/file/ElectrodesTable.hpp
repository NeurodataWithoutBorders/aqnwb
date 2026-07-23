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
class ElectrodesTable : public AQNWB::NWB::DynamicTable
{
public:
  // Register the ElectrodesTable as a subclass of Container
  REGISTER_SUBCLASS(ElectrodesTable,
                    DynamicTable,
                    AQNWB::SPEC::CORE::namespaceName)

protected:
  /**
   * @brief Constructor.
   * @param io The shared pointer to the BaseIO object.
   * extracellular electrodes").
   */
  explicit ElectrodesTable(std::shared_ptr<IO::BaseIO> io);

  // required so we can call create
  ElectrodesTable(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /** \brief Convenience factor method since the path is fixed to
   * electrodeTablePath
   * @param io A shared pointer to the IO object.
   * @return A shared pointer to the created NWBFile object, or nullptr if
   * creation failed.
   */
  static std::shared_ptr<ElectrodesTable> create(std::shared_ptr<IO::BaseIO> io)
  {
    return RegisteredType::create<ElectrodesTable>(
        ElectrodesTable::electrodesTablePath, io);
  }

  /**
   * @brief Destructor.
   */
  ~ElectrodesTable() override;

  /**
   * @brief Validates the provided data specifications for the ElectrodesTable.
   * @param dataSpecs The data specifications to validate.
   * @return Status::Success if the specifications are valid, otherwise
   * Status::Failure.
   */
  Status validateDataSpecs(
      const std::vector<DataSpecPtr>& dataSpecs) const override;

  /**
   * @brief Creates the default data specs for the ElectrodesTable.
   * @param rowChunkSize The chunk size for the rows of the table.
   * @return A vector of DataSpecPtr containing the default specs.
   */
  static std::vector<DataSpecPtr> createDefaultDataSpecs(
      const SizeType rowChunkSize = 100);

  /**
   * @brief Initializes the ElectrodesTable.
   *
   * Initializes the ElectrodesTable by creating NWB related attributes and
   * adding required columns.
   *
   * @param description The description of the table (default: "metadata about
   * extracellular electrodes")
   * @param columnSpecs The column specifications to use for initialization.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::string& description =
                        "metadata about extracellular electrodes",
                    const std::vector<DataSpecPtr>& columnSpecs = {});

  /**
   * @brief Finalizes the ElectrodesTable.
   *
   * Finalizes the ElectrodesTable by adding the required columns and writing
   * the data to the file.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status finalize() override;

  /**
   * @brief Sets up the ElectrodesTable by adding electrodes and their metadata.
   * @param channelsInput The vector of Channel objects to add to the table.
   */
  void addElectrodes(const std::vector<Channel>& channelsInput);

  /**
   * @brief The path to the ElectrodesTable.
   */
  inline const static std::string electrodesTablePath =
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
};

}  // namespace AQNWB::NWB
