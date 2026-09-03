var userdocs =
[
    [ "Installation 🛠️", "user_install_page.html", [
      [ "Requirements", "user_install_page.html#user_requirements_sec", null ],
      [ "Source", "user_install_page.html#userbuild_source_sec", null ],
      [ "Build", "user_install_page.html#userbuild_build_sec", null ],
      [ "Install", "user_install_page.html#userbuild_install_sec", null ]
    ] ],
    [ "Acquiring Data 📊", "workflow.html", [
      [ "Overview of a recording workflow", "workflow.html#recording_workflow", [
        [ "1. Create the I/O object", "workflow.html#create_io", null ],
        [ "2. Create the NWBFile", "workflow.html#create_nwbfile", null ],
        [ "3. Create the recording metadata", "workflow.html#create_recmeta", [
          [ "Create the extracellular recording metadata", "workflow.html#create_recmeta_ecephys", null ]
        ] ],
        [ "4. Create datasets and add to RecordingObjects", "workflow.html#create_datasets", null ],
        [ "5. Start the recording", "workflow.html#start_recording", null ],
        [ "6. Write data", "workflow.html#write_data", [
          [ "Writing all channels at once (interleaved data)", "workflow.html#write_data_allchannels", null ]
        ] ],
        [ "7. Stop the recording and finalize the file", "workflow.html#stop_recording", null ]
      ] ],
      [ "Advanced: Recording individual datasets", "workflow.html#advanced_recording_workflow", [
        [ "Resetting the recording position", "workflow.html#advanced_recording_workflow_reset", null ]
      ] ]
    ] ],
    [ "Acquiring Event Data 📋", "events.html", [
      [ "Overview", "events.html#events_overview", null ],
      [ "Setup: I/O, NWBFile, and table creation", "events.html#events_setup", [
        [ "1. Create the I/O object", "events.html#events_io", null ],
        [ "2. Create the NWBFile", "events.html#events_nwbfile", null ],
        [ "3. Configure and create the EventsTable", "events.html#events_configure_table", null ],
        [ "4. Start the recording", "events.html#events_start", null ]
      ] ],
      [ "Pattern 1: Row-based acquisition", "events.html#events_rowbased", [
        [ "Add rows", "events.html#events_rowbased_addrow", null ],
        [ "Stop the recording", "events.html#events_rowbased_stop", null ]
      ] ],
      [ "Pattern 2: Column-based (bulk) acquisition", "events.html#events_colbased", [
        [ "Add data to columns", "events.html#events_colbased_adddata", null ],
        [ "Stop the recording", "events.html#events_colbased_stop", null ]
      ] ],
      [ "Annotating columns with a MeaningsTable", "events.html#events_meanings", [
        [ "Setup: create the EventsTable with a coded column", "events.html#events_meanings_setup", null ],
        [ "Create the MeaningsTable", "events.html#events_meanings_create", null ],
        [ "Start the recording", "events.html#events_meanings_start", null ],
        [ "Write event data", "events.html#events_meanings_write_events", null ],
        [ "Write meanings data", "events.html#events_meanings_write_meanings", null ],
        [ "Stop the recording", "events.html#events_meanings_stop", null ]
      ] ],
      [ "Tips and best practices", "events.html#events_tips", null ]
    ] ],
    [ "Annotating Time Intervals ⏱️", "time_intervals.html", [
      [ "Overview", "time_intervals.html#time_intervals_overview", null ],
      [ "Setup: I/O, NWBFile, and table creation", "time_intervals.html#time_intervals_setup", [
        [ "1. Create the I/O object", "time_intervals.html#time_intervals_io", null ],
        [ "2. Create the NWBFile", "time_intervals.html#time_intervals_nwbfile", null ],
        [ "3. Configure and create the TimeIntervals table", "time_intervals.html#time_intervals_configure_table", null ],
        [ "4. Start the recording", "time_intervals.html#time_intervals_start", null ]
      ] ],
      [ "Pattern 1: Row-based acquisition", "time_intervals.html#time_intervals_rowbased", [
        [ "Add rows", "time_intervals.html#time_intervals_rowbased_addrow", null ],
        [ "Stop the recording", "time_intervals.html#time_intervals_rowbased_stop", null ]
      ] ],
      [ "Pattern 2: Column-based (bulk) acquisition", "time_intervals.html#time_intervals_colbased", [
        [ "Add data to columns", "time_intervals.html#time_intervals_colbased_adddata", null ],
        [ "Stop the recording", "time_intervals.html#time_intervals_colbased_stop", null ]
      ] ],
      [ "Further reading", "time_intervals.html#time_intervals_further_reading", null ]
    ] ],
    [ "Reading Data 📤", "read_page.html", [
      [ "Opening an existing file for reading", "read_page.html#read_example_open", null ],
      [ "Reading NWB neurodata_types", "read_page.html#read_example_registered_type", [
        [ "Reading known RegisteredType objects", "read_page.html#read_example_predefined_registered_type", null ],
        [ "Searching for RegisteredType objects", "read_page.html#read_example_search", null ]
      ] ],
      [ "Reading data from RegisteredType objects", "read_page.html#read_example_read", [
        [ "Reading predefined data fields", "read_page.html#read_design_example_read_posthoc_read_field", null ],
        [ "Reading arbitrary fields", "read_page.html#read_example_arbitrary", null ],
        [ "Working with fields with unknown data type", "read_page.html#read_example_variant_data", null ],
        [ "Reading data from DynamicTable objects", "read_page.html#read_example_dynamic_table", null ]
      ] ],
      [ "Further reading", "read_page.html#read_further_reading", null ]
    ] ],
    [ "Reading Remote Data ☁️", "reads3_page.html", [
      [ "Reading via the ROS3 VFD", "reads3_page.html#reads3_ros3", [
        [ "Opening a file from S3", "reads3_page.html#reads3_ros3_open", null ],
        [ "Reading objects and data", "reads3_page.html#reads3_ros3_read", null ]
      ] ],
      [ "Reading via the remfile VFD", "reads3_page.html#reads3_remfile", null ],
      [ "Further reading", "reads3_page.html#reads3_further_reading", null ]
    ] ],
    [ "HDF5 I/O 📂", "hdf5io.html", [
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
    ] ],
    [ "Using Links 🔗", "links.html", [
      [ "Overview", "links.html#link_overview", null ],
      [ "Use Case: Time-Aligned TimeSeries", "links.html#link_use_case", [
        [ "1. Setup", "links.html#link_setup", null ],
        [ "2. Create the Original TimeSeries", "links.html#link_original", null ],
        [ "3. Create a ProcessingModule", "links.html#link_processing", null ],
        [ "4. Create the Linked TimeSeries", "links.html#link_linked", null ],
        [ "5. Link to the Original Series", "links.html#link_reference", null ],
        [ "6. Cleanup", "links.html#link_cleanup", null ],
        [ "Verification", "links.html#link_verification", null ]
      ] ]
    ] ],
    [ "Demos 📝", "user_demos.html", [
      [ "Inspect Electrical Series Data using AqNWB", "user_demos.html#demos_inspect_electrical_series", null ],
      [ "Integrating LabMetaData Extension with AqNWB", "user_demos.html#demos_labmetadata_extension", null ],
      [ "Remote Read Benchmark", "user_demos.html#demos_remote_read_benchmark", null ]
    ] ]
];