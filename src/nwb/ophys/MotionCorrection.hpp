#pragma once

#include <memory>
#include <string>

#include "io/BaseIO.hpp"
#include "nwb/base/NWBDataInterface.hpp"
#include "nwb/ophys/CorrectedImageStack.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief An image stack where all frames are shifted (registered) to a
 * common coordinate system, to account for movement and drift between frames.
 *
 * A container for CorrectedImageStack objects, which are created via
 * createCorrectedImageStack. By convention, the object is named
 * "MotionCorrection" and stored in the "ophys" ProcessingModule.
 */
class MotionCorrection : public NWBDataInterface
{
public:
  // Register the MotionCorrection as a subclass of NWBDataInterface
  REGISTER_SUBCLASS(MotionCorrection,
                    NWBDataInterface,
                    AQNWB::SPEC::CORE::namespaceName)

  /**
   * @brief Specification for initializing a MotionCorrection.
   *
   * MotionCorrection does not define any additional fields.
   */
  struct Spec : public NWBDataInterface::Spec
  {
    REGISTER_SPEC(MotionCorrection, NWBDataInterface::Spec)
  };

  /**
   * @brief Destructor
   */
  ~MotionCorrection() override;

  /**
   * @brief Initializes the MotionCorrection from a spec.
   * @param spec The specification of the MotionCorrection.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const Spec& spec);

  DEFINE_UNNAMED_REGISTERED_FIELD(readCorrectedImageStack,
                                  createCorrectedImageStack,
                                  CorrectedImageStack,
                                  "",
                                  Results from motion correction)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the MotionCorrection in the file.
   * @param io A shared pointer to the IO object.
   */
  MotionCorrection(const std::string& path, std::shared_ptr<IO::BaseIO> io);
};
}  // namespace AQNWB::NWB
