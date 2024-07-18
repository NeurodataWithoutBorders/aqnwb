#pragma once

#include <string>

#include "BaseIO.hpp"
#include "Channel.hpp"
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
                   const BaseDataType& dataType,
                   const BaseDataType& timestampsType,
                   const Types::ChannelGroup& channelGroup,
                   const std::string& electrodesTablePath,
                   const std::string& unit = "volts",
                   const std::string& description = "no description",
                   const std::string& comments = "no comments",
                   const SizeArray& dsetSize = SizeArray{0},
                   const SizeArray& chunkSize = SizeArray{1},
                   const float& conversion = 1.0f,
                   const float& resolution = -1.0f,
                   const float& offset = 0.0f);

  /**
   * @brief Destructor
   */
  ~ElectricalSeries();

  /**
   * @brief Initializes the Electrical Series
   */
  void initialize();

  /**
   * @brief Channel group that this time series is associated with.
   */
  Types::ChannelGroup channelGroup;

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
