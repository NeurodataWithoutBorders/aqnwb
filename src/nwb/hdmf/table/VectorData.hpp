#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "Types.hpp"
#include "Utils.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/base/Data.hpp"
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{

using CellValue = AQNWB::CellValue;

/**
 * @brief An n-dimensional dataset representing a column of a DynamicTable.
 */
class VectorData : public Data
{
public:
  /**
   * @brief Runtime configuration for creating and initializing a VectorData
   * column.
   */
  struct DataSpec : public Data::DataSpec<VectorData>
  {
    DataSpec(const std::string& datasetName,
             const IO::ArrayDataSetConfig& dataConfig,
             const std::string& columnDescription);

    virtual ~DataSpec() = default;

    std::string description;

    Status initialize(Data& data) const override;
  };

  REGISTER_SUBCLASS(VectorData, Data, AQNWB::SPEC::HDMF_COMMON::namespaceName)

protected:
  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  VectorData(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~VectorData() override {}

  /**
   * @brief Create a VectorData object with a reference dataset
   *
   * @param path The path of the container
   * @param io A shared pointer to the IO object
   * @param description The description of the VectorData
   * @param references The vector of references
   * @return A shared pointer to the created VectorData object, or nullptr if
   * creation failed
   */
  static std::shared_ptr<VectorData> createReferenceVectorData(
      const std::string& path,
      std::shared_ptr<IO::BaseIO> io,
      const std::string& description,
      const std::vector<std::string>& references);

  /**
   * @brief Create a ColumnSpec for configuring this type as a DynamicTable
   * column.
   *
   * @param name The column name.
   * @param dataConfig Dataset configuration for the column.
   * @param description The column description attribute.
   * @return Shared pointer to the ColumnSpec for this column type.
   */
  static std::shared_ptr<DataSpec> createDataSpec(
      const std::string& name,
      const IO::ArrayDataSetConfig& dataConfig,
      const std::string& description);

  /**
   *  @brief Initialize the dataset for the VectorData object
   *
   *  This function creates a dataset using the provided configuration
   *
   * @param dataConfig The configuration for the dataset
   * @param description The description of the VectorData
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                    const std::string& description);

  /**
   * @brief Appends a cell value (scalar or vector) to the dataset.
   *
   * @param cellValue The value to append.
   * @param elementsAppended Output parameter that will be set to the number of
   * elements appended.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status appendData(const CellValue& cellValue, size_t& elementsAppended);

  DEFINE_ATTRIBUTE_FIELD(readDescription,
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
  friend class AQNWB::NWB::RegisteredType; /* base can call constructor */

protected:
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

  using VectorData::VectorData; /* inherit from immediate base */

public:
  /** \brief Factor method to create a VectorDataTyped object.
   *
   * This is required here since VectorDataTyped is a template class and
   * is not being registered with the RegisteredType class registry via
   * REGISTER_SUBCLASS.
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   * @return A shared pointer to the created NWBFile object, or nullptr if
   * creation failed.
   */
  static std::shared_ptr<VectorDataTyped> create(
      const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
  {
    return RegisteredType::create<VectorDataTyped>(path, io);
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
      const std::shared_ptr<VectorData>& data)
  {
    return VectorDataTyped<DTYPE>::create(data->getPath(), data->getIO());
  }

  using RegisteredType::getIO;
  using RegisteredType::getPath;

  // Define the data fields to expose for lazy read access
  DEFINE_DATASET_FIELD(readData, recordData, DTYPE, "", The main data)
};
}  // namespace AQNWB::NWB
