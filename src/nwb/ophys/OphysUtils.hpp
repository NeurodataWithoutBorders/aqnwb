#pragma once

#include <memory>
#include <string>

#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "io/RecordingObjects.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/image/ImageSeries.hpp"
#include "nwb/ophys/ImagingPlane.hpp"
#include "nwb/ophys/OnePhotonSeries.hpp"
#include "nwb/ophys/TwoPhotonSeries.hpp"

/*!
 * \namespace AQNWB::NWB::Ophys
 * \brief Convenience functions to acquire optical physiology data.
 *
 * The functions are free functions (rather than methods of NWBFile) so that the
 * optical physiology module can be disabled at build time via the CMake option
 * `AQNWB_ENABLE_OPHYS`.
 */
namespace AQNWB::NWB::Ophys
{

/**
 * @brief Path of the group for the optophysiology metadata, i.e., the
 * imaging planes.
 */
inline const std::string OPTOPHYSIOLOGY_PATH = "/general/optophysiology";

/**
 * @brief Create and initialize an ImagingPlane in
 * `/general/optophysiology/<name>`.
 *
 * @param nwbfile The NWBFile to add the imaging plane to.
 * @param name The name of the imaging plane.
 * @param spec The specification of the imaging plane.
 * @return The initialized ImagingPlane or nullptr if the creation failed.
 */
std::shared_ptr<ImagingPlane> createImagingPlane(
    NWBFile& nwbfile, const std::string& name, const ImagingPlane::Spec& spec);

/**
 * @brief Create and initialize a TwoPhotonSeries in the acquisition group.
 *
 * Use getRecordingObjectIndex() of the returned series to write frames via
 * writeImageSeriesFrames, or call ImageSeries::writeFrames directly.
 *
 * @param nwbfile The NWBFile to add the series to.
 * @param name The name of the series.
 * @param spec The specification of the series. Use
 * getDefaultImageSeriesDataConfig to configure the data.
 * @return The initialized TwoPhotonSeries or nullptr if the creation failed.
 */
std::shared_ptr<TwoPhotonSeries> createTwoPhotonSeries(
    NWBFile& nwbfile,
    const std::string& name,
    const TwoPhotonSeries::Spec& spec);

/**
 * @brief Create and initialize a OnePhotonSeries in the acquisition group.
 *
 * @param nwbfile The NWBFile to add the series to.
 * @param name The name of the series.
 * @param spec The specification of the series. Use
 * getDefaultImageSeriesDataConfig to configure the data.
 * @return The initialized OnePhotonSeries or nullptr if the creation failed.
 */
std::shared_ptr<OnePhotonSeries> createOnePhotonSeries(
    NWBFile& nwbfile,
    const std::string& name,
    const OnePhotonSeries::Spec& spec);

/**
 * @brief Get the default configuration of the data of an ImageSeries.
 *
 * The data is extendable in time (shape [0, x, y(, z)]) and each chunk holds a
 * single frame (chunking [1, x, y(, z)]), which suits writing frame by frame.
 *
 * @param dataType The data type of the pixels, e.g., BaseDataType::U16.
 * @param frameShape The shape of a single frame, i.e., [x, y] or [x, y, z].
 * @return The dataset configuration.
 */
IO::ArrayDataSetConfig getDefaultImageSeriesDataConfig(
    const IO::BaseDataType& dataType, const SizeArray& frameShape);

/**
 * @brief Append frames to an ImageSeries (e.g., a TwoPhotonSeries) by its
 * recording object index.
 *
 * @param recordingObjects The recording objects of the IO.
 * @param containerInd The recording object index of the ImageSeries.
 * @param numFrames The number of frames to write.
 * @param data Pointer to the frames. See ImageSeries::writeFrames.
 * @param timestamps Pointer to the numFrames timestamps or nullptr if the
 * series uses a starting time and rate.
 * @return Status::Success if successful, otherwise Status::Failure.
 */
Types::Status writeImageSeriesFrames(
    const std::shared_ptr<IO::RecordingObjects>& recordingObjects,
    const SizeType& containerInd,
    const SizeType& numFrames,
    const void* data,
    const void* timestamps = nullptr);

}  // namespace AQNWB::NWB::Ophys
