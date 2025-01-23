var userdocs =
[
    [ "Installing AqNWB", "user_install_page.html", [
      [ "Requirements", "user_install_page.html#user_requirements_sec", null ],
      [ "Source", "user_install_page.html#userbuild_source_sec", null ],
      [ "Build", "user_install_page.html#userbuild_build_sec", null ],
      [ "Install", "user_install_page.html#userbuild_install_sec", null ]
    ] ],
    [ "Acquiring Data", "workflow.html", [
      [ "Overview of a recording workflow", "workflow.html#recording_workflow", [
        [ "Create the I/O object.", "workflow.html#create_io", null ],
        [ "Create the RecordingContainer object.", "workflow.html#create_recording_container", null ],
        [ "Create the NWBFile", "workflow.html#create_nwbfile", null ],
        [ "Create datasets and add to RecordingContainers.", "workflow.html#create_datasets", null ],
        [ "Start the recording.", "workflow.html#start_recording", null ],
        [ "Write data.", "workflow.html#write_data", null ],
        [ "Stop the recording and finalize the file.", "workflow.html#stop_recording", null ]
      ] ]
    ] ],
    [ "Reading data", "read_page.html", [
      [ "Introduction", "read_page.html#read_design_intro", null ],
      [ "Software Design", "read_page.html#read_design_sec", [
        [ "Reading datasets and attributes", "read_page.html#read_design_sec_read_date", [
          [ "Container", "read_page.html#read_design_wrapper_container", null ],
          [ "ReadDataWrapper", "read_page.html#read_design_wrapper_propos", null ],
          [ "DataBlockGeneric and DataBlock", "read_page.html#read_design_data_block", [
            [ "DataBlock with typed data", "read_page.html#read_design_data_block_typed", null ],
            [ "Using Boost Multi Array for N-Dimensional Data", "read_page.html#read_design_data_block_multiarray", null ]
          ] ],
          [ "I/O", "read_page.html#read_design_wrapper_io", null ]
        ] ],
        [ "Reading typed objects", "read_page.html#read_design_sec_read_types", [
          [ "RegisteredType", "read_page.html#read_design_wrapper_registeredType", null ],
          [ "Child classes of RegisteredType (e.g., Container)", "read_page.html#read_design_wrapper_subtypes", null ]
        ] ]
      ] ],
      [ "Example", "read_page.html#read_design_example", [
        [ "Create a NWB file as usual", "read_page.html#read_design_example_create", [
          [ "Setup mock data for write", "read_page.html#read_design_example_step_1", null ],
          [ "Create the NWBFile and record data", "read_page.html#read_design_example_step_1_2", null ]
        ] ],
        [ "Reading Datasets and Attributes", "read_page.html#read_design_example_read_during", [
          [ "Lazy data access", "read_page.html#read_design_example_lazy_read", null ],
          [ "Check that the object exists", "read_page.html#read_design_example_check_exists", null ],
          [ "Read data into memory", "read_page.html#read_design_example_load_data", null ],
          [ "Accessing multi-dimensional data as Boost multi-array", "read_page.html#read_design_example_boostarray", null ],
          [ "Reading an attribute", "read_page.html#read_design_example_attribute", null ],
          [ "Reading data with unknown type", "read_page.html#read_design_example_readgeneric", null ]
        ] ],
        [ "Finalize the recording", "read_page.html#read_design_example_stop_recording", null ],
        [ "Reading from an existing file", "read_page.html#read_design_example_read_from_existing_file", [
          [ "Opening an existing file for reading", "read_page.html#read_design_example_read_create_io", null ],
          [ "Searching for Registered Type Objects (e.g.,ElectricalSeries)", "read_page.html#read_design_example_read_posthoc_search", null ],
          [ "Reading the Registered Type Objects", "read_page.html#read_design_example_read_posthoc_read", null ],
          [ "Reading data fields", "read_page.html#read_design_example_read_posthoc_read_field", null ],
          [ "Reading arbitrary fields", "read_page.html#read_design_example_read_arbitrary_field", null ]
        ] ]
      ] ]
    ] ],
    [ "HDF5 I/O", "hdf5io.html", [
      [ "Chunking", "hdf5io.html#hdf5io_chunking", null ],
      [ "Single-Writer Multiple-Reader (SWMR) Mode", "hdf5io.html#hdf5io_swmr", [
        [ "Why does AqNWB use SMWR mode?", "hdf5io.html#hdf5io_swmr_features", null ],
        [ "Writing an NWB file with SWMR mode", "hdf5io.html#hdf5io_swmr_workflow", [
          [ "Code Examples", "hdf5io.html#hdf5io_swmr_examples", [
            [ "Workflow with SWMR", "hdf5io.html#hdf5io_swmr_examples_with_swmr", null ],
            [ "Workflow with SWMR disabled", "hdf5io.html#hdf5io_noswmr_examples_without_swmr", null ]
          ] ]
        ] ],
        [ "Reading with SWMR mode", "hdf5io.html#hdf5io_swmr_read", null ]
      ] ]
    ] ]
];