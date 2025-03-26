var devdocs =
[
    [ "Installation & Setup", "dev_install_page.html", [
      [ "Requirements", "dev_install_page.html#dev_requirements_sec", null ],
      [ "Developer Build", "dev_install_page.html#devbuild_sec", null ],
      [ "Developer Presets", "dev_install_page.html#devbuild_presets_subsec", [
        [ "Configure, Build and Test", "dev_install_page.html#configure_build_test", null ]
      ] ],
      [ "Developer Mode Targets", "dev_install_page.html#devbuild_dev_mode_targets_subsec", [
        [ "Target options", "dev_install_page.html#devbuild_target_options_subsubsec", null ]
      ] ]
    ] ],
    [ "Testing", "testing.html", [
      [ "Unit Tests", "testing.html#testing_unit", [
        [ "Running Unit Tests", "testing.html#testing_unit_run", null ],
        [ "Automated tests on GitHub", "testing.html#testing_github", null ]
      ] ],
      [ "Spellcheck", "testing.html#testing_spellcheck", null ],
      [ "Linting", "testing.html#testing_lint", null ]
    ] ],
    [ "Documentation", "dev_docs_page.html", [
      [ "Building the Docs", "dev_docs_page.html#dev_docs_requirements_sec", null ],
      [ "Creating New Documentation Pages", "dev_docs_page.html#dev_docs_creating", null ],
      [ "Creating Code Examples", "dev_docs_page.html#dev_docs_codeexamples_sec", [
        [ "Creating the Example Code", "dev_docs_page.html#dev_docs_codeexamples_def_sec", null ],
        [ "Testing the Example Code", "dev_docs_page.html#dev_docs_codeexamples_run_sec", null ],
        [ "Including Code Examples in Doxygen", "dev_docs_page.html#dev_docs_codeexamples_incl_sec", null ]
      ] ],
      [ "Creating Custom Dot Graphs", "dev_docs_page.html#dev_docs_graphviz_sec", null ]
    ] ],
    [ "NWB Schema", "nwb_schema_page.html", [
      [ "Generating the schema header files", "nwb_schema_page.html#dev_docs_generating_nwb_schema_headers_section", null ],
      [ "Updating the schema", "nwb_schema_page.html#dev_docs_updating_nwb_schema_section", null ]
    ] ],
    [ "Implementing a new Neurodata Type", "registered_type_page.html", [
      [ "How to Implement a RegisteredType", "registered_type_page.html#implement_registered_type", [
        [ "Example: Implementing a new type", "registered_type_page.html#implement_registered_type_example", null ],
        [ "DEFINE_FIELD: Creating read methods for datasets and attributes", "registered_type_page.html#use_the_define_field_macro", null ],
        [ "DEFINE_REGISTERED_FIELD: Defining read methods for neurodata_type objects", "registered_type_page.html#use_the_define_registered_field_macro", null ],
        [ "DEFINE_REFERENCED_REGISTERED_FIELD: Defining read methods for references to neurodata_type objects", "registered_type_page.html#use_the_define_referenced_registered_field_macro", null ]
      ] ],
      [ "How to implement a RegisteredType with a custom type name", "registered_type_page.html#using_registered_subclass_with_typename", [
        [ "Templated RegisteredType Classes", "registered_type_page.html#implement_templated_registered_type", [
          [ "Using a base class and templated child class", "registered_type_page.html#implement_templated_registered_type_two_class", null ],
          [ "Using a single templated class", "registered_type_page.html#implement_templated_registered_type_single_class", null ]
        ] ],
        [ "Limitations of REGISTER_SUBCLASS_WITH_TYPENAME", "registered_type_page.html#limitations_registered_subclass_with_typename", null ]
      ] ],
      [ "Testing RegisteredTypes", "registered_type_page.html#implement_registered_type_unit_tests", null ]
    ] ],
    [ "Implementation of data read", "read_design_page.html", [
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
    [ "Code of Conduct", "code_of_conduct_page.html", null ],
    [ "License", "license_page.html", null ],
    [ "Copyright", "copyright_page.html", null ]
];