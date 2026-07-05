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
#include "nwb/hdmf/table/DynamicTable.hpp"
// Forward declarations for referenced types
namespace AQNWB::NWB
{
class ElementIdentifiers;
class VectorData;
}  // namespace AQNWB::NWB
// Include for the namespace schema header
#include "spec/hdmf_common.hpp"

namespace AQNWB::NWB
{

/**
 * @brief A table to store information about the meanings of values in a linked
 * VectorData object. All possible values of the linked VectorData object should
 * be present in the 'value' column of this table, even if the value is not
 * observed in the data. Additional columns may be added to store additional
 * metadata about each value. The name of the MeaningsTable should correspond to
 * the name of the linked VectorData object with a "_meanings" suffix. e.g., if
 * the linked VectorData object is named "stimulus_type", the corresponding
 * MeaningsTable should be named "stimulus_type_meanings".
 */
class MeaningsTable : public AQNWB::NWB::DynamicTable
{
public:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  MeaningsTable(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~MeaningsTable() override {}

  /**
   * @brief Creates the default data specs for the MeaningsTable.
   * @param valueDataType The data type of the 'value' column in the
   * MeaningsTable.
   * @param rowChunkSize The chunk size for the rows of the table.
   * @return A vector of DataSpecPtr containing the default specs.
   */
  static std::vector<DataSpecPtr> createDefaultDataSpecs(
      const AQNWB::IO::BaseDataType& valueDataType,
      const SizeType rowChunkSize = 100);

  /**
   * @brief Initialize the object
   * @param targetVectorData The VectorData object that this MeaningsTable is
   * associated with.
   * @param valueDataType The data type of the 'value' column in the
   * MeaningsTable.
   * @param description Description of the table (optional)
   * @param columnSpecs The column specifications to use for initialization.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const VectorData& targetVectorData,
                    const AQNWB::IO::BaseDataType& valueDataType,
                    const std::string& description =
                        "A table to store information about the meanings of "
                        "values in a linked VectorData object.",
                    const std::vector<DataSpecPtr>& columnSpecs = {});

  DEFINE_REGISTERED_FIELD(readValueColumn,
                          AQNWB::NWB::VectorData,
                          "value",
                          "The value of a row in the linked VectorData object.")

  DEFINE_REGISTERED_FIELD(
      readMeaningColumn,
      AQNWB::NWB::VectorDataTyped<std::string>,
      "meaning",
      "The meaning of the value in the linked VectorData object.")

  DEFINE_REGISTERED_FIELD(
      readTarget,
      AQNWB::NWB::VectorData,
      "target",
      "The VectorData object for which this table provides meanings.")

  REGISTER_SUBCLASS(MeaningsTable,
                    DynamicTable,
                    AQNWB::SPEC::HDMF_COMMON::namespaceName)
};

}  // namespace AQNWB::NWB
