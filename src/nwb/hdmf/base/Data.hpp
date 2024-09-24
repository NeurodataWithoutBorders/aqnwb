#pragma once

#include <memory>

#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An abstract data type for a dataset.
 */
class Data : public RegisteredType
{
public:
  // Register Data class as a registered type
  REGISTER_SUBCLASS(Data, "hdmf-common")

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Data(const std::string& path, std::shared_ptr<IO::BaseIO> io);

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
   * @param dataset The rvalue unique pointer to the BaseRecordingData object
   */
  inline void setDataset(std::unique_ptr<IO::BaseRecordingData>&& dataset)
  {
    m_dataset = std::move(dataset);
  }

  /**
   * @brief Check whether the m_dataset has been initialized
   */
  inline bool isInitialized() { return m_dataset != nullptr; }

  std::unique_ptr<IO::BaseRecordingData>
      m_dataset;  // TODO We may not want this here if we need Data for read
};
}  // namespace AQNWB::NWB
