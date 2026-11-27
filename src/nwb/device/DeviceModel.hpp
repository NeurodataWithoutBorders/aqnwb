#pragma once

#include <optional>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Model properties of a data acquisition device, e.g., recording
 * system, electrode, microscope.
 *
 * The name of the DeviceModel should be the most common representation of the
 * model name, e.g., "Neuropixels 1.0". DeviceModels are stored in
 * `/general/devices/models`, and a Device links to its model (see
 * NWBFile::createDeviceModel and Device::DeviceSpec::withModel).
 */
class DeviceModel : public NWBContainer
{
public:
  // Register the DeviceModel as a subclass of NWBContainer
  REGISTER_SUBCLASS(DeviceModel, NWBContainer, AQNWB::SPEC::CORE::namespaceName)

  // Bring base class initialize method into scope
  using Container::initialize;

  /**
   * @brief Specification of the attributes of a DeviceModel.
   *
   * The manufacturer is required; the other attributes are optional and only
   * written if set.
   */
  struct DeviceModelSpec
  {
    /**
     * @brief Constructor.
     * @param manufacturerName The name of the manufacturer of the device model,
     * e.g., Imec, Plexon, Thorlabs.
     */
    explicit DeviceModelSpec(std::string manufacturerName)
        : manufacturer(std::move(manufacturerName))
    {
    }

    /// @brief The name of the manufacturer of the device model.
    std::string manufacturer;
    /// @brief The model number (or part/product number) of the device, e.g.,
    /// PRB_1_4_0480_1.
    std::optional<std::string> modelNumber = std::nullopt;
    /// @brief Description of the device model as free-form text.
    std::optional<std::string> description = std::nullopt;

    /**
     * @brief Set the model number of the device model.
     * @param value The model number (or part/product number) of the device.
     * @return Reference to this DeviceModelSpec, to allow method chaining.
     */
    DeviceModelSpec& withModelNumber(std::string value)
    {
      modelNumber = std::move(value);
      return *this;
    }

    /**
     * @brief Set the description of the device model.
     * @param value Description of the device model as free-form text.
     * @return Reference to this DeviceModelSpec, to allow method chaining.
     */
    DeviceModelSpec& withDescription(std::string value)
    {
      description = std::move(value);
      return *this;
    }
  };

protected:
  /**
   * @brief Constructor.
   * @param path The location of the device model in the file.
   * @param io A shared pointer to the IO object.
   */
  DeviceModel(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Destructor
   */
  ~DeviceModel() override;

  /**
   * @brief Initializes the device model by creating the group and writing its
   * attributes.
   *
   * @param spec The attributes of the device model.
   * @return Status::Success if successful, otherwise Status::Failure (e.g., if
   * the manufacturer is empty).
   */
  Status initialize(const DeviceModelSpec& spec);

  // Define the data fields to expose for lazy read access
  DEFINE_ATTRIBUTE_FIELD(readManufacturer,
                         std::string,
                         "manufacturer",
                         The name of the manufacturer of the device model)

  DEFINE_ATTRIBUTE_FIELD(readModelNumber,
                         std::string,
                         "model_number",
                         The model number of the device)

  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         Description of the device model)
};
}  // namespace AQNWB::NWB
