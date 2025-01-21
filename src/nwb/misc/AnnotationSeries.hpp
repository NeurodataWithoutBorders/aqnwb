
#pragma once

#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/TimeSeries.hpp"

namespace AQNWB::NWB
{
/**
 * @brief TimeSeries storing text-based records about the experiment.
 */
class AnnotationSeries : public TimeSeries
{
public:
  // Register the AnnotationSeries
  REGISTER_SUBCLASS(AnnotationSeries, "core")

  /**
   * @brief Constructor.
   * @param path The location of the AnnotationSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  AnnotationSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor
   */
  ~AnnotationSeries();

  /**
   * @brief Initializes the AnnotationSeries
   * @param description The description of the AnnotationSeries.
   * @param dsetSize Initial size of the main dataset. This must be a vector
   *                 with one element specifying the length in time.
   * @param chunkSize Chunk size to use.
   */
  void initialize(const std::string& description,
                  const std::string& comments,
                  const SizeArray& dsetSize,
                  const SizeArray& chunkSize);

  /**
   * @brief Writes a channel to an AnnotationSeries dataset.
   * @param numSamples The number of samples to write (length in time).
   * @param dataInput A vector of strings.
   * @param timestampsInput A pointer to the timestamps block.
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeAnnotation(const SizeType& numSamples,
                         const std::vector<std::string> dataInput,
                         const void* timestampsInput,
                         const void* controlInput = nullptr);

  DEFINE_FIELD(readData,
               DatasetField,
               std::string,
               "data",
               Annotations made during an experiment.)

private:
  /**
   * @brief The number of samples already written per channel.
   */
  SizeType m_samplesRecorded = 0;
};
}  // namespace AQNWB::NWB
