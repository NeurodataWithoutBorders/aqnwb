# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(aqnwbTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(aqnwb_test
    testBase.cpp
    testEcephys.cpp
    testFile.cpp
    testHDF5IO.cpp
    testNWBFile.cpp
    testNWBRecording.cpp)
target_link_libraries(
    aqnwb_test PRIVATE
    aqnwb::aqnwb
    Catch2::Catch2WithMain
)
target_compile_features(aqnwb_test PRIVATE cxx_std_17)

catch_discover_tests(aqnwb_test)

# ---- End-of-file commands ----

add_folders(aqnwbTests)
