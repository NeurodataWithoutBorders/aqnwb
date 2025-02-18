#pragma once

#include "nwb/hdmf/base/Data.hpp"

namespace AQNWB::NWB
{
/**
 * @brief A list of unique identifiers for values within a dataset, e.g. rows of
 * a DynamicTable.
 */
class ElementIdentifiers : public Data<int>
{
public:
  // Register ElementIdentifiers class as a registered type
  REGISTER_SUBCLASS(ElementIdentifiers, "hdmf-common")

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  ElementIdentifiers(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~ElementIdentifiers() override {}
};
}  // namespace AQNWB::NWB
