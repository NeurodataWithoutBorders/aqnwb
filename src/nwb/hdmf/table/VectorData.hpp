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

  /**
   *  @brief Initialize the dataset for the Data object
   *
   *  This functions takes ownership of the passed rvalue unique_ptr and moves
   *  ownership to its internal m_dataset variable
   *
   * @param dataset The rvalue unique pointer to the BaseRecordingData object
   * @param description The description of the VectorData
   */
  void initialize(std::unique_ptr<AQNWB::IO::BaseRecordingData>&& dataset,
                  const std::string& description);

  DEFINE_FIELD(readDescription,
               AttributeField,
               std::string,
               "description",
               Description of what these vectors represent)

private:
  /**
   * @brief Description of VectorData.
   */
  std::string m_description;
};
}  // namespace AQNWB::NWB
