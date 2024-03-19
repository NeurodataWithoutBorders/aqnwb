cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    aq-nwb
    VERSION 0.1.0
    DESCRIPTION "An API to acquire neurophysiology data in the NWB format"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    aq-nwb_lib OBJECT
    src/HDF5IO.cpp
    src/BaseIO.cpp
    src/NWBFile.cpp
    src/NWBDataTypes.cpp
    src/Utils.hpp

)

target_include_directories(
    aq-nwb_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_compile_features(aq-nwb_lib PUBLIC cxx_std_17)

# ---- Declare executable ----

add_executable(aq-nwb_exe src/main.cpp)
add_executable(aq-nwb::exe ALIAS aq-nwb_exe)

set_property(TARGET aq-nwb_exe PROPERTY OUTPUT_NAME aq-nwb)

target_compile_features(aq-nwb_exe PRIVATE cxx_std_17)

target_link_libraries(aq-nwb_exe PRIVATE aq-nwb_lib)

# ---- Additional libraries needed ----
find_package(HDF5 REQUIRED COMPONENTS CXX)
include_directories(${HDF5_INCLUDE_DIRS})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(aq-nwb_lib ${HDF5_CXX_LIBRARIES} ${Boost_LIBRARIES})

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT aq-nwb_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of aq-nwb"
  )
endif()

include(cmake/dev-mode.cmake)
