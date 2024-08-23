#pragma once

#include <memory>

#include "aqnwb/BaseIO.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An abstract data type for a dataset.
 */
class Data
{
public:
  /**
   * @brief Constructor.
   */
  Data() {}

  /**
   * @brief Destructor.
   */
  ~Data() {}

  /**
   * @brief Pointer to dataset.
   */
  std::unique_ptr<BaseRecordingData> dataset;
};
}  // namespace AQNWB::NWB
