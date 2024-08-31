#pragma once

#include <memory>

#include "BaseIO.hpp"

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
      dataset;  // TODO For read we may not want this here if we ned Data for
                // reads
};
}  // namespace AQNWB::NWB
