
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
   * @param dataType The data type to use for storing the user annotations
   * @param description The description of the AnnotationSeries.
   * @param dsetSize Initial size of the main dataset. This must be a vector
   *                 with one element specifying the length in time.
   * @param chunkSize Chunk size to use.
   */
  void initialize(const IO::BaseDataType& dataType,
                  const std::string& description,
                  const std::string& comments,
                  const SizeArray& dsetSize,
                  const SizeArray& chunkSize);

  /**
   * @brief Writes a channel to an AnnotationSeries dataset.
   * @param numSamples The number of samples to write (length in time).
   * @param dataInput A pointer to the data block.
   * @param timestampsInput A pointer to the timestamps block.
   * @return The status of the write operation.
   */
  Status writeAnnotation(const SizeType& numSamples,
                         const void* dataInput,
                         const void* timestampsInput);

  DEFINE_FIELD(readData,
               DatasetField,
               std::any,
               "data",
               Annotations made during an experiment.)

private:
  /**
   * @brief The number of samples already written per channel.
   */
  SizeType m_samplesRecorded;
};
}  // namespace AQNWB::NWB
