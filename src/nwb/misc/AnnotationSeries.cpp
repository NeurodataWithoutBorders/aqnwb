#include "nwb/misc/AnnotationSeries.hpp"

using namespace AQNWB::NWB;

// AnnotationSeries

/** Constructor */
AnnotationSeries::AnnotationSeries(const std::string& path,
                                   std::shared_ptr<BaseIO> io,
                                   const BaseDataType& dataType,
                                   const std::string& description,
                                   const std::string& comments,
                                   const SizeArray& dsetSize,
                                   const SizeArray& chunkSize)
    : TimeSeries(path,
                 io,
                 dataType,
                 "n/a",  // default unit is 'n/a' for AnnotationSeries
                 description,
                 comments,
                 dsetSize,
                 chunkSize)
{
}

/** Destructor */
AnnotationSeries::~AnnotationSeries() {}
