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
  // Register the NWBDataInterface as a subclass of NWBContainer
  REGISTER_SUBCLASS(NWBDataInterface,
                    NWBContainer,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a NWBDataInterface.
   *
   * NWBDataInterface does not define any additional fields.
   */
  struct Spec : public NWBContainer::Spec
  {
    REGISTER_SPEC(NWBDataInterface, NWBContainer::Spec)
  };

  /**
   * @brief Virtual destructor.
   */
  virtual ~NWBDataInterface() override {}

  /**
   * @brief Initialize the object
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize();

  /**
   * @brief Initialize the object from a spec.
   * @param spec The specification of the object.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

protected:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  NWBDataInterface(const std::string& path,
                   std::shared_ptr<AQNWB::IO::BaseIO> io);
};

}  // namespace AQNWB::NWB
