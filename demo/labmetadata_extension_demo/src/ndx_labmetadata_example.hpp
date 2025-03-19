#pragma once

#include <vector>
#include <string>
#include <string_view>
#include "spec/NamespaceRegistry.hpp"

namespace AQNWB::SPEC::NDX_LABMETADATA_EXAMPLE
{

const std::string  namespaceName = "ndx-labmetadata-example";

const std::string version = "0.1.0";

constexpr std::string_view ndx_labmetadata_example_extensions = R"delimiter(
{"groups":[{"neurodata_type_def":"LabMetaDataExtensionExample","neurodata_type_inc":"LabMetaData","name":"custom_lab_metadata","doc":"Example extension type for storing lab metadata","datasets":[{"name":"tissue_preparation","dtype":"text","doc":"Lab-specific description of the preparation of the tissue","quantity":"?"}]}]})delimiter";

constexpr std::string_view namespaces = R"delimiter(
{"namespaces":[{"author":["Oliver Ruebel"],"contact":["oruebel@lbl.gov"],"doc":" Example extension to illustrate how to extend LabMetaData for adding lab-specific metadata","name":"ndx-labmetadata-example","schema":[{"namespace":"core","neurodata_types":["LabMetaData"]},{"source":"ndx-labmetadata-example.extensions.yaml"}],"version":"0.1.0"}]})delimiter";

const std::vector<std::pair<std::string_view, std::string_view>>
    specVariables {{
  {"ndx-labmetadata-example.extensions", ndx_labmetadata_example_extensions},
  {"namespace", namespaces}

}};

// Register this namespace with the global registry
REGISTER_NAMESPACE(namespaceName, version, specVariables)

}  // namespace AQNWB::SPEC::NDX_LABMETADATA_EXAMPLE
