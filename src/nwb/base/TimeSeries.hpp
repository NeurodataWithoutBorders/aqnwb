#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBDataInterface.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief General purpose time series.
 */
class TimeSeries : public NWBDataInterface
{
public:
  // Register the TimeSeries as a subclass of NWBDataInterface
  REGISTER_SUBCLASS(TimeSeries,
                    NWBDataInterface,
                    AQNWB::SPEC::CORE::namespaceName)

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
   * @brief Specification for initializing a TimeSeries.
   *
   * Example:
   * @code
   * IO::ArrayDataSetConfig config(IO::BaseDataType::F32, {0, 4}, {1024, 4});
   * auto spec = TimeSeries::Spec()
   *                 .withData(config)
   *                 .withUnit("volts")
   *                 .withDescription("Filtered LFP signal");
   * Status status = timeSeries->initialize(spec);
   * @endcode
   */
  struct Spec : public NWBDataInterface::Spec
  {
    REGISTER_SPEC(TimeSeries, NWBDataInterface::Spec)

    /// @brief Shared pointer to a dataset configuration (const)
    using DataConfigPtr = std::shared_ptr<const IO::BaseArrayDataSetConfig>;

    /**
     * @brief Configuration for the data dataset including data type and shape
     * and chunking. May also be a LinkArrayDataSetConfig to link to the data of
     * another TimeSeries. Required.
     */
    DEFINE_SPEC_FIELD(Spec, DataConfigPtr, data, withData, nullptr)

    /**
     * @brief Set the configuration of the data dataset.
     *
     * Stores a copy of the given configuration via
     * IO::BaseArrayDataSetConfig::clone, i.e., backend-specific settings (e.g.,
     * the filters of an HDF5ArrayDataSetConfig) are preserved also if the
     * configuration is passed via a base class reference.
     *
     * @param config The configuration of the data dataset, e.g., an
     * IO::ArrayDataSetConfig or IO::LinkArrayDataSetConfig.
     * @return Reference to this spec to allow chaining.
     */
    Spec& withData(const IO::BaseArrayDataSetConfig& config)
    {
      data = config.clone();
      if (data == nullptr) {
        std::cerr << "TimeSeries::Spec::withData: The configuration does not "
                     "support clone()."
                  << std::endl;
      }
      return *this;
    }

    /**
     * @brief Base unit of measurement for working with the data. Required.
     */
    DEFINE_SPEC_FIELD(Spec, std::string, unit, withUnit, "")
    /**
     * @brief Description of the time series. The description attribute is not
     * written if set to an empty string.
     */
    DEFINE_SPEC_FIELD(
        Spec, std::string, description, withDescription, "no description")
    /**
     * @brief Human-readable comments about the TimeSeries.
     */
    DEFINE_SPEC_FIELD(Spec, std::string, comments, withComments, "no comments")
    /**
     * @brief Scalar to multiply each element in data to convert it to the
     * specified unit.
     */
    DEFINE_SPEC_FIELD(Spec, float, conversion, withConversion, 1.0f)
    /**
     * @brief Smallest meaningful difference between values in data stored in
     * the specified unit. -1.0 means unknown.
     */
    DEFINE_SPEC_FIELD(Spec, float, resolution, withResolution, -1.0f)
    /**
     * @brief Scalar to add to the data after scaling by conversion to finalize
     * its coercion to the specified unit.
     */
    DEFINE_SPEC_FIELD(Spec, float, offset, withOffset, 0.0f)
    /**
     * @brief Continuity of the data. The continuity attribute is not written if
     * Undefined.
     */
    DEFINE_SPEC_FIELD(Spec,
                      ContinuityType,
                      continuity,
                      withContinuity,
                      ContinuityType::Undefined)
    /**
     * @brief Timestamp of the first sample in seconds. If set then the series
     * uses a regular sampling rate (see startingTimeRate) and no timestamps
     * dataset is created. If not set then timestamps are recorded explicitly.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::optional<double>,
                      startingTime,
                      withStartingTime,
                      std::nullopt)
    /**
     * @brief Sampling rate in Hz. Only used if startingTime is set.
     */
    DEFINE_SPEC_FIELD(Spec, float, startingTimeRate, withStartingTimeRate, 1.0f)
    /**
     * @brief Description of each control value. If not empty then the control
     * and control_description datasets are created.
     */
    DEFINE_SPEC_FIELD(Spec,
                      std::vector<std::string>,
                      controlDescription,
                      withControlDescription,
                      {})

    /**
     * @brief Validate the spec.
     *
     * Checks that data is set, that unit is not empty, and that the
     * startingTime and startingTimeRate are valid if startingTime is set.
     *
     * @return Status::Success if the spec is valid, otherwise Status::Failure.
     */
    Status validate() const override;
  };

  /**
   * @brief Destructor
   */
  ~TimeSeries() override;

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
  Status writeData(const SizeArray& dataShape,
                   const SizeArray& positionOffset,
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
   *                   stored in the specified unit
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
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  [[deprecated("Use TimeSeries::initialize(const TimeSeries::Spec&) instead")]]
  Status initialize(
      const IO::BaseArrayDataSetConfig& dataConfig,
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
   * @brief Initializes the TimeSeries from a spec.
   *
   * Creates the data dataset and its attributes (conversion, resolution,
   * offset, unit, and continuity), the description and comments attributes,
   * either the timestamps or the starting_time dataset, and, if
   * controlDescription is not empty, the control and control_description
   * datasets.
   *
   * @param spec The specification of the TimeSeries.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  /**
   * @brief Append samples to the data (and timestamps and control) of the
   * series.
   *
   * The samples are appended along the first dimension of the data. The
   * offset and the shape of a sample are taken from the current extent of the
   * data in the file, such that appending also works after reopening a file.
   *
   * @param numSamples The number of samples (along the first dimension) to
   * write.
   * @param dataInput Pointer to the samples, i.e., a contiguous array of shape
   * [numSamples, ...] (the remaining dimensions of the data) in row-major
   * order with the data type of the data.
   * @param timestampsInput Pointer to the numSamples timestamps (float64).
   * Required if the series uses timestamps and must be nullptr if the series
   * uses startingTime and startingTimeRate.
   * @param controlInput Pointer to the numSamples control values (uint8).
   * Optional. Requires that the series has a control dataset.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status appendSamples(const SizeType& numSamples,
                       const void* dataInput,
                       const void* timestampsInput = nullptr,
                       const void* controlInput = nullptr);

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

protected:
  /**
   * @brief True if the data of the series is a link to the data of another
   * series (set by initialize). appendSamples refuses to write to links.
   */
  bool m_dataIsLink = false;

  /**
   * @brief True if m_dataType has been set (by initialize or appendSamples).
   */
  bool m_dataTypeInitialized = false;

public:
  // Define the data fields to expose for lazy read access
  DEFINE_ATTRIBUTE_FIELD(readDescription,
                         std::string,
                         "description",
                         Description of the series)

  DEFINE_ATTRIBUTE_FIELD(readComments,
                         std::string,
                         "comments",
                         Human - readable comments about the TimeSeries)

  DEFINE_DATASET_FIELD(readData, recordData, std::any, "data", The main data)

  DEFINE_ATTRIBUTE_FIELD(readDataConversion,
                         float,
                         "data/conversion",
                         Scalar to multiply each element in data to convert it
                             to the specified unit)

  DEFINE_ATTRIBUTE_FIELD(readDataOffset,
                         float,
                         "data/offset",
                         Scalar to add to the data after scaling by conversion
                             to finalize its coercion to the specified unit)

  DEFINE_ATTRIBUTE_FIELD(readDataResolution,
                         float,
                         "data/resolution",
                         Smallest meaningful difference between values in data)

  DEFINE_ATTRIBUTE_FIELD(readDataUnit,
                        std::string,
                        "data/unit",
                        Base unit of measurement for working with the data)

  DEFINE_ATTRIBUTE_FIELD(readDataContinuity,
                         std::string,
                         "data/continuity",
                         Continuity of the data)

  DEFINE_DATASET_FIELD(readStartingTime,
                       recordStartingTime,
                       double,
                       "starting_time",
                       Timestamp of the first sample in seconds)

  DEFINE_ATTRIBUTE_FIELD(readStartingTimeRate,
                         float,
                         "starting_time/rate",
                         Sampling rate in Hz)

  DEFINE_ATTRIBUTE_FIELD(readStartingTimeUnit,
                         std::string,
                         "starting_time/unit",
                         Unit of measurement for time fixed to seconds)

  DEFINE_DATASET_FIELD(readTimestamps,
                       recordTimestamps,
                       double,
                       "timestamps",
                       Timestamps offset in seconds relative to the master time for samples stored in data)

  DEFINE_ATTRIBUTE_FIELD(readTimestampsInterval,
                         int,
                         "timestamps/interval",
                         Interval value is 1)

  DEFINE_ATTRIBUTE_FIELD(readTimestampsUnit,
                         std::string,
                         "timestamps/unit",
                         Unit of measurement for timestamps fixed to seconds)

  DEFINE_DATASET_FIELD(readControl,
                       recordControl,
                       uint8_t,
                       "control",
                       Numerical labels that apply to each time point in data)

  DEFINE_DATASET_FIELD(readControlDescription,
                       recordControlDescription,
                       std::string,
                       "control_description",
                       Description of each control value)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the TimeSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  TimeSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

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
