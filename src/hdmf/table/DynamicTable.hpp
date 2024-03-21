#pragma once

#include <string>
#include "io/BaseIO.hpp"
#include "hdmf/base/Container.hpp"
#include "hdmf/table/ElementIdentifiers.hpp"
#include "hdmf/table/VectorData.hpp"

using namespace AQNWBIO;

/**
 * @brief Represents a group containing multiple datasets that are aligned on the first dimension
 *
 * This class inherits from the `Container` class and provides methods to add columns of different types of data to the table.
 */
class DynamicTable : public Container
{
public:
  /**
   * @brief Constructer.
   * @param path The location of the table in the file.
   * @param io A shared pointer to the IO object.
   * @param description The description of the table (optional).
   */
  DynamicTable(const std::string& path, std::shared_ptr<BaseIO> io, const std::string& description = " ");

  /**
   * @brief Destructor
   */
  virtual ~DynamicTable();

  /**
   * @brief Initializes the `DynamicTable` object by creating NWB attributes and column names.
   */
  void initialize();

  /**
   * @brief Adds a column of vector string data to the table.
   * @param name The name of the column.
   * @param colDescription The description of the column.
   * @param vectorData A unique pointer to the `VectorData` dataset.
   * @param values The vector of string values.
   */
  void addColumn(const std::string& name, const std::string& colDescription, std::unique_ptr<VectorData>& vectorData, const std::vector<std::string>& values);

  /**
   * @brief Adds a column of element identifiers to the table.
   * @param name The name of the column. Usually `id`.
   * @param colDescription The description of the column.
   * @param elementIDs A unique pointer to the `ElementIdentifiers` dataset.
   * @param values The vector of id values.
   */
  void addColumn(const std::string& name, const std::string& colDescription, std::unique_ptr<ElementIdentifiers>& elementIDs, const std::vector<int>& values);

  /**
   * @brief Adds a column of references to the table.
   * @param name The name of the column.
   * @param colDescription The description of the column.
   * @param dataset The vector of string values representing the references.
   */
  void addColumn(const std::string& name, const std::string& colDescription, const std::vector<std::string>& dataset);

  /**
   * @brief Gets the description of the table.
   * @return The description of the table.
   */
  std::string getDescription() const;

  /**
   * @brief Gets the column names of the table.
   * @return A vector of column names.
   */
  virtual std::vector<std::string> getColNames() const = 0;

private:
  /**
   * @brief Description of the DynamicTable.
   */
  std::string description;

  /**
   * @brief Names of the columns in the table.
   */
  std::vector<std::string> colNames;
};
