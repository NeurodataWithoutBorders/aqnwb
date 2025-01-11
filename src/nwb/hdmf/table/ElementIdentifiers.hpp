#pragma once

#include "nwb/hdmf/base/Data.hpp"

namespace AQNWB::NWB
{
/**
 * @brief A list of unique identifiers for values within a dataset, e.g. rows of
 * a DynamicTable.
 */
class ElementIdentifiers : public Data
{
public:
  // Register Data class as a registered type
  REGISTER_SUBCLASS(ElementIdentifiers, "hdmf-common")

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  ElementIdentifiers(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  // Define the data fields to expose for lazy read access
  DEFINE_FIELD(readData, DatasetField, int, "", The data identifiers)
};
}  // namespace AQNWB::NWB
