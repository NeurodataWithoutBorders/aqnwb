# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(aq-nwbTests LANGUAGES CXX)

# ---- Tests ----

add_executable(aq-nwb_test aq-nwb_test.cpp)
target_link_libraries(aq-nwb_test PRIVATE aq-nwb_lib)
target_compile_features(aq-nwb_test PRIVATE cxx_std_17)

add_test(NAME aq-nwb_test COMMAND aq-nwb_test)

# ---- End-of-file commands ----

add_folders(Test)
