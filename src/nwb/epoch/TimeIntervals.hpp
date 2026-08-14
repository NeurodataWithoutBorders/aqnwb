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
// Includes for types that are referenced and used
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "nwb/hdmf/table/VectorIndex.hpp"
// #include "core/base/TimeSeriesReferenceVectorData.hpp"

#include "spec/core.hpp"

// TODO Need to implement this type #include "nwb/hdmf/table/VectorIndex.hpp"
// TODO Implement support for ragged arrays on read/write
// TODO Add unit tests for TimeIntervals

namespace AQNWB::NWB
{

/**
 * @brief A container for aggregating epoch data and the TimeSeries that each
 * epoch applies to.
 */
class TimeIntervals : public AQNWB::NWB::DynamicTable
{
protected:
  /**
   * @brief Constructor
   * @param path Path to the object in the file
   * @param io IO object for reading/writing
   */
  TimeIntervals(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io);

public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~TimeIntervals() override {}

  /**
   * @brief Validates the provided data specifications for the TimeIntervals
   * table.
   * @param dataSpecs The data specifications to validate.
   * @return Status::Success if the specifications are valid, otherwise
   * Status::Failure.
   */
  Status validateDataSpecs(
      const std::vector<DataSpecPtr>& dataSpecs) const override;

  /**
   * @brief Creates the default data specs for the TimeIntervals table.
   * @param timeIntervalsPath Path of the TimeIntervals table. Need to construct
   * target for VectorIndex
   * @param rowChunkSize The chunk size for the rows of the table.
   * @param addTagsColumn Add the tags column to the spec
   * @return A vector of DataSpecPtr containing the default specs.
   */
  static std::vector<DataSpecPtr> createDefaultDataSpecs(
      const std::string timeIntervalsPath,
      const SizeType rowChunkSize = 100,
      const bool addTagsColumn = false);

  /**
   * @brief Initializes the TimeIntervals table
   *
   * Initializes the TimeIntervals table by creating NWB related attributes and
   * adding required columns.
   *
   * @param description The description of the table
   * @param columnSpecs The column specifications to use for initialization.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::string& description,
                    const std::vector<DataSpecPtr>& columnSpecs = {});

  // Define read methods
  DEFINE_REGISTERED_FIELD(readStartTime,
                          VectorDataTyped<float>,
                          "start_time",
                          "Start time of epoch - in seconds.")

  DEFINE_REGISTERED_FIELD(readStopTime,
                          VectorDataTyped<float>,
                          "stop_time",
                          "Stop time of epoch - in seconds.")

  DEFINE_REGISTERED_FIELD(
      readTags,
      VectorDataTyped<std::string>,
      "tags",
      "User-defined tags that identify or categorize events.")

  DEFINE_REGISTERED_FIELD(readTagsIndex,
                          VectorIndex,
                          "tags_index",
                          "Index for tags.")

  /*
  // TODO Add support for the timeseries collumns requires support for
  TimeSeriesReferenceVectorData DEFINE_REGISTERED_FIELD( readTimeseries,
      CORE::TimeSeriesReferenceVectorData,
      "timeseries",
      "An index into a TimeSeries object.")
  */
  /*
  // TODO Add support for timeseries_index requires support for VectorIndex
  DEFINE_REGISTERED_FIELD(
      readTimeseriesIndex,
      HDMF_COMMON::VectorIndex,
      "timeseries_index",
      "Index for timeseries.")
  */

  REGISTER_SUBCLASS(TimeIntervals,
                    DynamicTable,
                    AQNWB::SPEC::CORE::namespaceName)
};

}  // namespace AQNWB::NWB
