# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(aq-nwbTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(aq-nwb_test
    testBase.cpp
    testEcephys.cpp
    testFile.cpp
    testHDF5IO.cpp
    testNWBFile.cpp
    testNWBRecording.cpp
    examples/test_HDF5IO_examples.cpp
    examples/test_example.cpp
)

# Ensure the aq-nwb_test target can include headers from the current directory
target_include_directories(aq-nwb_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # Include current directory
)

target_link_libraries(
    aq-nwb_test PRIVATE
    aq-nwb_lib
    Catch2::Catch2WithMain
)

target_compile_features(aq-nwb_test PRIVATE cxx_std_17)

catch_discover_tests(aq-nwb_test)

# ---- Custom Executable ----

add_executable(reader_executable
    reader.cpp
)

# Ensure the reader_executable target can include headers from the current directory
target_include_directories(reader_executable PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # Include current directory
)

target_link_libraries(
    reader_executable PRIVATE
    aq-nwb_lib
)

target_compile_features(reader_executable PRIVATE cxx_std_17)

# ---- End-of-file commands ----

add_folders(aq-nwbTests)
