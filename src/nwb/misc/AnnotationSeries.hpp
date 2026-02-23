#pragma once

#include <string>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/ReadIO.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "spec/core.hpp"

namespace AQNWB::NWB
{
/**
 * @brief TimeSeries storing text-based records about the experiment.
 */
class AnnotationSeries : public TimeSeries
{
public:
  // Register the AnnotationSeries
  REGISTER_SUBCLASS(AnnotationSeries,
                    TimeSeries,
                    AQNWB::SPEC::CORE::namespaceName)

protected:
  /**
   * @brief Constructor.
   * @param path The location of the AnnotationSeries in the file.
   * @param io A shared pointer to the IO object.
   */
  AnnotationSeries(const std::string& path, std::shared_ptr<IO::BaseIO> io);

public:
  /**
   * @brief Destructor
   */
  ~AnnotationSeries() override;

  /**
   * @brief Initializes the AnnotationSeries
   * @param description The description of the AnnotationSeries.
   * @param comments Comments about the AnnotationSeries.
   * @param dataConfig Configuration for the dataset including shape and
   * chunking. The shape must be a vector with one element specifying the length
   * in time. The data type is fixed to variable-length string according to
   * schema.
   * @return Status::Success if successful, otherwise Status::Failure.
   */
  Status initialize(const std::string& description,
                    const std::string& comments,
                    const IO::BaseArrayDataSetConfig& dataConfig);

  /**
   * @brief Writes a channel to an AnnotationSeries dataset.
   * @param numSamples The number of samples to write (length in time).
   * @param dataInput A vector of strings.
   * @param timestampsInput A pointer to the timestamps block.
   * @param controlInput A pointer to the control block data (optional)
   * @return The status of the write operation.
   */
  Status writeAnnotation(const SizeType& numSamples,
                         const std::vector<std::string>& dataInput,
                         const void* timestampsInput,
                         const void* controlInput = nullptr);

  DEFINE_DATASET_FIELD(readData,
                       recordData,
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
