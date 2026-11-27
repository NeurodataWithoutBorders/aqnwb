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
 * @brief dF/F information about a region of interest (ROI).
 *
 * A container for RoiResponseSeries objects, which are created via
 * createRoiResponseSeries. By convention, the object is named "DfOverF" and
 * stored in the "ophys" ProcessingModule.
 */
class DfOverF : public NWBDataInterface
{
public:
  // Register the DfOverF as a subclass of NWBDataInterface
  REGISTER_SUBCLASS(DfOverF, NWBDataInterface, AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a DfOverF.
   *
   * DfOverF does not define any additional fields.
   */
  struct Spec : public NWBDataInterface::Spec
  {
  };

  /**
   * @brief Destructor
   */
  ~DfOverF() override;

  /**
   * @brief Initializes the DfOverF from a spec.
   * @param spec The specification of the DfOverF.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_UNNAMED_REGISTERED_FIELD(readRoiResponseSeries,
                                  createRoiResponseSeries,
                                  RoiResponseSeries,
                                  "",
                                  RoiResponseSeries objects containing dF/F for a ROI)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the DfOverF in the file.
   * @param io A shared pointer to the IO object.
   */
  DfOverF(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
