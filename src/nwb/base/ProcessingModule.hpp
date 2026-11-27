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
// Includes for types that are referenced and used
#include "nwb/base/NWBDataInterface.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
// Include for the namespace schema header
#include "spec/core.hpp"

namespace AQNWB::NWB
{

/**
 * @brief A collection of processed data.
 */
class ProcessingModule : public NWBContainer
{
public:
  // Register the ProcessingModule as a subclass of NWBContainer
  REGISTER_SUBCLASS(ProcessingModule,
                    NWBContainer,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a ProcessingModule.
   */
  struct Spec : public NWBContainer::Spec
  {
    /**
     * @brief Description of this collection of processed data.
     */
    DEFINE_SPEC_FIELD(Spec, std::string, description, withDescription, "")
  };

  /**
   * @brief Virtual destructor.
   */
  virtual ~ProcessingModule() override {}

  /**
   * @brief Initialize the object.
   * @param description Description of this collection of processed data.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  [[deprecated(
      "Use ProcessingModule::initialize(const ProcessingModule::Spec&) "
      "instead")]]
  Status initialize(const std::string& description);

  /**
   * @brief Initialize the object from a spec.
   * @param spec The specification of the ProcessingModule.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         Description of this collection of processed data.)

  DEFINE_UNNAMED_REGISTERED_FIELD(readNWBDataInterface,
                                  createNWBDataInterface,
                                  NWBDataInterface,
                                  "",
                                  Data objects stored in this collection.)

  DEFINE_UNNAMED_REGISTERED_FIELD(readDynamicTable,
                                  createDynamicTable,
                                  DynamicTable,
                                  "",
                                  Tables stored in this collection.)

protected:
  /**
   * @brief Constructor.
   * @param path Path to the object in the file.
   * @param io IO object for reading/writing.
   */
  ProcessingModule(const std::string& path,
                   std::shared_ptr<AQNWB::IO::BaseIO> io);
};

}  // namespace AQNWB::NWB
