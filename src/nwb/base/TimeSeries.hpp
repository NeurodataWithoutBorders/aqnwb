#pragma once

#include <cstdint>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/hdmf/base/Container.hpp"

namespace AQNWB::NWB
{
/**
 * @brief General purpose time series.
 */
class TimeSeries : public Container
{
public:
  // Register the TimeSeries as a subclass of Container
  REGISTER_SUBCLASS(TimeSeries, "core")

  /**
   * @brief Constructor.
   * @param path The location of the TimeSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  TimeSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor
   */
  ~TimeSeries();

  /**
   * @brief Writes a timeseries data block to the file.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param dataInput A pointer to the data block.
   * @param timestampsInput A pointer to the timestamps block. May be null if
   * multidimensional TimeSeries and only need to write the timestamps once but
   * write data in separate blocks.
   * @return The status of the write operation.
   */
  Status writeData(const std::vector<SizeType>& dataShape,
                   const std::vector<SizeType>& positionOffset,
                   const void* dataInput,
                   const void* timestampsInput = nullptr);

  /**
   * @brief Initializes the TimeSeries by creating NWB related attributes and
   * writing the description and comment metadata.
   *
   * @param dataType The data type to use for storing the recorded signal
   * @param unit Unit for the electrical signal. Must be "volts"
   * @param description The description of the TimeSeries.
   * @param comments Human-readable comments about the TimeSeries
   * @param dsetSize Initial size of the main dataset
   * @param chunkSize Chunk size to use
   * @param conversion Scalar to multiply each element in data to convert it to
   *                   the specified ‘unit’
   * @param resolution Smallest meaningful difference between values in data,
   *                   stored in the specified by unit
   * @param offset Scalar to add to the data after scaling by ‘conversion’ to
   *               finalize its coercion to the specified ‘unit'
   */
  void initialize(const IO::BaseDataType& dataType,
                  const std::string& unit,
                  const std::string& description = "no description",
                  const std::string& comments = "no comments",
                  const SizeArray& dsetSize = SizeArray {0},
                  const SizeArray& chunkSize = SizeArray {1},
                  const float& conversion = 1.0f,
                  const float& resolution = -1.0f,
                  const float& offset = 0.0f);

  /**
   * @brief Pointer to data values.
   */
  std::unique_ptr<IO::BaseRecordingData> data;

  /**
   * @brief Pointer to timestamp values.
   */
  std::unique_ptr<IO::BaseRecordingData> timestamps;

  /**
   * @brief Data type of the data.
   */
  IO::BaseDataType dataType;

  /**
   * @brief Data type of the timestamps (float64).
   */
  IO::BaseDataType timestampsType = IO::BaseDataType::F64;

  // Define the data fields to expose for lazy read access
  DEFINE_FIELD(readDescription,
               AttributeField,
               std::string,
               "description",
               Description of the series)

  DEFINE_FIELD(readComments,
               AttributeField,
               std::string,
               "comments",
               Human - readable comments about the TimeSeries)

  DEFINE_FIELD(readData, DatasetField, std::any, "data", The main data)

  DEFINE_FIELD(readDataConversion,
               AttributeField,
               float,
               "data/conversion",
               Scalar to multiply each element in data to convert it to the
                   specified unit)

  DEFINE_FIELD(readDataOffset,
               AttributeField,
               float,
               "data/offset",
               Scalar to add to the data after scaling by conversion to finalize
                   its coercion to the specified unit)

  DEFINE_FIELD(readDataResolution,
               AttributeField,
               float,
               "data/resolution",
               Smallest meaningful difference between values in data)

  DEFINE_FIELD(readDataUnit,
               AttributeField,
               std::string,
               "data/unit",
               Base unit of measurement for working with the data)

  DEFINE_FIELD(readDataContinuity,
               AttributeField,
               std::string,
               "data/continuity",
               Continuity of the data)

  DEFINE_FIELD(readStartingTime,
               DatasetField,
               double,
               "starting_time",
               Timestamp of the first sample in seconds)

  DEFINE_FIELD(readStartingTimeRate,
               AttributeField,
               float,
               "starting_time/rate",
               Sampling rate in Hz)

  DEFINE_FIELD(readStartingTimeUnit,
               AttributeField,
               std::string,
               "starting_time/unit",
               Unit of measurement for time fixed to seconds)

  DEFINE_FIELD(readTimestamps,
               DatasetField,
               double,
               "timestamps",
               Timestamps offset in seconds relative to the master time for samples stored in data)

  DEFINE_FIELD(readTimestampsInterval,
               AttributeField,
               int,
               "timestamps/interval",
               Interval value is 1)

  DEFINE_FIELD(readTimestampsUnit,
               AttributeField,
               std::string,
               "timestamps/unit",
               Unit of measurement for timestamps fixed to seconds)

  DEFINE_FIELD(readControl,
               DatasetField,
               uint8_t,
               "control",
               Numerical labels that apply to each time point in data)

  DEFINE_FIELD(readControlDescription,
               DatasetField,
               std::string,
               "control_description",
               Description of each control value)
};
}  // namespace AQNWB::NWB
