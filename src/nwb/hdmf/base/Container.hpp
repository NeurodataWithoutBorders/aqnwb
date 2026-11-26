#pragma once

#include <memory>
#include <string>

#include "nwb/RegisteredType.hpp"
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Abstract data type for a group storing collections of data and
 * metadata
 *
 */
class Container : public RegisteredType
{
public:
  // Register the Container class as a registered type
  REGISTER_SUBCLASS(Container,
                    RegisteredType,
                    AQNWB::SPEC::HDMF_COMMON::namespaceName)

  /**
   * @brief Specification for initializing a Container.
   *
   * Container does not define any fields. This is the root of the spec
   * hierarchy for all group-based neurodata_types.
   */
  struct Spec : public RegisteredTypeSpec
  {
    REGISTER_SPEC(Container, RegisteredTypeSpec)
  };

protected:
  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Container(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Destructor.
   */
  ~Container() override;

  /**
   * @brief Initialize the container.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize();

  /**
   * @brief Initialize the container from a spec.
   *
   * Validates the spec and creates the group with the common attributes.
   *
   * @param spec The specification of the container.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  // Define the data fields to expose for lazy read access
  DEFINE_ATTRIBUTE_FIELD(readNeurodataType,
                         std::string,
                         "neurodata_type",
                         The name of the type)

  DEFINE_ATTRIBUTE_FIELD(readNamespace,
                         std::string,
                         "namespace",
                         The name of the namespace)
};

}  // namespace AQNWB::NWB
