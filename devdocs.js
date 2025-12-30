var devdocs =
[
    [ "Installation 🛠️", "dev_install_page.html", [
      [ "Installing AqNWB", "dev_install_page.html#dev_install_aqnwb_sec", [
        [ "Requirements", "dev_install_page.html#dev_requirements_sec", null ],
        [ "Developer Build", "dev_install_page.html#devbuild_sec", [
          [ "Developer Presets", "dev_install_page.html#devbuild_presets_subsec", null ],
          [ "Configure, Build and Test", "dev_install_page.html#configure_build_test", null ],
          [ "Developer Install", "dev_install_page.html#devbuild_install_subsec", null ],
          [ "Developer Mode Targets", "dev_install_page.html#devbuild_dev_mode_targets_subsec", [
            [ "Target options", "dev_install_page.html#devbuild_target_options_subsubsec", null ]
          ] ]
        ] ]
      ] ],
      [ "Installing Python Utilities", "dev_install_page.html#dev_install_utils_sec", null ]
    ] ],
    [ "Testing 🛡️", "testing.html", [
      [ "Unit Tests", "testing.html#testing_unit", [
        [ "Running Unit Tests", "testing.html#testing_unit_run", null ],
        [ "Automated tests on GitHub", "testing.html#testing_github", null ]
      ] ],
      [ "Spellcheck", "testing.html#testing_spellcheck", null ],
      [ "Linting", "testing.html#testing_lint", null ]
    ] ],
    [ "Documentation 📚", "dev_docs_page.html", [
      [ "Building the Docs", "dev_docs_page.html#dev_docs_requirements_sec", null ],
      [ "Creating New Documentation Pages", "dev_docs_page.html#dev_docs_creating", null ],
      [ "Creating Code Examples", "dev_docs_page.html#dev_docs_codeexamples_sec", [
        [ "Creating the Example Code", "dev_docs_page.html#dev_docs_codeexamples_def_sec", null ],
        [ "Testing the Example Code", "dev_docs_page.html#dev_docs_codeexamples_run_sec", null ],
        [ "Including Code Examples in Doxygen", "dev_docs_page.html#dev_docs_codeexamples_incl_sec", null ]
      ] ],
      [ "Creating Custom Dot Graphs", "dev_docs_page.html#dev_docs_graphviz_sec", null ]
    ] ],
    [ "NWB Schema 🧠", "nwb_schema_page.html", [
      [ "Generating the schema header files", "nwb_schema_page.html#dev_docs_generating_nwb_schema_headers_section", null ],
      [ "Updating the schema", "nwb_schema_page.html#dev_docs_updating_nwb_schema_section", null ]
    ] ],
    [ "Implementing a Registered Type 🔧", "registered_type_page.html", [
      [ "How to Implement a RegisteredType", "registered_type_page.html#implement_registered_type", [
        [ "Example: Implementing a New Type", "registered_type_page.html#implement_registered_type_example", null ],
        [ "DEFINE_ATTRIBUTE_FIELD: Creating read methods for attributes", "registered_type_page.html#use_the_define_attribute_field_macro", null ],
        [ "DEFINE_DATASET_FIELD: Creating read and write methods for datasets", "registered_type_page.html#use_the_define_dataset_field_macro", null ],
        [ "DEFINE_REGISTERED_FIELD: Defining read methods for neurodata_type objects", "registered_type_page.html#use_the_define_registered_field_macro", null ],
        [ "DEFINE_UNNAMED_REGISTERED_FIELD: Defining read methods for unnamed neurodata_type objects", "registered_type_page.html#use_the_define_unnamed_registered_field_macro", null ],
        [ "DEFINE_REFERENCED_REGISTERED_FIELD: Defining read methods for references to neurodata_type objects", "registered_type_page.html#use_the_define_referenced_registered_field_macro", null ]
      ] ],
      [ "How to implement a RegisteredType with a custom type name", "registered_type_page.html#using_registered_subclass_with_typename", [
        [ "Templated RegisteredType Classes", "registered_type_page.html#implement_templated_registered_type", [
          [ "Using a base class and templated child class", "registered_type_page.html#implement_templated_registered_type_two_class", null ],
          [ "Using a single templated class", "registered_type_page.html#implement_templated_registered_type_single_class", null ]
        ] ],
        [ "Limitations of REGISTER_SUBCLASS_WITH_TYPENAME", "registered_type_page.html#limitations_registered_subclass_with_typename", null ]
      ] ],
      [ "Using the generate-types command", "registered_type_page.html#using_schematype_to_aqnwb", null ],
      [ "Testing RegisteredTypes", "registered_type_page.html#implement_registered_type_unit_tests", null ]
    ] ],
    [ "Integrating NWB Extensions 🧩", "integrating_extensions_page.html", [
      [ "How to Integrate a New Namespace", "integrating_extensions_page.html#integrate_namespace", null ],
      [ "How the NamespaceRegistry Works", "integrating_extensions_page.html#namespace_registry", [
        [ "Registering a Namespace", "integrating_extensions_page.html#register_namespace", null ],
        [ "Looking up Namespaces", "integrating_extensions_page.html#namespace_lookup", null ]
      ] ],
      [ "LabMetaData Extension Demo", "integrating_extensions_page.html#labmetadata_extension_demo", [
        [ "Step 1: Get the Schema of the Extension", "integrating_extensions_page.html#labmetadata_extension_schema", null ],
        [ "Step 2: Convert the Schema to C++", "integrating_extensions_page.html#labmetadata_extension_cpp_generation", null ],
        [ "Step 3: Create RegisteredType Classes for all neurodata_types", "integrating_extensions_page.html#labmetadata_extension_registered_type", null ],
        [ "Step 4: Using the Extension", "integrating_extensions_page.html#labmetadata_extension_usage", null ]
      ] ]
    ] ],
    [ "Implementation of Data Read 📤", "read_design_page.html", [
      [ "Reading datasets and attributes", "read_design_page.html#read_design_sec_read_date", null ],
      [ "Reading neurodata_type objects", "read_design_page.html#reading_neurodata_type_objects", [
        [ "How the Type Registry in RegisteredType Works", "read_design_page.html#type_registry", null ],
        [ "How to Use the RegisteredType Registry", "read_design_page.html#use_registered_type_registry", null ],
        [ "Reading templated RegisteredType classes", "read_design_page.html#read_design_templates", null ],
        [ "Example: Using the type registry", "read_design_page.html#use_registered_type_registry_example", null ]
      ] ],
      [ "Example Implementation Details", "read_design_page.html#read_design_example", [
        [ "Creating and Writing Data", "read_design_page.html#read_design_example_create", null ],
        [ "Reading and Processing Data", "read_design_page.html#read_design_example_read", null ]
      ] ]
    ] ],
    [ "Implementation of Data Recording 📊", "record_design_page.html", [
      [ "Recording datasets with BaseRecordingData", "record_design_page.html#record_design_sec_recording_data", [
        [ "The DEFINE_DATASET_FIELD Macro for Recording", "record_design_page.html#record_design_sec_define_dataset_field", null ],
        [ "BaseRecordingData for Managing Recording", "record_design_page.html#record_design_sec_baserecordingdata", null ]
      ] ],
      [ "TimeSeries Convenience Methods for Consistent Recording", "record_design_page.html#record_design_sec_timeseries", null ],
      [ "RecordingObjects for Managing Collections", "record_design_page.html#record_design_sec_recording_containers", null ],
      [ "NWB I/O convenience utilities", "record_design_page.html#record_design_nwbio_utils", null ],
      [ "Object and memory management", "record_design_page.html#record_design_object_memory_management", null ],
      [ "Further Reading", "record_design_page.html#recording_design_further_reading", null ]
    ] ],
    [ "Legal ⚖️", "legal_page.html", [
      [ "Code of Conduct", "legal_page.html#legal_code_of_conduct", null ],
      [ "License", "legal_page.html#legal_license", null ],
      [ "Copyright", "legal_page.html#legal_copyright", null ]
    ] ],
    [ "Changelog ⚖️", "changelog_page.html", null ]
];