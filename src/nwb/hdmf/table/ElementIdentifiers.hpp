#pragma once

#include "nwb/hdmf/base/Data.hpp"
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{
/**
 * @brief A list of unique identifiers for values within a dataset, e.g. rows of
 * a DynamicTable.
 */
class ElementIdentifiers : public Data
{
public:
  // Register ElementIdentifiers class as a registered type
  REGISTER_SUBCLASS(ElementIdentifiers, AQNWB::SPEC::HDMF_COMMON::namespaceName)

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

  using RegisteredType::m_io;
  using RegisteredType::m_path;
  DEFINE_DATASET_FIELD(readData, recordData, int, "", The main data)
};
}  // namespace AQNWB::NWB
