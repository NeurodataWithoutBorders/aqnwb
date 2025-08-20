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
  REGISTER_SUBCLASS(Container, AQNWB::SPEC::HDMF_COMMON::namespaceName)

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Container(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor.
   */
  virtual ~Container();

  /**
   * @brief Initialize the container.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize() override;

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
