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
  inline std::string getDescription() const { return m_description; }

private:
  /**
   * @brief Description of VectorData.
   */
  std::string m_description;
};
}  // namespace AQNWB::NWB
