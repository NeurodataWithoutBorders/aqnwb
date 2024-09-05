#pragma once

#include <memory>

#include "io/BaseIO.hpp"

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
  std::unique_ptr<BaseRecordingData>
      dataset;  // TODO We may not want this here if we need Data for read
};
}  // namespace AQNWB::NWB
