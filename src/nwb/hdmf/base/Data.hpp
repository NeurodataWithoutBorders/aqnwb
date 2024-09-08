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
  REGISTER_SUBCLASS(Data)

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Data(const std::string& path, std::shared_ptr<IO::BaseIO> io)
      : RegisteredType(path, io)
  {
  }

  /**
   * @brief Destructor.
   */
  ~Data() {}

  /**
   * @brief Pointer to dataset.
   */
  std::unique_ptr<IO::BaseRecordingData>
      dataset;  // TODO We may not want this here if we need Data for read
};
}  // namespace AQNWB::NWB
