cmake_minimum_required(VERSION 3.15)
project(nwb_analysis_demo VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find aqnwb - this automatically brings in HDF5 as a transitive dependency
find_package(aqnwb REQUIRED)

# Add the executable
add_executable(nwb_analysis_demo main.cpp)

# Link to aqnwb - HDF5 includes/libraries are provided transitively
target_link_libraries(nwb_analysis_demo PRIVATE aqnwb::aqnwb)

# Set the output directory
set_target_properties(nwb_analysis_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)

# Install rules
install(TARGETS nwb_analysis_demo
    RUNTIME DESTINATION bin
)
