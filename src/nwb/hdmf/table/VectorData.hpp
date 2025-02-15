#pragma once

#include <string>

#include "Utils.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/base/Data.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An n-dimensional dataset representing a column of a DynamicTable.
 * @tparam DTYPE The data type of the data managed by VectorData
 */
template<typename DTYPE = std::any>
class VectorData : public Data<DTYPE>
{
public:
  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  VectorData(const std::string& path, std::shared_ptr<IO::BaseIO> io)
      : Data<DTYPE>(path, io)
  {
  }

  // Register the Data template class with the type registry for dynamic
  // creation. This registration is generic and applies to any specialization of
  // the Data template. It allows the system to dynamically create instances of
  // Data with different data types.
  REGISTER_SUBCLASS_WITH_TYPENAME(VectorData<DTYPE>,
                                  "hdmf-common",
                                  "VectorData")

  /**
   * @brief Virtual destructor.
   */
  virtual ~VectorData() override {}

  /**
   *  @brief Initialize the dataset for the Data object
   *
   *  This functions takes ownership of the passed rvalue unique_ptr and moves
   *  ownership to its internal m_dataset variable
   *
   * @param dataset The rvalue unique pointer to the BaseRecordingData object
   * @param description The description of the VectorData
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(std::unique_ptr<AQNWB::IO::BaseRecordingData>&& dataset,
                    const std::string& description)
  {
    Status dataStatus = Data<DTYPE>::initialize(std::move(dataset));
    Status attrStatus =
        m_io->createAttribute(description, m_path, "description");
    return dataStatus && attrStatus;
  }

  // Define the data fields to expose for lazy read access
  using RegisteredType::m_io;
  using RegisteredType::m_path;

  DEFINE_FIELD(readDescription,
               AttributeField,
               std::string,
               "description",
               Description of what these vectors represent)
};
}  // namespace AQNWB::NWB
