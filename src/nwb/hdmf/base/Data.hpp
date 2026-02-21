#pragma once

#include <any>
#include <memory>

#include "io/BaseIO.hpp"
#include "nwb/RegisteredType.hpp"
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An abstract data type for a dataset.
 * @tparam DTYPE The data type of the data managed by Data
 */
class Data : public RegisteredType
{
public:
  // Register the Data class with the type registry
  REGISTER_SUBCLASS(Data, RegisteredType, "hdmf-common")

protected:
  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Data(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~Data() override {}

  /**
   *  @brief Initialize the dataset for the Data object
   *
   *  This function creates a dataset using the provided configuration
   *
   * @param dataConfig The configuration for the dataset
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const IO::BaseArrayDataSetConfig& dataConfig);

  /**
   * @brief Check whether the dataset has been initialized
   */
  inline bool isInitialized() { return this->readData()->exists(); }

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
};

/**
 * @brief A typed data container for a dataset.
 *
 * This typed variant of Data allows for the specification of the data type
 * at compile time, enabling type-safe access to the data. This is useful for
 * data read to simplify access when the type is known. While we can use
 * the typed version also for data write, in most case the base version
 * of Data is sufficient. NOTE: Only Data is registered with the
 * RegisteredType class registry. The DataTyped class is not registered
 * since the DTYPE information is not available as part of the neurodata_type
 * attribute in the NWB file.
 *
 * @tparam DTYPE The data type of the data managed by DataTyped
 */
template<typename DTYPE = std::any>
class DataTyped : public Data
{
  friend class AQNWB::NWB::RegisteredType; /* base can call constructor */

protected:
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

  using Data::Data; /* inherit from immediate base */

public:
  /** \brief Factor method to create a DataTyped object.
   *
   * This is required here since DataTyped is a template class and
   * is not being registered with the RegisteredType class registry via
   * REGISTER_SUBCLASS.
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   * @return A shared pointer to the created NWBFile object, or nullptr if
   * creation failed.
   */
  static std::shared_ptr<DataTyped> create(
      const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
  {
    return RegisteredType::create<DataTyped>(path, io);
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
  static std::shared_ptr<DataTyped<DTYPE>> fromData(
      const std::shared_ptr<Data>& data)
  {
    return DataTyped<DTYPE>::create(data->getPath(), data->getIO());
  }

  // Define the data fields to expose for lazy read access
  DEFINE_DATASET_FIELD(readData, recordData, DTYPE, "", The main data)

  using RegisteredType::getIO;
  using RegisteredType::getPath;
};
}  // namespace AQNWB::NWB
