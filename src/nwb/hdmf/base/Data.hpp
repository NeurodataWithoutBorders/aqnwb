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
class Data : public RegisteredType
{
public:
  REGISTER_SUBCLASS(Data, "hdmf-common")

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Data(const std::string& path, std::shared_ptr<IO::BaseIO> io);

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
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(std::unique_ptr<IO::BaseRecordingData>&& dataset)
  {
    m_dataset = std::move(dataset);
    // setup common attributes
    Status commonAttrsStatus = m_io->createCommonNWBAttributes(
        m_path, this->getNamespace(), this->getTypeName());
    return commonAttrsStatus;
  }

  /**
   * @brief Check whether the m_dataset has been initialized
   */
  inline bool isInitialized() { return m_dataset != nullptr; }

  // Define the data fields to expose for lazy read access
  DEFINE_DATASET_FIELD(readData, recordData, std::any, "", The main data)

  DEFINE_ATTRIBUTE_FIELD(readNeurodataType,
                         std::string,
                         "neurodata_type",
                         The name of the type)

  DEFINE_ATTRIBUTE_FIELD(readNamespace,
                         std::string,
                         "namespace",
                         The name of the namespace)

  std::unique_ptr<IO::BaseRecordingData> m_dataset;
};

/**
 * @brief A typed data container for a dataset.
 *
 * This typed variant of Data allows for the specification of the data type
 * at compile time, enabling type-safe access to the data. This is useful for
 * data read to simplify access when the type is known. While we can use
 * the typed version also for data write, in most case the base version
 * of VectorData is sufficient. NOTE: Only VectorData is registered with the
 * RegisteredType class registry. The VectorDataTyped class is not registered
 * since the DTYPE information is not available as part of the neurodata_type
 * attribute in the NWB file.
 *
 * @tparam DTYPE The data type of the data managed by DataTyped
 */
template<typename DTYPE = std::any>
class DataTyped : public Data
{
public:
  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  DataTyped(const std::string& path, std::shared_ptr<IO::BaseIO> io)
      : Data(path, io)
  {
  }

  /**
   * @brief Virtual destructor.
   */
  virtual ~DataTyped() override {}

  /**
   *  \brief Create a DataTyped object from a Data object
   *
   *  This function is useful when the type of the data is known and we want
   *  read data in a typed manner where the type is stored in the DTYPE template
   *  parameter. NOTE: The original Data object retains ownership of the
   *  Data.m_dataset recording dataset object if it was initialized, i.e.,
   *  the returned DataTyped object will have a nullptr m_dataset.
   *
   *  @param data The Data object to convert
   *  @return A DataTyped object with the same path and IO object as the input
   */
  static std::shared_ptr<DataTyped<DTYPE>> fromData(const Data& data)
  {
    return std::make_shared<DataTyped<DTYPE>>(data.getPath(), data.getIO());
  }

  // Define the data fields to expose for lazy read access
  DEFINE_DATASET_FIELD(readData, recordData, DTYPE, "", The main data)

  using RegisteredType::m_io;
  using RegisteredType::m_path;
};
}  // namespace AQNWB::NWB
