// Auto-generated SWIG interface for ElectricalSeries
// Generated from template: timeseries.i.template
// Provides default methods + type-specific alternatives for all BaseRecordingData types

%module aqnwb_electricalseries

%{
#include "nwb/ecephys/ElectricalSeries.hpp"
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







// Include the actual header file
%include "nwb/ecephys/ElectricalSeries.hpp"
