#pragma once

#include <any>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "nwb/device/Device.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief The ElectrodeGroup class represents a physical grouping of electrodes,
 * e.g. a shank of an array.
 */
class ElectrodeGroup : public NWBContainer
{
public:
  // Register ElectrodeGroup as a subclass of NWBContainer
  REGISTER_SUBCLASS(ElectrodeGroup,
                    NWBContainer,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing an ElectrodeGroup.
   */
  struct Spec : public NWBContainer::Spec
  {
    REGISTER_SPEC(ElectrodeGroup, NWBContainer::Spec)

    /**
     * @brief Description of this electrode group. The description attribute is
     * not written if empty.
     */
    DEFINE_SPEC_FIELD(Spec, std::string, description, withDescription, "")
    /**
     * @brief Location of electrode group. Specify the area or layer or brain
     * region. Required.
     */
    DEFINE_SPEC_FIELD(Spec, std::string, location, withLocation, "")
    /**
     * @brief The device that was used to record from this electrode group.
     * Required.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::shared_ptr<Device>, device, withDevice, nullptr)

    /**
     * @brief Validate the spec.
     *
     * Checks that the location is not empty and that the device is set.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

protected:
  /**
   * @brief Constructor.
   * @param path The location in the file of the electrode group.
   * @param io A shared pointer to the IO object.
   */
  ElectrodeGroup(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Destructor.
   */
  ~ElectrodeGroup() override;

  /**
   * @brief Initializes the ElectrodeGroup object.
   *
   * Initializes the ElectrodeGroup by creating NWB related attributes and
   * linking to the Device object.
   *
   * @param description The description of the electrode group.
   * @param location The location of electrode group within the subject e.g.
   * brain region.
   * @param device The device associated with the electrode group.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  [[deprecated(
      "Use ElectrodeGroup::initialize(const ElectrodeGroup::Spec&) instead")]]
  Status initialize(const std::string& description,
                    const std::string& location,
                    const std::shared_ptr<Device>& device);

  /**
   * @brief Initializes the ElectrodeGroup from a spec.
   *
   * Creates the NWB related attributes and the link to the device.
   *
   * @param spec The specification of the ElectrodeGroup.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_DATASET_FIELD(readPosition,
                       recordPosition,
                       std::any,
                       "position",
                       Stereotaxic or common framework coordinates)
};
}  // namespace AQNWB::NWB
