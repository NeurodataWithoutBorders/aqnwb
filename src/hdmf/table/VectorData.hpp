#pragma once

#include <string>
#include "hdmf/base/Data.hpp"

/**
 * @brief An n-dimensional dataset representing a column of a DynamicTable.
 */
class VectorData : public Data
{
public:

  /**
   * @brief Description of VectorData.
   */
  std::string description;
};
