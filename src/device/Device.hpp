#pragma once

#include <string>
#include "hdmf/base/Container.hpp"
#include "io/BaseIO.hpp"

using namespace AQNWBIO;

/**

Metadata about a data acquisition device, e.g., recording system, electrode, microscope.

*/
class Device : public Container
{
public:
  /** Constructor */
  Device(std::string path, std::shared_ptr<BaseIO> io);

  /** Destructor */
  ~Device();

  /** Initialization function */
  void initialize();

  std::string manufacturer = "unknown";
  std::string description = "description";
};
