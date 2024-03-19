#pragma once

#include <string>
#include <memory>
#include "io/BaseIO.hpp"

using namespace AQNWBIO;

/**

Abstract data type for a group storing collections of data and metadata

*/
class Container 
{
public:
  /** Constructor */
  Container(std::string path, std::shared_ptr<BaseIO> io); 

  /** Destructor */
  virtual ~Container();

protected:
  std::string path;
  std::shared_ptr<BaseIO> io;
};
