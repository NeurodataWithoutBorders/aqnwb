#pragma once

#include <string>
#include "hdmf/base/Container.hpp"
#include "io/BaseIO.hpp"

using namespace AQNWBIO;

/**

A physical grouping of electrodes, e.g. a shank of an array.

*/
class ElectrodeGroup : public Container
{
public:
  /** Constructor */
  ElectrodeGroup(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  ~ElectrodeGroup();

  /** Initialization function */
  void initialize();

  /** Add link to acquisition device */
  void linkDevice();

  std::unique_ptr<BaseRecordingData> positionDataset;
  std::string device;  // path to be linked
  std::string description = "description";
  std::string location = "unknown";
};  
