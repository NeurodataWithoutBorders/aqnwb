#pragma once

#include <string>

#include "BaseIO.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{
/**
 * @brief General purpose time series.
 */
class ElectricalSeries : public TimeSeries
{
public:
  /**
   * @brief Constructor.
   * @param path The location of the ElectricalSeries in the file.
   * @param io A shared pointer to the IO object.
   * @param description The description of the TimeSeries.
   * @param electrodesTablePath Location of the ElectrodesTable this time series
   * was generated from
   */
  ElectricalSeries(const std::string& path,
                   std::shared_ptr<BaseIO> io,
                   const std::string& description,
                   const std::string& electrodesTablePath);

  /**
   * @brief Destructor
   */
  ~ElectricalSeries();

  /**
   * @brief Initializes the Electrical Series
   */
  void initialize();

  /**
   * @brief Path to the electrodes table this time series references
   */
  std::string electrodesTablePath;

  /**
   * @brief Pointer to channel-specific conversion factor dataset.
   */
  std::unique_ptr<BaseRecordingData> channelConversion;

  /**
   * @brief Pointer to electrodes dataset.
   */
  std::unique_ptr<BaseRecordingData> electrodesDataset;

private:
  /**
   * @brief The neurodataType of the TimeSeries.
   */
  std::string neurodataType = "ElectricalSeries";
};
}  // namespace AQNWB::NWB
