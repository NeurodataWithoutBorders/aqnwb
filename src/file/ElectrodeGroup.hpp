#pragma once

#include <string>
#include "hdmf/base/Container.hpp"
#include "io/BaseIO.hpp"
#include "device/Device.hpp"

using namespace AQNWBIO;

/**

A physical grouping of electrodes, e.g. a shank of an array.

*/
class ElectrodeGroup : public Container
{
public:
  /** Constructor */
  ElectrodeGroup(std::string path, std::shared_ptr<BaseIO> io, std::string description, std::string location, Device device);

  /** Destructor */
  ~ElectrodeGroup();

  /** Initialization function */
  void initialize();

  /** Getter for description */
  std::string getDescription() const;

  /** Getter for location */
  std::string getLocation() const;

  /** Getter for devicePath */
  Device getDevice() const;

private:
  std::unique_ptr<BaseRecordingData> positionDataset;
  std::string description;
  std::string location;
  Device device;
};  
