# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(aqnwbTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(aqnwb_test
    testData.cpp
    testDevice.cpp
    testEcephys.cpp
    testElementIdentifiers.cpp
    testFile.cpp
    testHDF5IO.cpp
    testMisc.cpp
    testNWBFile.cpp
    testRecordingWorkflow.cpp
    testRegisteredType.cpp
    testTimeSeries.cpp
    testUtilsFunctions.cpp
    testVectorData.cpp
    examples/test_HDF5IO_examples.cpp
    examples/test_example.cpp
    examples/testWorkflowExamples.cpp
    examples/test_ecephys_data_read.cpp
    examples/test_RegisteredType_example.cpp
)

# Ensure the aqnwb_test target can include headers from the current directory
target_include_directories(aqnwb_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # Include current directory
)

target_link_libraries(
    aqnwb_test PRIVATE
    aqnwb::aqnwb
    Catch2::Catch2WithMain
)
target_compile_features(aqnwb_test PRIVATE cxx_std_17)
target_compile_definitions(aqnwb_test PRIVATE BUILD_CONFIG=\"$<CONFIG>\")
catch_discover_tests(aqnwb_test)

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
    aqnwb_aqnwb
)

target_compile_features(reader_executable PRIVATE cxx_std_17)

# ---- End-of-file commands ----

add_folders(aqnwbTests)
