#pragma once

#include <string>

#include "BaseIO.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Stores user annotations made during an experiment.
 */
class AnnotationSeries : public TimeSeries
{
public:
  /**
   * @brief Constructor.
   * @param path The location of the AnnotationSeries in the file.
   * @param io A shared pointer to the IO object.
   * @param description The description of the AnnotationSeries.
   * @param comments Human-readable comments about the AnnotationSeries
   */
  AnnotationSeries(const std::string& path,
             std::shared_ptr<BaseIO> io,
             const BaseDataType& dataType = BaseDataType::V_STR,
             const std::string& description = "no description",
             const std::string& comments = "no comments",
             const SizeArray& dsetSize = SizeArray {0},
             const SizeArray& chunkSize = SizeArray {1});

  /**
   * @brief Destructor
   */
  ~AnnotationSeries();

private:
  /**
   * @brief The neurodataType of the AnnotationSeries.
   */
  std::string neurodataType = "AnnotationSeries";
};
}  // namespace AQNWB::NWB
