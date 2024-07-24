cmake_minimum_required(VERSION 3.15)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

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
    src/BaseIO.cpp
    src/Channel.cpp
    src/hdf5/HDF5IO.cpp
    src/nwb/NWBFile.cpp
    src/nwb/NWBRecording.cpp
    src/nwb/base/TimeSeries.cpp
    src/nwb/device/Device.cpp
    src/nwb/ecephys/ElectricalSeries.cpp
    src/nwb/file/ElectrodeGroup.cpp
    src/nwb/file/ElectrodeTable.cpp
    src/nwb/hdmf/base/Data.hpp
    src/nwb/hdmf/base/Container.cpp
    src/nwb/hdmf/table/DynamicTable.cpp
    src/nwb/hdmf/table/ElementIdentifiers.hpp
    src/nwb/hdmf/table/VectorData.hpp
)

target_include_directories(
    aq-nwb_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_compile_features(aq-nwb_lib PUBLIC cxx_std_17)

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
