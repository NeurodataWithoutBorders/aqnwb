include(CMakeFindDependencyMacro)

find_dependency(HDF5 COMPONENTS CXX)

# When aqnwb was built with AQNWB_USE_REMFILE=ON, the exported target
# references remfile::remfile. remfile installs into the same prefix as
# aqnwb when it was fetched at build time, so this lookup is quiet and
# harmless when aqnwb was built without remfile support.
find_package(remfile CONFIG QUIET)

include("${CMAKE_CURRENT_LIST_DIR}/aqnwbTargets.cmake")
