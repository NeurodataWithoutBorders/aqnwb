#pragma once

#include <string>

#include "../../BaseIO.hpp"
#include "../hdmf/base/Container.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Metadata about a data acquisition device, e.g., recording system,
 * electrode, microscope.
 */
class Device : public Container
{
public:
  /**
   * @brief Constructor.
   * @param path The location of the device in the file.
   * @param io A shared pointer to the IO object.
   * @param description The description of the device.
   * @param manufacturer The manufacturer of the device.
   */
  Device(const std::string& path,
         std::shared_ptr<BaseIO> io,
         const std::string& description,
         const std::string& manufacturer);

  /**
   * @brief Destructor
   */
  ~Device();

  /**
   * @brief Initializes the device by creating NWB related attributes and
   * writing the manufactor and description metadata.
   */
  void initialize();

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

private:
  /**
   * @brief The description of the device.
   */
  std::string description;

  /**
   * @brief The manufacturer of the device.
   */
  std::string manufacturer;
};
}  // namespace AQNWB::NWB
