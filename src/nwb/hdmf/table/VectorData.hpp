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
