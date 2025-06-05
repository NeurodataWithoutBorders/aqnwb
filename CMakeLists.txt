cmake_minimum_required(VERSION 3.15)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

include(cmake/prelude.cmake)

project(
    aqnwb
    VERSION 0.1.0
    DESCRIPTION "An API to acquire neurophysiology data in the NWB format"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    aqnwb_aqnwb
    src/io/BaseIO.cpp
    src/Channel.cpp
    src/io/hdf5/HDF5IO.cpp
    src/io/hdf5/HDF5RecordingData.cpp
    src/io/hdf5/HDF5ArrayDataSetConfig.cpp
    src/nwb/NWBFile.cpp
    src/nwb/RecordingContainers.cpp
    src/nwb/RegisteredType.cpp
    src/nwb/base/TimeSeries.cpp
    src/nwb/device/Device.cpp
    src/nwb/ecephys/ElectricalSeries.cpp
    src/nwb/ecephys/SpikeEventSeries.cpp
    src/nwb/file/ElectrodeGroup.cpp
    src/nwb/file/ElectrodeTable.cpp
    src/nwb/misc/AnnotationSeries.cpp
    src/nwb/hdmf/base/Container.cpp
    src/nwb/hdmf/base/Data.cpp
    src/nwb/hdmf/table/DynamicTable.cpp
    src/nwb/hdmf/table/VectorData.cpp
    src/nwb/hdmf/table/ElementIdentifiers.cpp
    src/spec/NamespaceRegistry.cpp
)

add_library(aqnwb::aqnwb ALIAS aqnwb_aqnwb)

include(GenerateExportHeader)
generate_export_header(
    aqnwb_aqnwb
    BASE_NAME aqnwb
    EXPORT_FILE_NAME export/aqnwb/aqnwb_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(aqnwb_aqnwb PUBLIC SHARED_STATIC_DEFINE)
endif()

set_target_properties(
    aqnwb_aqnwb PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME aqnwb
    OUTPUT_NAME aqnwb
)

target_include_directories(
    aqnwb_aqnwb ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    aqnwb_aqnwb ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_include_directories(
    aqnwb_aqnwb SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(aqnwb_aqnwb PUBLIC cxx_std_17)
target_compile_definitions(aqnwb_aqnwb PUBLIC BOOST_NO_CXX98_FUNCTION_BASE)

# ---- Additional libraries needed ----
find_package(HDF5 REQUIRED COMPONENTS CXX)
include_directories(${HDF5_INCLUDE_DIRS})

find_package(Boost REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

target_link_libraries(aqnwb_aqnwb ${HDF5_CXX_LIBRARIES} ${Boost_LIBRARIES})
if (WIN32)
    target_link_libraries(aqnwb_aqnwb bcrypt)
endif()

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT aqnwb_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of aqnwb"
  )
endif()

include(cmake/dev-mode.cmake)
