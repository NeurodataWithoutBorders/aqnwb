#!/usr/bin/env python3
"""
Example Python usage of the generated SWIG bindings for TimeSeries.

This demonstrates how users would interact with the aqnwb library
through the automatically generated Python bindings.
"""

# This would be the actual import after SWIG compilation
# import aqnwb_timeseries

def example_timeseries_usage():
    """
    Example of how TimeSeries would be used through Python bindings.
    
    Note: This is a demonstration of the API that would be available
    after the SWIG bindings are compiled. The actual implementation
    would require the full aqnwb library to be built and linked.
    """
    
    print("Example TimeSeries Usage in Python")
    print("=" * 40)
    
    # Example of what the API would look like:
    print("""
# Create an IO object
io = aqnwb_timeseries.HDF5IO("example.nwb")

# Create a TimeSeries object
ts = aqnwb_timeseries.TimeSeries("/acquisition/timeseries", io)

# Use macro-generated accessor methods
description = ts.readDescription_std_string()
print(f"Description: {description.get()}")

comments = ts.readComments_std_string() 
print(f"Comments: {comments.get()}")

# Read data using the macro-generated method
data = ts.readData_std_any()
print(f"Data shape: {data.getShape()}")

# Read timestamps
timestamps = ts.readTimestamps_double()
print(f"Number of timestamps: {timestamps.getSize()}")

# Read conversion factor
conversion = ts.readDataConversion_float()
print(f"Conversion factor: {conversion.get()}")

# Read unit information
unit = ts.readDataUnit_std_string()
print(f"Data unit: {unit.get()}")

# Write data (using the write accessor)
write_data = ts.recordData()
write_data.writeData(new_data_array)

# Write timestamps
write_timestamps = ts.recordTimestamps()
write_timestamps.writeData(new_timestamps_array)
""")

def demonstrate_macro_benefits():
    """
    Demonstrate how the macro-generated methods provide convenience.
    """
    print("\nBenefits of Macro-Generated Bindings:")
    print("-" * 40)
    
    print("""
The SWIG bindings automatically expose all macro-generated methods:

1. Type-safe accessors:
   - readDescription_std_string() -> returns string wrapper
   - readDataConversion_float() -> returns float wrapper
   - readTimestamps_double() -> returns double array wrapper

2. Lazy loading:
   - Methods return wrapper objects that load data on demand
   - Efficient memory usage for large datasets
   - Consistent API across all field types

3. Write capabilities:
   - recordData() -> returns dataset for writing
   - recordTimestamps() -> returns dataset for writing
   - Maintains recording state for streaming data

4. Automatic template instantiation:
   - No manual template specification needed
   - All common types pre-instantiated
   - Consistent naming convention
""")

if __name__ == "__main__":
    example_timeseries_usage()
    demonstrate_macro_benefits()
