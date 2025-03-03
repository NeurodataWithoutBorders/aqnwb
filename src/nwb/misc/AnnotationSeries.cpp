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
                                  const IO::ArrayDataSetConfig& dataConfig)
{
  TimeSeries::initialize(dataConfig,
                         "n/a",  // unit fixed to "n/a"
                         description,
                         comments,
                         1.0f,  // conversion fixed to 1.0, since unit is n/a
                         -1.0f,  // resolution fixed to -1.0
                         0.0f);  // offset fixed to 0.0, since unit is n/a
}

Status AnnotationSeries::writeAnnotation(const SizeType& numSamples,
                                         std::vector<std::string> dataInput,
                                         const void* timestampsInput,
                                         const void* controlInput)
{
  std::vector<SizeType> dataShape = {numSamples};
  std::vector<SizeType> positionOffset = {this->m_samplesRecorded};

  // Write timestamps
  Status tsStatus = Status::Success;
  tsStatus = this->timestamps->writeDataBlock(
      dataShape, positionOffset, this->timestampsType, timestampsInput);

  // Write the data
  Status dataStatus = this->data->writeDataBlock(
      dataShape, positionOffset, this->m_dataType, dataInput);

  // Write the control data if it exists
  if (controlInput != nullptr) {
    tsStatus = this->control->writeDataBlock(
        dataShape, positionOffset, this->controlType, controlInput);
  }

  // track samples recorded
  m_samplesRecorded += numSamples;

  return dataStatus && tsStatus;
}
