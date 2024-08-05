#pragma once

#include <string>

#include "BaseIO.hpp"
#include "nwb/hdmf/base/Container.hpp"

namespace AQNWB::NWB
{
/**
 * @brief General purpose time series.
 */
class TimeSeries : public Container
{
public:
  /**
   * @brief Constructor.
   * @param path The location of the TimeSeries in the file.
   * @param io A shared pointer to the IO object.
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
  TimeSeries(const std::string& path,
             std::shared_ptr<BaseIO> io,
             const BaseDataType& dataType,
             const std::string& unit,
             const std::string& description = "no description",
             const std::string& comments = "no comments",
             const SizeArray& dsetSize = SizeArray {0},
             const SizeArray& chunkSize = SizeArray {1},
             const float& conversion = 1.0f,
             const float& resolution = -1.0f,
             const float& offset = 0.0f);

  /**
   * @brief Destructor
   */
  ~TimeSeries();

  /**
   * @brief Writes a timeseries data block to the file.
   * @param dataShape The size of the data block.
   * @param positionOffset The position of the data block to write to.
   * @param data A pointer to the data block.
   * @param timestamps A pointer to the timestamps block. May be null if
   * multidimensional TimeSeries and only need to write the timestamps once but
   * write data in separate blocks.
   * @return The status of the write operation.
   */
  Status writeData(const std::vector<SizeType>& dataShape,
                   const std::vector<SizeType>& positionOffset,
                   const void* data,
                   const void* timestamps = nullptr);

  /**
   * @brief Initializes the TimeSeries by creating NWB related attributes and
   * writing the description and comment metadata.
   */
  void initialize();

  /**
   * @brief Pointer to data values.
   */
  std::unique_ptr<BaseRecordingData> data;

  /**
   * @brief Pointer to timestamp values.
   */
  std::unique_ptr<BaseRecordingData> timestamps;

  /**
   * @brief Data type of the data.
   */
  BaseDataType dataType;

  /**
   * @brief Data type of the timestamps (float64).
   */
  BaseDataType timestampsType = BaseDataType::F64;

  /**
   * @brief Base unit of measurement for working with the data. Actual stored
   * values are not necessarily stored in these units. To access the data in
   * these units, multiply ‘data’ by ‘conversion’ and add ‘offset’.
   */
  std::string unit;

  /**
   * @brief The description of the TimeSeries.
   */
  std::string description;

  /**
   * @brief Human-readable comments about the TimeSeries.
   */
  std::string comments;

  /**
   * @brief Size used in dataset creation. Can be expanded when writing if
   * needed.
   */
  SizeArray dsetSize;

  /**
   * @brief Chunking size used in dataset creation.
   */
  SizeArray chunkSize;

  /**
   * @brief Scalar to multiply each element in data to convert it to the
   * specified ‘unit’.
   */
  float conversion;

  /**
   * @brief Smallest meaningful difference between values in data, stored in the
   * specified by unit.
   */
  float resolution;

  /**
   * @brief Scalar to add to the data after scaling by ‘conversion’ to finalize
   * its coercion to the specified ‘unit’.
   */
  float offset;

  /**
   * @brief The starting time of the TimeSeries.
   */
  float startingTime = 0.0;

private:
  /**
   * @brief The neurodataType of the TimeSeries.
   */
  std::string neurodataType = "TimeSeries";
};
}  // namespace AQNWB::NWB
