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
   * @param description The description of the TimeSeries.
   * @param comments Human-readable comments about the TimeSeries
   */
  TimeSeries(const std::string& path,
             std::shared_ptr<BaseIO> io,
             const std::string& description = "no description",
             const std::string& comments = "no comments");

  /**
   * @brief Destructor
   */
  ~TimeSeries();

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

private:
  /**
   * @brief The description of the TimeSeries.
   */
  std::string description;

  /**
   * @brief Human-readable comments about the TimeSeries.
   */
  std::string comments;

  /**
   * @brief The starting time of the TimeSeries.
   */
  float startingTime = 0.0;

  /**
   * @brief The neurodataType of the TimeSeries.
   */
  std::string neurodataType = "TimeSeries";
};
}  // namespace AQNWB::NWB
