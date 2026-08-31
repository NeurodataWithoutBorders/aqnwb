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
// Include for the namespace schema header
#include "spec/core.hpp"

namespace AQNWB::NWB
{

/**
 * @brief A 1-D VectorData that stores durations in seconds.
 */
class DurationVectorData : public AQNWB::NWB::VectorData
{
public:
  struct DataSpec : public Data::DataSpec<DurationVectorData>
  {
    DataSpec(const std::string& datasetName,
             const AQNWB::IO::ArrayDataSetConfig& dataConfig,
             const std::string& columnDescription,
             std::optional<float> columnResolution = std::nullopt)
        : Data::DataSpec<DurationVectorData>(datasetName, dataConfig)
        , description(columnDescription)
        , resolution(columnResolution)
    {
    }

    std::string description;
    std::optional<float> resolution;

    Status initialize(Data& data) const override;
  };

  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  DurationVectorData(const std::string& path,
                     std::shared_ptr<AQNWB::IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~DurationVectorData() override {}

  /**
   * @brief Initialize the object
   * @param data The configuration for the dataset
   * @param description Description of the dataset
   * @param resolution The temporal resolution of the durations - in seconds.
   * This is typically the sampling period (1 / sampling_rate), also known as
   * the clock period, of the data acquisition system from which the durations
   * were recorded or derived.
   */
  Status initialize(const AQNWB::IO::ArrayDataSetConfig& data,
                    const std::string& description,
                    std::optional<float> resolution = std::nullopt);

  static std::shared_ptr<DataSpec> createDataSpec(
      const std::string& name,
      const AQNWB::IO::ArrayDataSetConfig& dataConfig,
      const std::string& description,
      std::optional<float> resolution = std::nullopt);

  // Define read methods
  DEFINE_ATTRIBUTE_FIELD(
      readUnit,
      std::string,
      "unit",
      "The unit of measurement for the durations - fixed to seconds.")

  DEFINE_ATTRIBUTE_FIELD(
      readResolution,
      float,
      "resolution",
      "The temporal resolution of the durations - in seconds. This is "
      "typically the sampling period (1 / sampling_rate) - also known as the "
      "clock period - of the data acquisition system from which the durations "
      "were recorded or derived.")

  DEFINE_DATASET_FIELD(readData,
                       recordData,
                       float,
                       "",
                       "A 1-D VectorData that stores durations in seconds.")

  REGISTER_SUBCLASS(DurationVectorData,
                    VectorData,
                    AQNWB::SPEC::CORE::namespaceName)
};

}  // namespace AQNWB::NWB
