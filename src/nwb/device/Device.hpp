#pragma once

#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/base/Container.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Metadata about a data acquisition device, e.g., recording system,
 * electrode, microscope.
 */
class Device : public Container
{
public:
  // Register the Device as a subclass of Container
  REGISTER_SUBCLASS(Device, "core")

  /**
   * @brief Constructor.
   * @param path The location of the device in the file.
   * @param io A shared pointer to the IO object.
   */
  Device(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor
   */
  ~Device();

  /**
   * @brief Initializes the device by creating NWB related attributes and
   * writing the manufactor and description metadata.
   *
   * @param description The description of the device.
   * @param manufacturer The manufacturer of the device.
   */
  void initialize(const std::string& description,
                  const std::string& manufacturer);

  // Define the data fields to expose for lazy read access
  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         Description of the series)

  DEFINE_ATTRIBUTE_FIELD(readManufacturer,
                         std::string,
                         "manufacturer",
                         Manufacturer of the device)
};
}  // namespace AQNWB::NWB
