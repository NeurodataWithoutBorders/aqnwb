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
   *  @brief Initialize the dataset for the Data object
   *
   *  This functions takes ownership of the passed rvalue unique_ptr and moves
   *  ownership to its internal m_dataset variable
   *
   * @param The rvalue unique pointer to the BaseRecordingData object
   */
  inline void setDataset(std::unique_ptr<BaseRecordingData>&& dataset)
  {
    m_dataset = std::move(dataset);
  }

  /**
   * @brief Check whether the m_dataset has been initialized
   */
  inline bool isInitialized() { return this->m_dataset != nullptr; }

  std::unique_ptr<BaseRecordingData> m_dataset;
};
}  // namespace AQNWB::NWB
