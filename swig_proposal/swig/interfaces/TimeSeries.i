// Auto-generated SWIG interface for TimeSeries
// Generated from template: timeseries.i.template
// Provides default methods + type-specific alternatives for all BaseRecordingData types

%module aqnwb_timeseries

%{
#include "nwb/base/TimeSeries.hpp"
%}

// Include standard SWIG libraries
%include <std_string.i>
%include <std_vector.i>
%include <std_shared_ptr.i>
%include <std_any.i>

// Shared pointer declarations for referenced types
%shared_ptr(AQNWB::IO::BaseIO)
%shared_ptr(AQNWB::IO::BaseRecordingData)
%shared_ptr(AQNWB::IO::ReadDataWrapper)

// Template instantiations for macro-generated methods
// Provides both default convenience and type-specific performance

// readDescription - default type: std::string
%template(readDescription) AQNWB::NWB::TimeSeries::readDescription<std::string>;

// readComments - default type: std::string
%template(readComments) AQNWB::NWB::TimeSeries::readComments<std::string>;

// readDataConversion - default type: float
%template(readDataConversion) AQNWB::NWB::TimeSeries::readDataConversion<float>;

// readDataUnit - default type: std::string
%template(readDataUnit) AQNWB::NWB::TimeSeries::readDataUnit<std::string>;

// readData - default type: std::any, with type-specific alternatives
%template(readData) AQNWB::NWB::TimeSeries::readData<std::any>;
%template(readDataUint8_T) AQNWB::NWB::TimeSeries::readData<uint8_t>;
%template(readDataUint16_T) AQNWB::NWB::TimeSeries::readData<uint16_t>;
%template(readDataUint32_T) AQNWB::NWB::TimeSeries::readData<uint32_t>;
%template(readDataUint64_T) AQNWB::NWB::TimeSeries::readData<uint64_t>;
%template(readDataInt8_T) AQNWB::NWB::TimeSeries::readData<int8_t>;
%template(readDataInt16_T) AQNWB::NWB::TimeSeries::readData<int16_t>;
%template(readDataInt32_T) AQNWB::NWB::TimeSeries::readData<int32_t>;
%template(readDataInt64_T) AQNWB::NWB::TimeSeries::readData<int64_t>;
%template(readDataFloat) AQNWB::NWB::TimeSeries::readData<float>;
%template(readDataDouble) AQNWB::NWB::TimeSeries::readData<double>;
%template(readDataStd_String) AQNWB::NWB::TimeSeries::readData<std::string>;

// readTimestamps - default type: double, with type-specific alternatives
%template(readTimestamps) AQNWB::NWB::TimeSeries::readTimestamps<double>;

// readControl - default type: uint8_t, with type-specific alternatives
%template(readControl) AQNWB::NWB::TimeSeries::readControl<uint8_t>;



// Include the actual header file
%include "nwb/base/TimeSeries.hpp"
