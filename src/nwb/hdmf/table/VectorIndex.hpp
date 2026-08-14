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
#include "nwb/hdmf/table/VectorData.hpp"
// Includes for types that are referenced and used
#include "nwb/hdmf/base/Data.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
// Include for the namespace schema header
#include "spec/hdmf_common.hpp"

// TODO Add ReadDataWrapper that supports ragged array read
// TODO Update DynamicTable::addRow and DynamicTable::addRows to support ragged
// array cells with VectorIndex
// TODO Update VectorData code to move implementation of funcitons into the cpp
// file
// TODO in the PR that adds Subject double check to make sure the constructor
// for Subject is protected
// TODO Add unit tests for VectorIndex

namespace AQNWB::NWB
{

/**
 * @brief Used with VectorData to encode a ragged array. An array of indices
 * into the first dimension of the target VectorData, and forming a map between
 * the rows of a DynamicTable and the indices of the VectorData. The name of the
 * VectorIndex is expected to be the name of the target VectorData object
 * followed by "_index".
 */
class VectorIndex : public AQNWB::NWB::VectorData
{
protected:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  VectorIndex(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io);

public:
  /**
   * @brief Runtime configuration for creating and initializing a VectorIndex
   * column.
   */
  struct DataSpec : public Data::DataSpec<VectorIndex>
  {
    DataSpec(const std::string& datasetName,
             const IO::ArrayDataSetConfig& dataConfig,
             const std::string& columnDescription,
             const VectorData& columnTarget)
        : Data::DataSpec<VectorIndex>(datasetName, dataConfig)
        , description(columnDescription)
        , target(columnTarget)
    {
    }

    virtual ~DataSpec() = default;

    std::string description;
    const VectorData& target;

    Status initialize(Data& data) const override
    {
      auto* vectorIndex = dynamic_cast<VectorIndex*>(&data);
      if (!vectorIndex) {
        std::cerr << "VectorIndex::DataSpec::initialize received incompatible "
                     "VectorIndex object"
                  << std::endl;
        return Status::Failure;
      }
      return vectorIndex->initialize(
          static_cast<const IO::ArrayDataSetConfig&>(*this),
          description,
          target);
    }
  };

  /**
   * @brief Virtual destructor.
   */
  virtual ~VectorIndex() override {}

  /**
   *  @brief Initialize the dataset for the VectorIndex object
   *
   *  This function creates a dataset using the provided configuration
   *
   * @param dataConfig The configuration for the dataset. Must use a uint data
   * type.
   * @param description The description of the VectorData
   * @throw std::invalid_argument if the provided dataSpecs are invalid.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                    const std::string& description,
                    const VectorData& target);

  // Define read methods
  DEFINE_REFERENCED_REGISTERED_FIELD(
      readTarget,
      VectorData,
      "target",
      "Reference to the target dataset that this index applies to.")

  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         "Description of what these vectors represent.")

  DEFINE_DATASET_FIELD(
      readData,
      recordData,
      uint32_t,
      "data",
      "Used with VectorData to encode a ragged array. An array of indices into "
      "the first dimension of the target VectorData - and forming a map "
      "between the rows of a DynamicTable and the indices of the VectorData. "
      "The name of the VectorIndex is expected to be the name of the target "
      "VectorData object followed by _index.")

  REGISTER_SUBCLASS(VectorIndex,
                    VectorData,
                    AQNWB::SPEC::HDMF_COMMON::namespaceName)
};

}  // namespace AQNWB::NWB
