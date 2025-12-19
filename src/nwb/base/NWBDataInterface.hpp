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
#include "nwb/base/NWBContainer.hpp"
// Include for the namespace schema header
#include "spec/core.hpp"

namespace AQNWB::NWB
{

/**
 * @brief An abstract data type for a generic container storing collections of
 * data, as opposed to metadata.
 */
class NWBDataInterface : public AQNWB::NWB::NWBContainer
{
public:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  NWBDataInterface(const std::string& path,
                   std::shared_ptr<AQNWB::IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~NWBDataInterface() override {}

  /**
   * @brief Initialize the object
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize();

  REGISTER_SUBCLASS(NWBDataInterface, NWBContainer, AQNWB::SPEC::CORE::namespaceName)
};

}  // namespace AQNWB::NWB
