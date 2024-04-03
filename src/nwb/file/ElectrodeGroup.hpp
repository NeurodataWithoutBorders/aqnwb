#pragma once

#include <string>

#include "BaseIO.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/hdmf/base/Container.hpp"

namespace AQNWB
{
namespace NWB
{
/**
 * @brief The ElectrodeGroup class represents a physical grouping of electrodes,
 * e.g. a shank of an array.
 */
class ElectrodeGroup : public Container
{
public:
  /**
   * @brief Constructor.
   * @param path The location in the file of the electrode group.
   * @param io A shared pointer to the IO object.
   * @param description The description of the electrode group.
   * @param location The location of electrode group within the subject e.g.
   * brain region.
   * @param device The device associated with the electrode group.
   */
  ElectrodeGroup(const std::string& path,
                 std::shared_ptr<BaseIO> io,
                 const std::string& description,
                 const std::string& location,
                 const Device& device);

  /**
   * @brief Destructor.
   */
  ~ElectrodeGroup();

  /**
   * @brief Initializes the ElectrodeGroup object.
   *
   * Initializes the ElectrodeGroup by creating NWB related attributes and
   * linking to the Device object.
   */
  void initialize();

  /**
   * @brief Gets the description of the electrode group.
   * @return The description of the electrode group.
   */
  std::string getDescription() const;

  /**
   * @brief Gets the location of the electrode group.
   * @return The location of the electrode group.
   */
  std::string getLocation() const;

  /**
   * @brief Gets the device associated with the electrode group.
   * @return The device associated with the electrode group.
   */
  const Device& getDevice() const;

private:
  /**
   * @brief The description of the electrode group.
   */
  std::string description;

  /**
   * @brief The location of electrode group within the subject e.g. brain
   * region.
   */
  std::string location;

  /**
   * @brief The device associated with the electrode group.
   */
  Device device;
};
}  // namespace NWB
}  // namespace AQNWB
