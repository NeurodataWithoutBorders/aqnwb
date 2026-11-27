#pragma once

#include <memory>
#include <string>

#include "Types.hpp"
#include "io/BaseIO.hpp"
#include "io/RecordingObjects.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/ogen/OptogeneticSeries.hpp"
#include "nwb/ogen/OptogeneticStimulusSite.hpp"

/*!
 * \namespace AQNWB::NWB::Ogen
 * \brief Convenience functions to record optogenetic stimulation.
 *
 * The functions are free functions (rather than methods of NWBFile) so that the
 * optogenetics module can be disabled at build time via the CMake option
 * `AQNWB_ENABLE_OGEN`.
 */
namespace AQNWB::NWB::Ogen
{

/**
 * @brief Path of the group for the optogenetics metadata, i.e., the
 * stimulation sites.
 */
inline const std::string OPTOGENETICS_PATH = "/general/optogenetics";

/**
 * @brief Create and initialize an OptogeneticStimulusSite in
 * `/general/optogenetics/<name>`.
 *
 * @param nwbfile The NWBFile to add the stimulation site to.
 * @param name The name of the stimulation site.
 * @param spec The specification of the stimulation site.
 * @return The initialized OptogeneticStimulusSite or nullptr if the creation
 * failed.
 */
std::shared_ptr<OptogeneticStimulusSite> createOptogeneticStimulusSite(
    const NWBFile& nwbfile,
    const std::string& name,
    const OptogeneticStimulusSite::Spec& spec);

/**
 * @brief Create and initialize an OptogeneticSeries in the acquisition group.
 *
 * Use getRecordingObjectIndex() of the returned series to write samples via
 * IO::appendTimeSeriesData, or call OptogeneticSeries::writeSamples directly.
 *
 * @param nwbfile The NWBFile to add the series to.
 * @param name The name of the series.
 * @param spec The specification of the series.
 * @return The initialized OptogeneticSeries or nullptr if the creation failed.
 */
std::shared_ptr<OptogeneticSeries> createOptogeneticSeries(
    const NWBFile& nwbfile,
    const std::string& name,
    const OptogeneticSeries::Spec& spec);

}  // namespace AQNWB::NWB::Ogen
