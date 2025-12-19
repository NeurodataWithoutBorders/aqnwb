#pragma once

// Common STL includes
#include <memory>
#include <optional>
#include <string>
#include <vector>
// Base AqNWB includes for IO and RegisteredType
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/RegisteredType.hpp"
// Include for parent type
#include "nwb/hdmf/base/Container.hpp"
// Include for the namespace schema header
#include "spec/core.hpp"

namespace AQNWB::NWB
{

/**
 * @brief An abstract data type for a generic container storing collections of
 * data and metadata. Base type for all data and metadata containers.
 */
class NWBContainer : public AQNWB::NWB::Container
{
public:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  NWBContainer(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~NWBContainer() override {}

  /**
   * @brief Initialize the object
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize();

  REGISTER_SUBCLASS(NWBContainer, Container, AQNWB::SPEC::CORE::namespaceName)
};

}  // namespace AQNWB::NWB
