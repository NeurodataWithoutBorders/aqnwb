#pragma once

#include <string>

#include "Utils.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/base/Data.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An n-dimensional dataset representing a column of a DynamicTable.
 */
class VectorData : public Data
{
public:
  // Register Data class as a registered type
  REGISTER_SUBCLASS(VectorData, "hdmf-common")

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  VectorData(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  DEFINE_FIELD(readDescription,
               AttributeField,
               std::string,
               "description",
               Description of what these vectors represent);

private:
  /**
   * @brief Description of VectorData.
   */
  std::string m_description;
};
}  // namespace AQNWB::NWB
