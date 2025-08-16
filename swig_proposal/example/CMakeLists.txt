# CMakeLists.txt - Example integration of SWIG bindings with aqnwb build system
# This demonstrates how SWIG binding generation would integrate with the existing CMake setup

cmake_minimum_required(VERSION 3.21)

project(aqnwb_swig_example
    VERSION 1.0.0
    DESCRIPTION "Example SWIG bindings for aqnwb library"
    LANGUAGES CXX
)

# Include the SWIG bindings CMake functions
include(cmake/swig-bindings.cmake)

# Find required packages
find_package(SWIG 4.0 REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Optional: Find .NET for C# bindings
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(DOTNET dotnet)
endif()

# Optional: Find Java for Java bindings
find_package(Java QUIET)
find_package(JNI QUIET)

# Configuration options
option(AQNWB_BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(AQNWB_BUILD_CSHARP_BINDINGS "Build C# bindings" ${DOTNET_FOUND})
option(AQNWB_BUILD_JAVA_BINDINGS "Build Java bindings" ${Java_FOUND})

# Set up SWIG
include(UseSWIG)
set(CMAKE_SWIG_FLAGS -c++)

# Define source directories (would be actual aqnwb paths in real implementation)
set(AQNWB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(AQNWB_INCLUDE_DIR ${AQNWB_SOURCE_DIR}/src)

# SWIG interface generation
set(SWIG_INTERFACE_DIR ${CMAKE_CURRENT_BINARY_DIR}/swig/interfaces)
set(SWIG_TEMPLATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/swig_templates)

# Custom target to generate SWIG interfaces
add_custom_target(generate_swig_interfaces
    COMMAND ${Python3_EXECUTABLE} 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/generate_swig_interfaces.py
        --source-dir ${AQNWB_SOURCE_DIR}
        --template-dir ${SWIG_TEMPLATE_DIR}
        --output-dir ${SWIG_INTERFACE_DIR}
        --languages python,csharp,java
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating SWIG interface files from aqnwb classes"
    VERBATIM
)

# Ensure interfaces are generated before building bindings
file(MAKE_DIRECTORY ${SWIG_INTERFACE_DIR})

# List of NWB classes to generate bindings for
set(NWB_CLASSES
    TimeSeries
    ElectricalSeries
    Container
    # In real implementation, this would be auto-generated from schema analysis
)

# Generate SWIG interfaces at configure time (for demonstration)
execute_process(
    COMMAND ${Python3_EXECUTABLE} 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/generate_swig_interfaces.py
        --source-dir ${AQNWB_SOURCE_DIR}
        --template-dir ${SWIG_TEMPLATE_DIR}
        --output-dir ${SWIG_INTERFACE_DIR}
        --languages python,csharp,java
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE SWIG_GEN_RESULT
)

if(NOT SWIG_GEN_RESULT EQUAL 0)
    message(WARNING "SWIG interface generation failed, using existing interfaces")
endif()

# Python bindings
if(AQNWB_BUILD_PYTHON_BINDINGS)
    message(STATUS "Configuring Python bindings")
    
    foreach(CLASS_NAME ${NWB_CLASSES})
        set(INTERFACE_FILE ${SWIG_INTERFACE_DIR}/${CLASS_NAME}.i)
        
        if(EXISTS ${INTERFACE_FILE})
            # Create SWIG module for each class
            set_property(SOURCE ${INTERFACE_FILE} PROPERTY CPLUSPLUS ON)
            set_property(SOURCE ${INTERFACE_FILE} PROPERTY SWIG_MODULE_NAME aqnwb_${CLASS_NAME})
            
            swig_add_library(aqnwb_${CLASS_NAME}_python
                TYPE SHARED
                LANGUAGE python
                SOURCES ${INTERFACE_FILE}
            )
            
            # Set include directories
            target_include_directories(aqnwb_${CLASS_NAME}_python PRIVATE
                ${AQNWB_INCLUDE_DIR}
                ${Python3_INCLUDE_DIRS}
            )
            
            # Link libraries (would link to actual aqnwb library)
            target_link_libraries(aqnwb_${CLASS_NAME}_python PRIVATE
                ${Python3_LIBRARIES}
                # aqnwb::aqnwb  # Would link to actual aqnwb library
            )
            
            # Set output directory
            set_target_properties(aqnwb_${CLASS_NAME}_python PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/python
                ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/python
            )
            
            message(STATUS "  Added Python binding for ${CLASS_NAME}")
        else()
            message(WARNING "SWIG interface not found: ${INTERFACE_FILE}")
        endif()
    endforeach()
endif()

# C# bindings
if(AQNWB_BUILD_CSHARP_BINDINGS)
    message(STATUS "Configuring C# bindings")
    
    foreach(CLASS_NAME ${NWB_CLASSES})
        set(INTERFACE_FILE ${SWIG_INTERFACE_DIR}/${CLASS_NAME}.i)
        
        if(EXISTS ${INTERFACE_FILE})
            # Create SWIG module for each class
            set_property(SOURCE ${INTERFACE_FILE} PROPERTY CPLUSPLUS ON)
            set_property(SOURCE ${INTERFACE_FILE} PROPERTY SWIG_MODULE_NAME aqnwb_${CLASS_NAME})
            
            swig_add_library(aqnwb_${CLASS_NAME}_csharp
                TYPE SHARED
                LANGUAGE csharp
                SOURCES ${INTERFACE_FILE}
            )
            
            # Set include directories
            target_include_directories(aqnwb_${CLASS_NAME}_csharp PRIVATE
                ${AQNWB_INCLUDE_DIR}
            )
            
            # Link libraries (would link to actual aqnwb library)
            target_link_libraries(aqnwb_${CLASS_NAME}_csharp PRIVATE
                # aqnwb::aqnwb  # Would link to actual aqnwb library
            )
            
            # Set output directory
            set_target_properties(aqnwb_${CLASS_NAME}_csharp PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/csharp
                ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/csharp
            )
            
            message(STATUS "  Added C# binding for ${CLASS_NAME}")
        endif()
    endforeach()
    
    # Create C# extension library project file
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/AqnwbExtensions.cs
        ${CMAKE_CURRENT_BINARY_DIR}/swig/csharp/AqnwbExtensions.cs
        COPYONLY
    )
endif()

# Java bindings
if(AQNWB_BUILD_JAVA_BINDINGS AND Java_FOUND AND JNI_FOUND)
    message(STATUS "Configuring Java bindings")
    
    foreach(CLASS_NAME ${NWB_CLASSES})
        set(INTERFACE_FILE ${SWIG_INTERFACE_DIR}/${CLASS_NAME}.i)
        
        if(EXISTS ${INTERFACE_FILE})
            # Create SWIG module for each class
            set_property(SOURCE ${INTERFACE_FILE} PROPERTY CPLUSPLUS ON)
            set_property(SOURCE ${INTERFACE_FILE} PROPERTY SWIG_MODULE_NAME aqnwb_${CLASS_NAME})
            
            swig_add_library(aqnwb_${CLASS_NAME}_java
                TYPE SHARED
                LANGUAGE java
                SOURCES ${INTERFACE_FILE}
            )
            
            # Set include directories
            target_include_directories(aqnwb_${CLASS_NAME}_java PRIVATE
                ${AQNWB_INCLUDE_DIR}
                ${JNI_INCLUDE_DIRS}
            )
            
            # Link libraries (would link to actual aqnwb library)
            target_link_libraries(aqnwb_${CLASS_NAME}_java PRIVATE
                ${JNI_LIBRARIES}
                # aqnwb::aqnwb  # Would link to actual aqnwb library
            )
            
            # Set output directory
            set_target_properties(aqnwb_${CLASS_NAME}_java PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/java
                ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/java
            )
            
            message(STATUS "  Added Java binding for ${CLASS_NAME}")
        endif()
    endforeach()
endif()

# Installation rules
if(AQNWB_BUILD_PYTHON_BINDINGS)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/python/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/python
        FILES_MATCHING PATTERN "*.so" PATTERN "*.py"
    )
endif()

if(AQNWB_BUILD_CSHARP_BINDINGS)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/csharp/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/csharp
        FILES_MATCHING PATTERN "*.dll" PATTERN "*.cs"
    )
endif()

if(AQNWB_BUILD_JAVA_BINDINGS)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig/java/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/java
        FILES_MATCHING PATTERN "*.so" PATTERN "*.jar" PATTERN "*.java"
    )
endif()

# Summary
message(STATUS "")
message(STATUS "SWIG Bindings Configuration Summary:")
message(STATUS "  Python bindings: ${AQNWB_BUILD_PYTHON_BINDINGS}")
message(STATUS "  C# bindings: ${AQNWB_BUILD_CSHARP_BINDINGS}")
message(STATUS "  Java bindings: ${AQNWB_BUILD_JAVA_BINDINGS}")
message(STATUS "  SWIG interface directory: ${SWIG_INTERFACE_DIR}")
message(STATUS "  Number of NWB classes: ${CMAKE_LIST_LENGTH NWB_CLASSES}")
message(STATUS "")
