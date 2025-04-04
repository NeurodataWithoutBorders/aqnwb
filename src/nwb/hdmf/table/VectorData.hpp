#pragma once

#include <string>

#include "Utils.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/base/Data.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An n-dimensional dataset representing a column of a DynamicTable.
 */
class VectorData : public Data
{
public:
  REGISTER_SUBCLASS(VectorData, "hdmf-common")

  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  VectorData(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~VectorData() override {}

  /**
   *  @brief Initialize the dataset for the VectorData object
   *
   *  This functions takes ownership of the passed rvalue unique_ptr and moves
   *  ownership to its internal m_dataset variable
   *
   * @param dataset The rvalue unique pointer to the BaseRecordingData object
   * @param description The description of the VectorData
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(std::unique_ptr<IO::BaseRecordingData>&& dataset,
                    const std::string& description)
  {
    Status dataStatus = Data::initialize(std::move(dataset));
    Status attrStatus =
        m_io->createAttribute(description, m_path, "description");
    return dataStatus && attrStatus;
  }

  DEFINE_FIELD(readDescription,
               AttributeField,
               std::string,
               "description",
               Description of what these vectors represent)
};

/**
 * @brief A typed n-dimensional dataset representing a column of a DynamicTable.
 *
 * This typed variant of VectorData allows for the specification of the data
 * type at compile time, enabling type-safe access to the data. This is useful
 * for data read to simplify access when the type is known. While we can use the
 * typed version also for data write, in most case the base version of
 * VectorData is sufficient. NOTE: Only VectorData is registered with the
 * RegisteredType class registry. The VectorDataTyped class is not registered
 * since the DTYPE information is not available as part of the neurodata_type
 * attribute in the NWB file.
 *
 * @tparam DTYPE The data type of the data managed by VectorDataTyped
 */
template<typename DTYPE = std::any>
class VectorDataTyped : public VectorData
{
public:
  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  VectorDataTyped(const std::string& path, std::shared_ptr<IO::BaseIO> io)
      : VectorData(path, io)
  {
  }

  /**
   * @brief Virtual destructor.
   */
  virtual ~VectorDataTyped() override {}

  /**
   *  \brief Create a VectorDataTyped object from a Data object
   *
   *  This function is useful when the type of the data is known and we want
   *  read data in a typed manner where the type is stored in the DTYPE template
   *  parameter. NOTE: The original Data object retains ownership of the
   *  Data.m_dataset recording dataset object if it was initialized, i.e.,
   *  the returned VectorDataTyped object will have a nullptr m_dataset.
   *
   *  @param data The Data object to convert
   *  @return A shared pointer for VectorDataTyped object with the same path and
   * IO object as the input
   */
  static std::shared_ptr<VectorDataTyped<DTYPE>> fromVectorData(
      const VectorData& data)
  {
    return std::make_shared<VectorDataTyped<DTYPE>>(data.getPath(),
                                                    data.getIO());
  }

  using RegisteredType::m_io;
  using RegisteredType::m_path;

  // Define the data fields to expose for lazy read access
  DEFINE_FIELD(readData, DatasetField, DTYPE, "", The main data)
};
}  // namespace AQNWB::NWB
