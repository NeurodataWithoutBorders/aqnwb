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

namespace CORE
{

/**
 * @brief A 1-dimensional VectorData that stores timestamps in seconds from the
 * session start time. Timestamp are not required to be sorted in time.
 */
class TimestampVectorData : public AQNWB::NWB::VectorData
{
public:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  TimestampVectorData(const std::string& path,
                      std::shared_ptr<AQNWB::IO::BaseIO> io);

  /**
   * @brief Virtual destructor.
   */
  virtual ~TimestampVectorData() override {}

  // TODO: Update the initialize method as appropriate.
  /**
   * @brief Initialize the object
   */
  Status initialize(float resolution,
                    const std::string& description,
                    const AQNWB::IO::ArrayDataSetConfig& data);

  // Define read methods
  DEFINE_ATTRIBUTE_FIELD(
      readUnit,
      std::string,
      "unit",
      "The unit of measurement for the timestamps - fixed to seconds.")

  DEFINE_ATTRIBUTE_FIELD(
      readResolution,
      float,
      "resolution",
      "The temporal resolution of the timestamps - in seconds. This is "
      "typically the sampling period (1 / sampling_rate) - also known as the "
      "clock period - of the data acquisition system from which the timestamps "
      "were recorded or derived.")

  DEFINE_DATASET_FIELD(
      readData,
      recordData,
      float,
      "",
      "A 1-dimensional VectorData that stores timestamps in seconds from the "
      "session start time. Timestamp are not required to be sorted in time.")

  REGISTER_SUBCLASS(TimestampVectorData,
                    VectorData,
                    AQNWB::SPEC::CORE::namespaceName)
};

}  // namespace CORE
