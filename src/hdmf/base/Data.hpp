#pragma once

#include <memory>
#include "io/BaseIO.hpp"

/**

An abstract data type for a dataset.

*/
class Data
{
public:
  /** Constructor */
  Data(){}

  /** Destructor */
  ~Data(){}

  std::unique_ptr<BaseRecordingData> dataset;
};
