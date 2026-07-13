include(CMakeFindDependencyMacro)

find_dependency(HDF5 COMPONENTS CXX)

include("${CMAKE_CURRENT_LIST_DIR}/aqnwbTargets.cmake")

# The aqnwb_aqnwb target only propagates HDF5 include directories via
# $<BUILD_INTERFACE:...> (see the top-level CMakeLists.txt), so they are not
# part of the installed target's INTERFACE_INCLUDE_DIRECTORIES. Explicitly add
# the HDF5 include directories found above to the imported aqnwb::aqnwb target
# so that downstream consumers (e.g. via find_package(aqnwb)) can locate HDF5
# C/C++ headers regardless of where HDF5 is installed on the system.
target_include_directories(aqnwb::aqnwb SYSTEM INTERFACE ${HDF5_INCLUDE_DIRS})

