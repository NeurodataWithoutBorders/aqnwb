#pragma once

#include "nwb/hdmf/base/Data.hpp"
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{
/**
 * @brief A list of unique identifiers for values within a dataset, e.g. rows of
 * a DynamicTable.
 */
class ElementIdentifiers : public Data
{
public:
  struct DataSpec : public Data::DataSpec<ElementIdentifiers>
  {
    DataSpec(const std::string& name, const IO::ArrayDataSetConfig& dataConfig)
        : Data::DataSpec<ElementIdentifiers>(name, dataConfig)
    {
    }

    Status initialize(Data& data) const override
    {
      auto* elementIdentifiers = dynamic_cast<ElementIdentifiers*>(&data);
      if (!elementIdentifiers) {
        std::cerr << "ElementIdentifiers::DataSpec::initialize received "
                     "incompatible Data object"
                  << std::endl;
        return Status::Failure;
      }
      return elementIdentifiers->initialize(
          static_cast<const IO::ArrayDataSetConfig&>(*this));
    }
  };

  // Register ElementIdentifiers class as a registered type
  REGISTER_SUBCLASS(ElementIdentifiers,
                    Data,
                    AQNWB::SPEC::HDMF_COMMON::namespaceName)

protected:
  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  ElementIdentifiers(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~ElementIdentifiers() override {}

  using RegisteredType::m_io;
  using RegisteredType::m_path;

  static std::shared_ptr<DataSpec> createDataSpec(
      const std::string& name, const IO::ArrayDataSetConfig& dataConfig)
  {
    return std::make_shared<DataSpec>(name, dataConfig);
  }

  Status initialize(const IO::ArrayDataSetConfig& dataConfig)
  {
    return Data::initialize(dataConfig);
  }

  DEFINE_DATASET_FIELD(readData, recordData, int, "", The main data)
};
}  // namespace AQNWB::NWB
