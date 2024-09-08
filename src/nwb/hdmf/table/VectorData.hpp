#pragma once

#include <string>

#include "nwb/hdmf/base/Data.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An n-dimensional dataset representing a column of a DynamicTable.
 */
class VectorData : public Data
{
public:
  // Register Data class as a registered type
  REGISTER_SUBCLASS(VectorData)

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  VectorData(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Gets the description of the table.
   * @return The description of the table.
   */
  std::string getDescription() const;

private:
  /**
   * @brief Description of VectorData.
   */
  std::string description;
};
}  // namespace AQNWB::NWB
