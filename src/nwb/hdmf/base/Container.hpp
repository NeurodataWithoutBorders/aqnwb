#pragma once

#include <memory>
#include <string>

#include "nwb/RegisteredType.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Abstract data type for a group storing collections of data and
 * metadata
 *
 */
class Container : public RegisteredType
{
public:
  // Register the Container class as a registered type
  REGISTER_SUBCLASS(Container, "hdmf-common")

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Container(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor.
   */
  virtual ~Container();

  /**
   * @brief Initialize the container.
   */
  void initialize();
};

}  // namespace AQNWB::NWB
