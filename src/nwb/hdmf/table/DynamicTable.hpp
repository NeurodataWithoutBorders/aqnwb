#pragma once

#include <string>

#include "io/BaseIO.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/VectorData.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Represents a group containing multiple datasets that are aligned on
 * the first dimension
 *
 * This class inherits from the `Container` class and provides methods to add
 * columns of different types of data to the table.
 */
class DynamicTable : public Container
{
public:
  // Register the TimeSeries as a subclass of Container
  REGISTER_SUBCLASS(DynamicTable, "hdmf-common")

  /**
   * @brief Constructor.
   * @param path The location of the table in the file.
   * @param io A shared pointer to the IO object.
   * @param colNames Set the names of the columns for the table
   */
  DynamicTable(
      const std::string& path,
      std::shared_ptr<IO::BaseIO> io,
      const std::vector<std::string>& colNames =
          {});  // TODO Need to remove colNames here and move it to initialize

  /**
   * @brief Destructor
   */
  virtual ~DynamicTable();

  /**
   * @brief Initializes the `DynamicTable` object by creating NWB attributes and
   * column names.
   *
   * @param description The description of the table (optional).
   */
  void initialize(const std::string& description);

  /**
   * @brief Adds a column of vector string data to the table.
   * @param name The name of the column.
   * @param colDescription The description of the column.
   * @param vectorData A unique pointer to the `VectorData` dataset.
   * @param values The vector of string values.
   */
  void addColumn(const std::string& name,
                 const std::string& colDescription,
                 std::unique_ptr<VectorData>& vectorData,
                 const std::vector<std::string>& values);

  /**
   * @brief Adds a column of references to the table.
   * @param name The name of the column.
   * @param colDescription The description of the column.
   * @param dataset The vector of string values representing the references.
   */
  void addColumn(const std::string& name,
                 const std::string& colDescription,
                 const std::vector<std::string>& dataset);

  /**
   * @brief Adds a column of element identifiers to the table.
   * @param elementIDs A unique pointer to the `ElementIdentifiers` dataset.
   * @param values The vector of id values.
   */
  void setRowIDs(std::unique_ptr<ElementIdentifiers>& elementIDs,
                 const std::vector<int>& values);

  /**
   * @brief Gets the column names of the table.
   * @return A vector of column names.
   */
  virtual const std::vector<std::string>& getColNames() const
  {
    return m_colNames;
  }

  /**
   * @brief Sets the column names of the ElectrodeTable.
   * @param newColNames The vector of new column names.
   */
  virtual void setColNames(const std::vector<std::string>& newColNames)
  {
    m_colNames = newColNames;
  }

protected:
  /**
   * @brief Names of the columns in the table.
   */
  std::vector<std::string> m_colNames;
};
}  // namespace AQNWB::NWB
