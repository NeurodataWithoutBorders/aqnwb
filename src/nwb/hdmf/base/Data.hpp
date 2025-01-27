#pragma once

#include <memory>

#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An abstract data type for a dataset.
 * @tparam DTYPE The data type of the data managed by Data
 */
template<typename DTYPE = std::any>
class Data : public RegisteredType
{
public:
  // Register Data class as a registered type
  REGISTER_SUBCLASS_WITH_TYPENAME(Data<DTYPE>, "hdmf-common", "Data")

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Data(const std::string& path, std::shared_ptr<IO::BaseIO> io)
      : RegisteredType(path, io)
  {
  }

  /**
   * @brief Virtual destructor.
   */
  virtual ~Data() override {}

  /**
   *  @brief Initialize the dataset for the Data object
   *
   *  This functions takes ownership of the passed rvalue unique_ptr and moves
   *  ownership to its internal m_dataset variable
   *
   * @param dataset The rvalue unique pointer to the BaseRecordingData object
   */
  void initialize(std::unique_ptr<IO::BaseRecordingData>&& dataset)
  {
    m_dataset = std::move(dataset);
    // setup common attributes
    m_io->createCommonNWBAttributes(
        m_path, this->getNamespace(), this->getTypeName());
  }

  /**
   * @brief Check whether the m_dataset has been initialized
   */
  inline bool isInitialized() { return m_dataset != nullptr; }

  std::unique_ptr<IO::BaseRecordingData> m_dataset;

  // Define the data fields to expose for lazy read access
  DEFINE_FIELD(readData, DatasetField, DTYPE, "", The main data)

  DEFINE_FIELD(readNeurodataType,
               AttributeField,
               std::string,
               "neurodata_type",
               The name of the type)

  DEFINE_FIELD(readNamespace,
               AttributeField,
               std::string,
               "namespace",
               The name of the namespace)
};
}  // namespace AQNWB::NWB
