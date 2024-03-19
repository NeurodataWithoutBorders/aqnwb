#pragma once

#include <string>
#include "hdmf/base/Data.hpp"

/**

An n-dimensional dataset representing a column of a DynamicTable.

*/
class VectorData : public Data
{
public:
  std::string description;
};
