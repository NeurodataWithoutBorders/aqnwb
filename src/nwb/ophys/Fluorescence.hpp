#pragma once

#include <memory>
#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/NWBDataInterface.hpp"
#include "nwb/ophys/RoiResponseSeries.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Fluorescence information about a region of interest (ROI).
 *
 * A container for RoiResponseSeries objects, which are created via
 * createRoiResponseSeries. By convention, the object is named "Fluorescence"
 * and stored in the "ophys" ProcessingModule.
 */
class Fluorescence : public NWBDataInterface
{
public:
  // Register the Fluorescence as a subclass of NWBDataInterface
  REGISTER_SUBCLASS(Fluorescence,
                    NWBDataInterface,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a Fluorescence.
   *
   * Fluorescence does not define any additional fields.
   */
  struct Spec : public NWBDataInterface::Spec
  {
  };

  /**
   * @brief Destructor
   */
  ~Fluorescence() override;

  /**
   * @brief Initializes the Fluorescence from a spec.
   * @param spec The specification of the Fluorescence.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_UNNAMED_REGISTERED_FIELD(readRoiResponseSeries,
                                  createRoiResponseSeries,
                                  RoiResponseSeries,
                                  "",
                                  RoiResponseSeries objects containing fluorescence data for a ROI)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the Fluorescence in the file.
   * @param io A shared pointer to the IO object.
   */
  Fluorescence(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
