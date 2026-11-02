#pragma once

#include <memory>
#include <optional>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "nwb/device/DeviceModel.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Metadata about a specific instance of a data acquisition device, e.g.,
 * recording system, electrode, microscope.
 *
 * Information about the model of the device (e.g., the manufacturer) is stored
 * in a DeviceModel that the Device links to (NWB >= 2.11).
 */
class Device : public NWBContainer
{
public:
  // Register the Device as a subclass of NWBContainer
  REGISTER_SUBCLASS(Device, NWBContainer, AQNWB::SPEC::CORE::namespaceName)

  // Bring base class initialize method into scope
  using Container::initialize;

  /**
   * @brief Specification of the attributes of a Device.
   *
   * All attributes are optional and only written if set.
   */
  struct DeviceSpec
  {
    /// @brief Description of the device as free-form text.
    std::optional<std::string> description = std::nullopt;
    /// @brief The serial number of the device.
    std::optional<std::string> serialNumber = std::nullopt;
    /// @brief The model of the device (stored as a link to the DeviceModel).
    std::shared_ptr<DeviceModel> model = nullptr;

    /**
     * @brief Set the description of the device.
     * @param value Description of the device as free-form text.
     * @return Reference to this DeviceSpec, to allow method chaining.
     */
    DeviceSpec& withDescription(std::string value)
    {
      description = std::move(value);
      return *this;
    }

    /**
     * @brief Set the serial number of the device.
     * @param value The serial number of the device.
     * @return Reference to this DeviceSpec, to allow method chaining.
     */
    DeviceSpec& withSerialNumber(std::string value)
    {
      serialNumber = std::move(value);
      return *this;
    }

    /**
     * @brief Set the model of the device.
     * @param value The DeviceModel of the device (see
     * NWBFile::createDeviceModel). The model must be in the same file as the
     * device.
     * @return Reference to this DeviceSpec, to allow method chaining.
     */
    DeviceSpec& withModel(std::shared_ptr<DeviceModel> value)
    {
      model = std::move(value);
      return *this;
    }
  };

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
   * @brief Initializes the device by creating the group, writing its
   * attributes, and linking it to its model (if set).
   *
   * @param spec The attributes of the device.
   * @return Status::Success if successful, otherwise Status::Failure (e.g., if
   * the model is in a different file than the device).
   */
  Status initialize(const DeviceSpec& spec);

  /**
   * @brief Initializes the device and writes the deprecated `manufacturer`
   * attribute.
   *
   * @deprecated The `manufacturer` attribute of Device is deprecated in NWB
   * 2.11. Use initialize(const DeviceSpec&) and store the manufacturer in a
   * DeviceModel (see NWBFile::createDeviceModel) instead.
   *
   * @param description The description of the device (not written if empty).
   * @param manufacturer The manufacturer of the device.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  [[deprecated(
      "The manufacturer attribute of Device is deprecated in NWB 2.11. Use "
      "initialize(const DeviceSpec&) and store the manufacturer in a "
      "DeviceModel instead.")]]
  Status initialize(const std::string& description,
                    const std::string& manufacturer);

  // Define the data fields to expose for lazy read access
  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         Description of the device)

  DEFINE_ATTRIBUTE_FIELD(readSerialNumber,
                         std::string,
                         "serial_number",
                         The serial number of the device)

  DEFINE_REGISTERED_FIELD(readModel,
                          DeviceModel,
                          "model",
                          The model of the device)

  DEFINE_ATTRIBUTE_FIELD(readManufacturer,
                         std::string,
                         "manufacturer",
                         DEPRECATED in NWB 2.11 : manufacturer of the device in
                             files written with earlier versions)

  DEFINE_ATTRIBUTE_FIELD(readModelNumber,
                         std::string,
                         "model_number",
                         DEPRECATED in NWB 2.11 : model number of the device in
                             files written with earlier versions)

  DEFINE_ATTRIBUTE_FIELD(readModelName,
                         std::string,
                         "model_name",
                         DEPRECATED in NWB 2.11 : model name of the device in
                             files written with earlier versions)
};
}  // namespace AQNWB::NWB
