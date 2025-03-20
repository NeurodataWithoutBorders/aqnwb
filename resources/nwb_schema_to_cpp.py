#!/usr/bin/env python3
"""
Script to generate C++ code from NWB schema files.

This script takes a NWB schema file (JSON or YAML) as input and generates C++ code
for interacting with the neurodata types defined in the schema using AqNWB.
"""

import argparse
import json
import os
import re
import sys
import yaml
from typing import Dict, List, Any, Optional, Tuple, Set
from hdmf.spec import GroupSpec, DatasetSpec, AttributeSpec, SpecNamespace, SpecCatalog
from hdmf.spec.write import YAMLSpecWriter
from hdmf.spec.namespace import NamespaceCatalog
from hdmf.spec.spec import Spec
from pynwb import NWBHDF5IO, NWBFile
from pynwb.spec import NWBNamespaceBuilder
from pynwb import get_manager, get_type_map
from pynwb.spec import NWBNamespaceBuilder


def snake_to_camel(name: str) -> str:
    """Convert snake_case to CamelCase."""
    if name is not None:
        return ''.join(word.title() for word in re.split('[_-]', name))
    else:
        return None


def get_cpp_type(dtype: str) -> str:
    """Convert NWB data type to C++ type."""
    type_mapping = {
        'text': 'std::string',
        'float': 'float',
        'float32': 'float',
        'double': 'double',
        'float64': 'double',
        'int': 'int32_t',
        'int8': 'int8_t',
        'int16': 'int16_t',
        'int32': 'int32_t',
        'int64': 'int64_t',
        'uint': 'uint32_t',
        'uint8': 'uint8_t',
        'uint16': 'uint16_t',
        'uint32': 'uint32_t',
        'uint64': 'uint64_t',
        'bool': 'bool',
        'isodatetime': 'std::string',  # Assuming isodatetime is stored as string
    }
    # Undefined dtype
    if dtype is None:
        return "std::any"
    
    # Handle array types
    if isinstance(dtype, str) and dtype.startswith('array of '):
        base_type = dtype[len('array of '):]
        cpp_type = get_cpp_type(base_type)
        return f"std::vector<{cpp_type}>"
    
    # Handle compound types (dictionaries with name and dtype)
    if isinstance(dtype, list):
        # This is a compound type, return a generic type
        return 'std::any'
    
    # Handle reference types
    if isinstance(dtype, dict) and 'reftype' in dtype:
        if dtype['reftype'] == 'object':
            target_type = dtype.get('target_type', 'RegisteredType')
            return f"std::shared_ptr<{target_type}>"
    
    return type_mapping.get(dtype, 'std::string')  # Default to string for unknown types


def parse_schema_file(file_path: str) -> Tuple[SpecNamespace, Dict[str, Spec]]:
    """Parse a schema file and return the namespace and data types using PyNWB."""
    # Find the namespace file
    namespace_path = file_path
    if not (namespace_path.endswith('.namespace.yaml') or namespace_path.endswith('.namespace.json')):
        # Try to find the namespace file
        if os.path.isdir(file_path):
            # If a directory is provided, look for namespace files in it
            for filename in os.listdir(file_path):
                if filename.endswith('.namespace.yaml') or filename.endswith('.namespace.json'):
                    namespace_path = os.path.join(file_path, filename)
                    break
        else:
            # If a schema file is provided, try to find the corresponding namespace file
            dir_path = os.path.dirname(file_path)
            base_name = os.path.basename(file_path).split('.')[0]
            for ext in ['.namespace.yaml', '.namespace.json']:
                potential_path = os.path.join(dir_path, f"{base_name}{ext}")
                if os.path.exists(potential_path):
                    namespace_path = potential_path
                    break
    
    # Load the namespace data to get the namespace name
    if namespace_path.endswith('.json'):
        with open(namespace_path, 'r') as f:
            namespace_data = json.load(f)
    else:  # Assume YAML
        with open(namespace_path, 'r') as f:
            namespace_data = yaml.safe_load(f)
    
    namespace_name = namespace_data['namespaces'][0]['name']
    
    type_map = get_type_map()
    type_map.load_namespaces(namespace_path)
    namespace = type_map.namespace_catalog.get_namespace(namespace_name)
    
    # Get all the neurodata types in the namespace
    neurodata_types = {}
    # Get all specs from the catalog
    for spec_name in namespace.catalog.get_registered_types():
        spec = namespace.catalog.get_spec(spec_name)
        if hasattr(spec, 'neurodata_type_def') and spec.neurodata_type_def is not None:
            neurodata_types[spec.neurodata_type_def] = spec
    
    return namespace, neurodata_types


def collect_neurodata_types(namespace: SpecNamespace) -> Dict[str, Spec]:
    """Collect all neurodata types from the namespace."""
    neurodata_types = {}
    
    for spec_name in namespace.catalog.get_spec_names():
        spec = namespace.catalog.get_spec(spec_name)
        if hasattr(spec, 'neurodata_type_def') and spec.neurodata_type_def is not None:
            neurodata_types[spec.neurodata_type_def] = spec
    
    return neurodata_types


def get_attributes(spec: Spec) -> Dict[str, Dict[str, Any]]:
    """Extract attributes from a spec object."""
    attributes = {}
    
    # Get attributes directly from the spec
    print(type(spec))
    if hasattr(spec, 'attributes'):
        for attr_spec in spec.attributes:
            attributes[attr_spec.name] = {
                'dtype': attr_spec.dtype,
                'doc': attr_spec.doc
            }
    
    return attributes

def get_datasets(spec: Spec) -> Dict[str, Dict[str, Any]]:
    """Extract datasets from a spec object."""
    datasets = {}
    
    # For GroupSpec, get datasets from the spec
    if isinstance(spec, GroupSpec) and hasattr(spec, 'datasets'):
        for dataset_spec in spec.datasets:
            datasets[dataset_spec.name] = {
                'dtype': dataset_spec.dtype,
                'doc': dataset_spec.doc
            }
            if hasattr(dataset_spec, 'neurodata_type_inc'):
                datasets[dataset_spec.name]['neurodata_type_inc'] = dataset_spec.neurodata_type_inc
    
    return datasets

def get_groups(spec: Spec) -> Dict[str, Dict[str, Any]]:
    """Extract groups from a spec object."""
    groups = {}
    
    # For GroupSpec, get groups from the spec
    if isinstance(spec, GroupSpec) and hasattr(spec, 'groups'):
        for group_spec in spec.groups:
            groups[group_spec.name] = {
                'doc': group_spec.doc
            }
            if hasattr(group_spec, 'neurodata_type_inc'):
                groups[group_spec.name]['neurodata_type_inc'] = group_spec.neurodata_type_inc
    
    return groups

def generate_header_file(namespace: SpecNamespace, neurodata_type: Spec, 
                         all_types: Dict[str, Spec]) -> str:
    """Generate C++ header file for a neurodata type."""
    namespace_name = namespace.name
    cpp_namespace_name = namespace_name.upper().replace("-", "_")
    type_name = neurodata_type.neurodata_type_def
    class_name = type_name
    
    # Get documentation
    doc = getattr(neurodata_type, 'doc', 'No documentation provided')
    
    # Determine parent class
    parent_type = getattr(neurodata_type, 'neurodata_type_inc', None)
    if parent_type:
        parent_class = parent_type
        # Check if parent is from a different namespace
        if '/' in parent_type:
            parent_namespace, parent_type = parent_type.split('/')
            parent_class = f"{parent_namespace}::{parent_type}"
    else:
        # Default parent class based on neurodata_type
        if isinstance(neurodata_type, DatasetSpec):
            parent_class = "AQNWB::NWB::Data"
        else:
            parent_class = "AQNWB::NWB::Container"
    
    # Start building the header file
    header = f"""#pragma once

#include <memory>
#include <string>
#include <vector>
#include "nwb/RegisteredType.hpp"
#include "io/ReasIO.hpp"
#include "io/BaseIO.hpp"
"""

    # Add additional includes based on parent class
    if parent_type:
        if parent_type == 'data':
            header += "#include \"nwb/hdmf/base/Data.hpp\"\n"
        elif '/' in parent_type:
            # External namespace, need to include appropriate header
            pass
        else:
            # Same namespace, include the parent header
            header += f"#include \"{parent_type}.hpp\"\n"
    else:
        if "Data" in parent_class:
            header += "#include \"nwb/hdmf/base/Data.hpp\"\n"
        else:
            header += "#include \"nwb/hdmf/base/Container.hpp\"\n"
    
    # Include the namespace header
    schema_header = namespace_name.replace("-", "_") + ".hpp"
    header += f"#include \"spec/{schema_header}\"\n"
    header += "using namespace AQNWB::IO;\n"
    
    header += f"""
namespace {cpp_namespace_name} {{

/**
 * @brief {neurodata_type.get('doc', 'No documentation provided')}
 */
class {class_name} : public {parent_class}
{{
public:
    /**
     * @brief Constructor
     * @param path Path to the object in the file
     * @param io IO object for reading/writing
     */
    {class_name}(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io);
    
    /**
     * @brief Initialize the object
     */
    void initialize();
"""

    # Get attributes, datasets, and groups
    attributes = get_attributes(neurodata_type)
    datasets = get_datasets(neurodata_type)
    groups = get_groups(neurodata_type)
    
    # Add DEFINE_FIELD and DEFINE_REGISTERED_FIELD macros
    header += "\n"
    header += "    // Define read methods\n"
    header += "    // TODO: Check all macro definiton details\n"

    # Add fields for attributes
    for attr_name, attr in attributes.items():
        doc = attr.get('doc', 'No documentation provided').replace('\n', ' ')
        header += f"    DEFINE_FIELD(read{snake_to_camel(attr_name)}, AttributeField, {get_cpp_type(attr.get('dtype', None))}, \"{attr_name}\", {doc})\n"
    
    # Add fields for datasets
    for dataset_name, dataset in datasets.items():
        doc = dataset.get('doc', 'No documentation provided').replace('\n', ' ')
        if 'neurodata_type_inc' in dataset:
            header += f"    DEFINE_REGISTERED_FIELD(read{snake_to_camel(dataset_name)}, {dataset['neurodata_type_inc']}, \"{dataset_name}\", {doc})\n"
        else:
            header += f"    DEFINE_FIELD(read{snake_to_camel(dataset_name)}, DatasetField, {get_cpp_type(dataset.get('dtype', None))}, \"{dataset_name}\", {doc})\n"
            # TODO: Need to also add DEFINE_FIELD for all attributes of the dataset

    # Add fields for groups
    for group_name, group in groups.items():
        doc = group.get('doc', 'No documentation provided').replace('\n', ' ')
        if 'neurodata_type_inc' in group:
            header += f"    DEFINE_REGISTERED_FIELD(read{snake_to_camel(group_name)}, {group['neurodata_type_inc']}, \"{group_name}\", {doc})\n"
        else:
            pass  # Groups without a type are just containers for named fields that should be exposed directly 
            # TODO Need to recursively expose attributes and datasets in the untyped subgroup
    
    # Add REGISTER_SUBCLASS macro
    header += f"""
    REGISTER_SUBCLASS({class_name}, AQNWB::SPEC::{cpp_namespace_name}::namespaceName)
}};

}} // namespace {cpp_namespace_name}
"""
    
    return header


def generate_implementation_file(namespace: SpecNamespace, neurodata_type: Spec, 
                                all_types: Dict[str, Spec]) -> str:
    """Generate C++ implementation file for a neurodata type."""
    namespace_name = namespace.name
    cpp_namespace_name = snake_to_camel(namespace_name)
    type_name = neurodata_type.neurodata_type_def
    class_name = type_name
    
    # Determine parent class
    parent_type = getattr(neurodata_type, 'neurodata_type_inc', None)
    if parent_type:
        parent_class = parent_type
        # Check if parent is from a different namespace
        if '/' in parent_type:
            parent_namespace, parent_type = parent_type.split('/')
            parent_class = f"{parent_namespace}::{parent_type}"
    else:
        # Default parent class based on neurodata_type
        if isinstance(neurodata_type, DatasetSpec):
            parent_class = "AQNWB::NWB::Data"
        else:
            parent_class = "AQNWB::NWB::Container"
    
    # Start building the implementation file
    impl = f"""#include "{class_name}.hpp"
#include "Utils.hpp"

using namespace {cpp_namespace_name};
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL({class_name})

/**
 * @brief Constructor
 */
{class_name}::{class_name}(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
    : {parent_class}(path, io)
{{
}}

/**
 * @brief Initialize the object
 */
void {class_name}::initialize()
{{
    // Call parent initialize method
    {parent_class}::initialize();
    
    // Initialize attributes
"""
    
    # Get attributes, datasets, and groups
    attributes = get_attributes(neurodata_type)
    datasets = get_datasets(neurodata_type)
    groups = get_groups(neurodata_type)
    
    # Add initialization for attributes
    for attr_name, attr in attributes.items():
        impl += f"    // TODO: Initialize {attr_name} attribute\n"
    
    # Add initialization for datasets
    for dataset_name, dataset in datasets.items():
        impl += f"    // TODO: Initialize {dataset_name} dataset\n"
    
    # Add initialization for groups
    for group_name, group in groups.items():
        impl += f"    // TODO: Initialize {group_name} group\n"
    
    impl += "}\n\n"
     
    return impl


def main():
    """Main function to parse arguments and generate code."""
    parser = argparse.ArgumentParser(description='Generate C++ code from NWB schema files.')
    parser.add_argument('schema_file', help='Path to the schema file (JSON or YAML)')
    parser.add_argument('output_dir', help='Directory to output the generated code')
    parser.add_argument('--namespace', help='Namespace to use (overrides the one in the schema)')
    
    args = parser.parse_args()
    
    # Parse the schema file
    namespace, neurodata_types = parse_schema_file(args.schema_file)
    
    # Override namespace if provided
    if args.namespace:
        # Create a new namespace with the provided name
        namespace.name = args.namespace
    
    # Create output directory if it doesn't exist
    os.makedirs(args.output_dir, exist_ok=True)
    
    # Generate code for each neurodata type
    for type_name, neurodata_type in neurodata_types.items():
        class_name = type_name
        
        # Generate header file
        header_file = generate_header_file(namespace, neurodata_type, neurodata_types)
        header_path = os.path.join(args.output_dir, f"{class_name}.hpp")
        with open(header_path, 'w') as f:
            f.write(header_file)
        
        # Generate implementation file
        impl_file = generate_implementation_file(namespace, neurodata_type, neurodata_types)
        impl_path = os.path.join(args.output_dir, f"{class_name}.cpp")
        with open(impl_path, 'w') as f:
            f.write(impl_file)
        
        print(f"Generated {header_path} and {impl_path}")


if __name__ == "__main__":
    main()
