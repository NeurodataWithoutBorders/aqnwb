
#include "nwb/misc/AnnotationSeries.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// AnnotationSeries
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(AnnotationSeries)

/** Constructor */
AnnotationSeries::AnnotationSeries(const std::string& path,
                                   std::shared_ptr<IO::BaseIO> io)
    : TimeSeries(path, io)
{
}

/** Destructor */
AnnotationSeries::~AnnotationSeries() {}

/** Initialization function*/
void AnnotationSeries::initialize(const std::string& description,
                                  const std::string& comments,
                                  const SizeArray& dsetSize,
                                  const SizeArray& chunkSize)
{
  TimeSeries::initialize(
      IO::BaseDataType::V_STR, // fixed to string according to schema
      "n/a",  // unit fixed to "n/a"
      description,
      comments,
      dsetSize,
      chunkSize,
      1.0f,  // conversion fixed to 1.0, since unit is n/a
      -1.0f,  // resolution fixed to -1.0
      0.0f);  // offset fixed to 0.0, since unit is n/a
}

Status AnnotationSeries::writeAnnotation(const SizeType& numSamples,
                                         const void* dataInput,
                                         const void* timestampsInput,
                                         const void* controlInput)
{
  // get offsets and datashape
  std::vector<SizeType> dataShape = {numSamples};
  std::vector<SizeType> positionOffset = {m_samplesRecorded};

  // track samples recorded per channel
  m_samplesRecorded += numSamples;

  // write channel data
  return writeData(dataShape, positionOffset, dataInput, timestampsInput, controlInput);
}