#pragma once

#include <string>

#include "nwb/hdmf/table/VectorData.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An n-dimensional dataset representing a column of a DynamicTable.
 */
class DynamicTableRegion : public VectorData  // TODO - I think path should maybe get initialized in VectorData instead
{ 
    friend class NWBFile;  // TODO - not sure if the best way to set this up

public:
  /**
   * @brief Constructor.
   * @param path The location of the DynamicTableRegion in the file.
   * @param io A shared pointer to the IO object.
   * @param tablePath The location of the DynamicTable object this region applies to.
   * @param description The description of what this table region points to.
   */
  DynamicTableRegion(const std::string& path, std::shared_ptr<BaseIO> io, const std::string& tablePath, const std::string& description);

  /**
   * @brief Destructor.
   */
  ~DynamicTableRegion();

  /**
   * @brief Initialize the container.
   */
  void initialize();

protected:
  /**
   * @brief The path of the DynamicTableRegion.
   */
  std::string path;

  /**
   * @brief A shared pointer to the IO object.
   */
  std::shared_ptr<BaseIO> io;
  
  /**
   * @brief Description of what this table region points to.
   */
  std::string description;

  /**
   * @brief The location of the DynamicTable object this region applies to.
   */
  std::string tablePath;

  /**
   * @brief Pointer to int(s) indicating the row(s) of the target array.
   */
  std::unique_ptr<BaseRecordingData> dataset;

  /**
   * @brief The neurodataType of the TimeSeries.
   */
  std::string neurodataType = "DynamicTableRegion";
};
}  // namespace AQNWB::NWB
