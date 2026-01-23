include(CMakeFindDependencyMacro)

find_dependency(HDF5 COMPONENTS CXX)

include("${CMAKE_CURRENT_LIST_DIR}/aqnwbTargets.cmake")