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
        [ "Create the recording metadata", "workflow.html#create_recmeta", [
          [ "Create the extracellular recording metadata", "workflow.html#create_recmeta_ecephys", null ]
        ] ],
        [ "Create datasets and add to RecordingContainers.", "workflow.html#create_datasets", null ],
        [ "Start the recording.", "workflow.html#start_recording", null ],
        [ "Write data.", "workflow.html#write_data", null ],
        [ "Stop the recording and finalize the file.", "workflow.html#stop_recording", null ]
      ] ]
    ] ],
    [ "Reading data", "read_page.html", [
      [ "Opening an existing file for reading", "read_page.html#read_example_open", null ],
      [ "Reading NWB neurodata_types", "read_page.html#read_example_registered_type", [
        [ "Reading known RegisteredType objects", "read_page.html#read_example_predefined_registered_type", null ],
        [ "Searching for RegisteredType objects", "read_page.html#read_example_search", null ]
      ] ],
      [ "Reading data from RegisteredType objects", "read_page.html#read_example_read", [
        [ "Reading predefined data fields", "read_page.html#read_design_example_read_posthoc_read_field", null ],
        [ "Reading arbitrary fields", "read_page.html#read_example_arbitrary", null ]
      ] ],
      [ "Further reading", "read_page.html#read_further_reading", null ]
    ] ],
    [ "HDF5 I/O", "hdf5io.html", [
      [ "Optimizing Data Layout", "hdf5io.html#hdf5io_data_layout", [
        [ "Chunking", "hdf5io.html#hdf5io_chunking", null ],
        [ "I/O Filters and Compression", "hdf5io.html#hdf5io_filters", null ],
        [ "Using Chunking and I/O Filters in AqNWB", "hdf5io.html#hdf5io_filters_usage", null ]
      ] ],
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