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
   * Used to describe the continuity of the data in a time series.
   */
  enum ContinuityType
  {
    /** Data is recorded from a continuous process */
    Continuous = 0,
    /** Data describes instantious evnets in time, e.g., moments of licking. */
    Instantaneous = 1,
    /** Data describes a step-function, e.g., image presented to a subject
     * that remain until the next timpoint. */
    Step = 2,
    /** The continuity of the data is not defined. */
    Undefined = -1
  };

  /**
   * @brief String names corresponding to the ContinuityType enum.
   *
   * These are the values to optionally describe the continuity of the data. Can
   * be “continuous”, “instantaneous”, or “step”. For example, a voltage trace
   * would be “continuous”, because samples are recorded from a continuous
   * process. An array of lick times would be “instantaneous”, because the data
   * represents distinct moments in time. Times of image presentations would be
   * “step” because the picture remains the same until the next timepoint. This
   * field is optional, but is useful in providing information about the
   * underlying data. It may inform the way this data is interpreted, the way it
   * is visualized, and what analysis methods are applicable.
   */
  static std::map<ContinuityType, std::string> ContinuityTypeNames;

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
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeData(const std::vector<SizeType>& dataShape,
                   const std::vector<SizeType>& positionOffset,
                   const void* dataInput,
                   const void* timestampsInput = nullptr,
                   const void* controlInput = nullptr);

  /**
   * @brief Initializes the TimeSeries by creating NWB related attributes and
   * writing the description and comment metadata.
   *
   * @param dataConfig Configuration for the dataset including data type, shape
   * and chunking
   * @param unit Unit for the electrical signal. Must be "volts"
   * @param description The description of the TimeSeries.
   * @param comments Human-readable comments about the TimeSeries
   * @param conversion Scalar to multiply each element in data to convert it to
   *                   the specified ‘unit’
   * @param resolution Smallest meaningful difference between values in data,
   *                   stored in the specified by unit
   * @param offset Scalar to add to the data after scaling by ‘conversion’ to
   *               finalize its coercion to the specified ‘unit'
   * @param continuity Continuity of the data
   * @param startingTime Timestamp of the first sample in seconds. Used when
   * timestamps are uniformly spaced, such that the timestamp of the first
   * sample can be specified and all subsequent ones calculated from the
   * sampling rate attribute. Set to -1.0 to indicate that recorded timestamps
   * should be used such that no starting_time dataset will be created. If set
   * to a value >= 0 then no timestamps dataset will be created.
   * @param startingTimeRate Sampling rate in Hz. Used only when timestamps are
   * uniformly spaced via startingTime.
   * @param controlDescription Description of each control value to be used
   * during the recording. If a non-empty vector is provided then control and
   * control_description data will be created (otherwise they will be nullptr).
   * We can update the control_description values later if needed via the
   * TimeSeries.control_description->writeStringDataBlock() method.
   */
  void initialize(const IO::ArrayDataSetConfig& dataConfig,
                  const std::string& unit,
                  const std::string& description = "no description",
                  const std::string& comments = "no comments",
                  const float& conversion = 1.0f,
                  const float& resolution = -1.0f,
                  const float& offset = 0.0f,
                  const ContinuityType& continuity = ContinuityType::Undefined,
                  const double& startingTime = -1.0,
                  const float& startingTimeRate = 1.0f,
                  const std::vector<std::string>& controlDescription = {});

  /**
   * @brief Pointer to data values.
   */
  std::unique_ptr<IO::BaseRecordingData> data;

  /**
   * @brief Pointer to timestamp values. This may be a nullptr if starting_time
   * is used.
   */
  std::unique_ptr<IO::BaseRecordingData> timestamps;

  /**
   * @brief Pointer to starting_time values. This may be a nullptr if timestamps
   * are used.
   *
   * The starting_time is usually set when calling initialize(), but is exposed
   * here to allow overwriting of the initial starting_time value in case that
   * the correct starting_time is not known until later.
   */
  std::unique_ptr<IO::BaseRecordingData> starting_time;

  /**
   * @brief Pointer to control values. This may be a nullptr if useControl is
   * false.
   */
  std::unique_ptr<IO::BaseRecordingData> control;

  /**
   * @brief Pointer to control_description values. This may be a nullptr if
   * useControl is false.
   */
  std::unique_ptr<IO::BaseRecordingData> control_description;

  /**
   * @brief Data type of the data.
   */
  IO::BaseDataType m_dataType;

  /**
   * @brief Data type of the timestamps (float64).
   */
  IO::BaseDataType timestampsType = IO::BaseDataType::F64;

  /**
   * @brief Data type of the control (uint8).
   */
  IO::BaseDataType controlType = IO::BaseDataType::U8;

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

private:
  /**
   * @brief Convenience function for creating timestamp related attributes.
   * @param path The location of the object in the file.
   * @return The status of the operation.
   */
  Status createTimestampsAttributes(const std::string& path);

  /**
   * @brief Convenience function for creating data related attributes.
   * @param path The location of the object in the file.
   * @param conversion Scalar to multiply each element in data to convert it to
   * the specified ‘unit’.
   * @param resolution Smallest meaningful difference between values in data.
   * @param offset Scalar to add to the data after scaling by ‘conversion’ to
   *               finalize its coercion to the specified ‘unit’.
   * @param unit Base unit of measurement for working with the data.
   * @param continuity Continuity of the data
   * @return The status of the operation.
   */
  Status createDataAttributes(const std::string& path,
                              const float& conversion,
                              const float& resolution,
                              const float& offset,
                              const std::string& unit,
                              const ContinuityType& continuity);
};
}  // namespace AQNWB::NWB
