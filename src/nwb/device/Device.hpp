#pragma once

#include <string>

#include "io/BaseIO.hpp"
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
  REGISTER_SUBCLASS(Device)

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

  /**
   * @brief Gets the manufacturer of the device.
   * @return The manufacturer of the device.
   */
  std::string getManufacturer() const;

  /**
   * @brief Gets the description of the device.
   * @return The description of the device.
   */
  std::string getDescription() const;
};
}  // namespace AQNWB::NWB
