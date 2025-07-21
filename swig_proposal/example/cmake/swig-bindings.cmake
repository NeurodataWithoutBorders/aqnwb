# Example CMakeLists.txt showing SWIG integration for TimeSeries
# This demonstrates how the proposed SWIG binding system would integrate
# with the existing aqnwb CMake build system.

cmake_minimum_required(VERSION 3.15)

# Find required packages
find_package(SWIG REQUIRED)
include(UseSWIG)

# Find Python for running the generation script
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Custom target to generate SWIG interface files
add_custom_target(generate_swig_interfaces
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generate_timeseries_swig.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating SWIG interface files from aqnwb classes"
    BYPRODUCTS TimeSeries.i
)

# Function to create SWIG bindings for a specific language
function(create_swig_binding MODULE_NAME INTERFACE_FILE LANGUAGE)
    # Set properties for SWIG
    set_property(SOURCE ${INTERFACE_FILE} PROPERTY CPLUSPLUS ON)
    set_property(SOURCE ${INTERFACE_FILE} PROPERTY SWIG_MODULE_NAME ${MODULE_NAME})
    
    # Create the SWIG library
    swig_add_library(${MODULE_NAME}_${LANGUAGE}
        TYPE SHARED
        LANGUAGE ${LANGUAGE}
        SOURCES ${INTERFACE_FILE}
    )
    
    # Link with aqnwb library (would be aqnwb::aqnwb in real implementation)
    # target_link_libraries(${MODULE_NAME}_${LANGUAGE} aqnwb::aqnwb)
    
    # Set include directories
    target_include_directories(${MODULE_NAME}_${LANGUAGE} PRIVATE 
        ${CMAKE_SOURCE_DIR}/src  # Would point to actual aqnwb src directory
    )
    
    # Make sure the interface file is generated first
    add_dependencies(${MODULE_NAME}_${LANGUAGE} generate_swig_interfaces)
    
    message(STATUS "Created SWIG binding: ${MODULE_NAME}_${LANGUAGE}")
endfunction()

# Example: Create bindings for TimeSeries in multiple languages
if(AQNWB_BUILD_SWIG_BINDINGS)
    # Python bindings
    create_swig_binding(aqnwb_timeseries TimeSeries.i python)
    
    # C# bindings  
    create_swig_binding(aqnwb_timeseries TimeSeries.i csharp)
    
    # Java bindings
    create_swig_binding(aqnwb_timeseries TimeSeries.i java)
endif()

# Example of how to build all SWIG bindings
add_custom_target(swig_bindings
    COMMENT "Building all SWIG language bindings"
)

if(AQNWB_BUILD_SWIG_BINDINGS)
    add_dependencies(swig_bindings 
        aqnwb_timeseries_python
        aqnwb_timeseries_csharp  
        aqnwb_timeseries_java
    )
endif()

# Installation rules for generated bindings
if(AQNWB_BUILD_SWIG_BINDINGS)
    # Install Python module
    install(TARGETS aqnwb_timeseries_python
        DESTINATION lib/python/site-packages
    )
    
    # Install C# assembly
    install(TARGETS aqnwb_timeseries_csharp
        DESTINATION lib/csharp
    )
    
    # Install Java library
    install(TARGETS aqnwb_timeseries_java
        DESTINATION lib/java
    )
endif()
