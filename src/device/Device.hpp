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
  Device(std::string path, std::shared_ptr<BaseIO> io, std::string description, std::string manufacturer);

  /** Destructor */
  ~Device();

  /** Initialization function */
  void initialize();

  // Getter for manufacturer
  std::string getManufacturer() const;

  // Getter for description
  std::string getDescription() const;
  
private:
  std::string description;
  std::string manufacturer;
};
