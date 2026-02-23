#pragma once

#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Metadata about a data acquisition device, e.g., recording system,
 * electrode, microscope.
 */
class Device : public NWBContainer
{
public:
  // Register the Device as a subclass of NWBContainer
  REGISTER_SUBCLASS(Device, NWBContainer, AQNWB::SPEC::CORE::namespaceName)

  // Bring base class initialize method into scope
  using Container::initialize;

protected:
  /**
   * @brief Constructor.
   * @param path The location of the device in the file.
   * @param io A shared pointer to the IO object.
   */
  Device(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Destructor
   */
  ~Device() override;

  /**
   * @brief Initializes the device by creating NWB related attributes and
   * writing the manufactor and description metadata.
   *
   * @param description The description of the device.
   * @param manufacturer The manufacturer of the device.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::string& description,
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
