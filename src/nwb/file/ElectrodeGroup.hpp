#pragma once

#include <any>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief The ElectrodeGroup class represents a physical grouping of electrodes,
 * e.g. a shank of an array.
 */
class ElectrodeGroup : public Container
{
public:
  // Register ElectrodeGroup as a subclass of Container
  REGISTER_SUBCLASS(ElectrodeGroup, AQNWB::SPEC::CORE::namespaceName)

  // Bring base class initialize method into scope
  using Container::initialize;

  /**
   * @brief Constructor.
   * @param path The location in the file of the electrode group.
   * @param io A shared pointer to the IO object.
   */
  ElectrodeGroup(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor.
   */
  ~ElectrodeGroup();

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
   */
  void initialize(const std::string& description,
                  const std::string& location,
                  const Device& device);

  DEFINE_DATASET_FIELD(readData,
                       recordData,
                       std::any,
                       "position",
                       Stereotaxis or common framework coordinates)
};
}  // namespace AQNWB::NWB
