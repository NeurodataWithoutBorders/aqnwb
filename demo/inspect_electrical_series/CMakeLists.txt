cmake_minimum_required(VERSION 3.15)
project(nwb_analysis_demo VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow configuring paths to dependencies
set(AQNWB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../build/dev" CACHE PATH "Path to aqnwb build directory")
set(HDF5_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/hdf5_build/install/cmake" CACHE PATH "Path to HDF5 build directory")

# Disable compiler flags that cause issues on macOS
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
endif()

# Find required dependencies
find_package(HDF5 REQUIRED COMPONENTS CXX)

# Add the executable
add_executable(nwb_analysis_demo main.cpp)

# Include directories
target_include_directories(nwb_analysis_demo PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${HDF5_INCLUDE_DIRS}
)

# Find the aqnwb library
if(EXISTS "${AQNWB_DIR}/libaqnwb.a")
    set(AQNWB_LIBRARY "${AQNWB_DIR}/libaqnwb.a")
elseif(EXISTS "${AQNWB_DIR}/libaqnwb.so")
    set(AQNWB_LIBRARY "${AQNWB_DIR}/libaqnwb.so")
elseif(EXISTS "${AQNWB_DIR}/libaqnwb.dylib")
    set(AQNWB_LIBRARY "${AQNWB_DIR}/libaqnwb.dylib")
else()
    message(FATAL_ERROR "Could not find aqnwb library in ${AQNWB_DIR}. Please build the main project first or set AQNWB_DIR to the correct path.")
endif()

# Link libraries
target_link_libraries(nwb_analysis_demo 
    ${AQNWB_LIBRARY}
    ${HDF5_CXX_LIBRARIES}
)

# If on Windows, link bcrypt
if(WIN32)
    target_link_libraries(nwb_analysis_demo bcrypt)
endif()

# Set the output directory
set_target_properties(nwb_analysis_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)

# Install rules
install(TARGETS nwb_analysis_demo
    RUNTIME DESTINATION bin
)
