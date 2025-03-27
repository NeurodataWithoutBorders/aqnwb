#!/usr/bin/env python3
"""
Script to generate C++ code from NWB schema files.

This script takes a NWB schema file (JSON or YAML) as input and generates C++ code
for interacting with the neurodata types defined in the schema using AqNWB.
"""

import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

import argparse
import json
import os
import re
from ruamel.yaml import YAML
from typing import Dict, List, Tuple
from hdmf.spec import DatasetSpec, SpecNamespace
from hdmf.spec.spec import Spec
from pynwb import get_type_map


def render_define_registered_field(
    field_name: str,
    neurodata_type: str,
    doc: str,
    is_inherited: bool = False,
    is_overridden: bool = False,
) -> str:
    """
    Return string for DEFINE_REGISTERED_FIELD macro.

    Parameters:
    field_name (str): Name of the field.
    neurodata_type (str): The neurodata type of the field.
    doc (str): Documentation string for the field.
    is_inherited (bool): Indicates whether the field is inherited from the parent
    is_overridden (bool): Indicates wheterh the field overrides a field from the parent

    Returns:
    str: A string representing the DEFINE_REGISTERED_FIELD macro.
    """
    doc_string = doc.replace('"', "").replace("'", "")
    unmodified_from_parent = is_inherited and not is_overridden
    re = "" if unmodified_from_parent else "    /*\n"  # Comment out if inherited and not overridden
    if is_inherited:
        re += f"    // name={field_name}, neurodata_type={neurodata_type}: This field is_inheritted={is_inherited}, is_overridden={is_overridden} from the parent neurodata_type\n"
    if field_name is not None:
        re += "    DEFINE_REGISTERED_FIELD(\n"
        re += f"        read{snake_to_camel(field_name)},\n"
        re += f"        {neurodata_type},\n"
        re += f'        "{field_name}",\n'
        re += f"        {doc_string})\n"
        re += "" if unmodified_from_parent else "    */\n"
    else:
        re += f"""    // TODO: Update or remove as appropriate (e.g., fix namespace of return type)
    /**
    * @brief Read an arbitrary {neurodata_type} object owned by this object
    *
    * For {neurodata_type} objects defined in the schema with a fixed name
    * the corresponding DEFINE_REGISTERED_FIELD read functions are preferred
    * because they help avoid the need for specifying the specific name
    * and data type to use.
    *
    * @return The {neurodata_type} object representing the object or a nullptr if the
    * object doesn't exist
    */\n"""
    re += "    /*\n" if unmodified_from_parent else ""
    re += f"""    std::shared_ptr<{neurodata_type}> read{neurodata_type}(const std::string& objectName)
    {{
        std::string objectPath = AQNWB::mergePaths(m_path, objectName);
        if (m_io->objectExists(objectPath)) {{
        return std::make_shared<{neurodata_type}>(objectPath, m_io);
        }}
        return nullptr;
    }}
"""
    re += "    */\n" if unmodified_from_parent else ""
    
    return re


def render_define_field(
    field_name: str,
    field_type: str,
    dtype: str,
    doc: str,
    field_parent: str = "",
    is_inherited: bool = False,
    is_overridden: bool = False,
) -> str:
    """
    Return string for DEFINE_FIELD macro.

    Parameters:
    field_name (str): Name of the field to use for generating the function name
    field_type (str): One of DatasetField or AttributeField.
    dtype (str): C++ data type to use by default for read.
    doc (str): Documentation string to use for the field.
    field_parent: The name of the parent object
    is_inherited (bool): Indicates whether the field is inherited from the parent
    is_overridden (bool): Indicates wheterh the field overrides a field from the parent

    Returns:
    str: A string representing the DEFINE_FIELD macro.
    """
    field_path = field_name if field_parent == "" else (field_parent + "/" + field_name)
    field_full_name = f"{snake_to_camel(field_parent)}{snake_to_camel(field_name)}"
    doc_string = doc.replace('"', "").replace("'", "")
    re = ""
    if is_inherited and not is_overridden:
        re += "    /*\n"
        re += f"    // {field_name} inherited from parent neurodata_type\n"
    if is_overridden:
        re += f"    // {field_name} overrides inherited field from parent neurodata_type\n"

    re += "    DEFINE_FIELD(\n"
    re += f"        read{field_full_name},\n"
    re += f"        {field_type},\n"
    re += f"        {dtype},\n"
    re += f'        "{field_path}",\n'
    re += f"        {doc_string})\n"
    if is_inherited and not is_overridden:
        re += "    */\n"
    return re


def render_initalize_method(
    class_name: str,
    parent_class_name: str,
    neurodata_type: Spec,
) -> Tuple[str, str]:
    """
    Render the initalize method for a neurodata_type

    Parameters:
    class_name (str): Name of the class
    parent_class_name (str): Name of the parent class
    neurodata_type (Spec): Spec of the neurodata_type to render

    Returns:
    Tuple(str, str) : The first string is the signature for the header
    and the second string is the implementation for the cpp file.
    """

    # Define helper functions for rendering the initialize method
    # TODO: The following helper function only support at most one layer of nesting via the parent
    def spec_to_func_param(obj: Spec, parent: Spec = None) -> str:
        """
        Internal helper function to create a paramter definition for the initialize method

        Parameter:
        obj (Spec) : The spec dataset or attribute spec to render
        parent (Spec) : Optional parent spec
        """
        obj_cpp_type = get_cpp_type(obj.dtype)
        field_full_name = f"{obj.name}"
        if parent is not None:
            field_full_name += f"{snake_to_camel(parent.name)}"
        if obj.shape is None:
            if obj_cpp_type == "std::string":
                return f"\n       const {obj_cpp_type}& {field_full_name},"
            else:
                return f"\n       {obj_cpp_type} {field_full_name},"
        else:
            return f"\n       const std::vector<{obj_cpp_type}>& {field_full_name},"

    def attr_init(
        attr: Spec, is_inherited: bool, is_overridden: bool, parent: Spec = None
    ) -> str:
        """
        Internal helper function to create suggested initializion code for an attribute

        Parameters:
        attr (Spec): The attribute to render
        is_inherited (bool): Is this an inherited attribute
        is_overriden (bool): Is this an overrriden attribute
        parent: (Spec): Optional parent spec

        """
        re = f"    // TODO: Initialize {attr.name} attribute"
        if is_inherited or is_overridden:
            re += f" This attribute is_inheritted={is_inherited}, is_overridden={is_overridden}"
        re += "\n"
        # add example initializtion hints
        attr_cpp_type = get_cpp_type(attr.dtype)
        attr_base_type = get_basedata_type(attr.dtype)
        parent_path = (
            "m_path" if parent is None else f"AQNWB::mergePaths(m_path, {parent.name})"
        )
        if attr_cpp_type == "std::string":
            re += f'    // m_io->createAttribute({attr.name}, {parent_path}, "{attr.name}");\n\n'
        elif attr.shape is None:
            re += f'    // m_io->createAttribute({attr_base_type}, &{attr.name}, {parent_path}, "{attr.name}");\n\n'
        return re

    def dataset_init(dataset: Spec, is_inherited: bool, is_overridden: bool) -> str:
        """
        Internal helper function to create suggested initializion code for a dataset

        Parameters:
        dataset (Spec): The dataset to render
        is_inherited (bool): Is this an inherited attribute
        is_overriden (bool): Is this an overrriden attribute
        """
        re = f"    // TODO: Initialize {dataset.name} dataset"
        if is_inherited or is_overridden:
            re += f" This dataset is_inheritted={is_inherited}, is_overridden={is_overridden}"
        re += "\n"
        # add example initializtion hints
        dataset_cpp_type = get_cpp_type(dataset.dtype)
        dataset_base_type = get_basedata_type(dataset.dtype)
        if dataset_cpp_type == "std::string":
            re += f"    // auto {dataset.name}Path = AQNWB::mergePaths(m_path, {dataset.name});\n"
            re += f"    // m_io->createStringDataSet({dataset.name}Path, {dataset.name})\n"
        else:
            re += f"    // IO::ArrayDataSetConfig {dataset.name}Config({dataset_base_type}, <SHAPE>, <CHUNK_SIZE>);\n"
            re += f"    // std::unique_ptr<IO::BaseRecordingData>(m_io->createArrayDataSet({dataset.name}Config, path));\n"
        re += "\n"
        return re

    # Retrieve all the fields
    attributes = neurodata_type.get("attributes", [])
    datasets = neurodata_type.get("datasets", [])
    groups = neurodata_type.get("groups", [])

    funcSignature = "initialize("
    # Add initialization for attributes
    # TODO: This is missing recursion through field required to initialize groups owned by the type
    for obj in attributes + datasets:
        funcSignature += spec_to_func_param(obj=obj)
        if hasattr(obj, "attributes"):
            for attr in obj.attributes:
                funcSignature += spec_to_func_param(obj=attr, parent=obj)

    funcSignature = funcSignature.rstrip(",") + ")"

    headerSrc = f"""
    // TODO: Update the initialize method as appropriate.
    /**
     * @brief Initialize the object
     */
     void {funcSignature};
"""
    cppSrc = f"""
/**
 * @brief Initialize the object
 */
void {class_name}::{funcSignature}
{{
    // Call parent initialize method. 
    // TODO: Call the parents initialize method. This should typically also initialize inherited fields.
    // {parent_class_name}::initialize(...);
    
    // Initialize attributes
"""

    # Add initialization for attributes, datasets and groups
    for attr in attributes:
        cppSrc += attr_init(
            attr=attr,
            is_inherited=neurodata_type.is_inherited_spec(attr),
            is_overridden=neurodata_type.is_overridden_spec(attr),
        )
    cppSrc += "    // Initialize datasets\n"
    for dataset in datasets:
        cppSrc += dataset_init(
            dataset=dataset,
            is_inherited=neurodata_type.is_inherited_spec(dataset),
            is_overridden=neurodata_type.is_overridden_spec(dataset),
        )
        for attr in dataset.attributes:
            cppSrc += attr_init(
                attr=attr,
                is_inherited=neurodata_type.is_inherited_spec(attr),
                is_overridden=neurodata_type.is_overridden_spec(attr),
                parent=dataset,
            )
    cppSrc += "    // Initialize groups\n"
    for group in groups:
        cppSrc += f"    // TODO: Initialize {group.name} group\n"
        cppSrc += f'    // auto {group.name}Path = AQNWB::mergePaths(m_path, "{group.name}");\n'
        cppSrc += f"    // m_io->createGroup({group.name}Path);\n"
        cppSrc += "\n"
        cppSrc += (
            f"    // TODO: Initialize any fields that the {group.name} group owns\n"
        )
    cppSrc += "}\n\n"

    return headerSrc, cppSrc


def snake_to_camel(name: str) -> str:
    """
    Convert snake_case to CamelCase.

    Parameters:
    name (str): The snake_case string to convert.

    Returns:
    str: The converted CamelCase string.
    """
    if name is not None:
        return "".join(word.title() for word in re.split("[_-]", name))
    else:
        return None


def get_basedata_type(dtype: str) -> str:
    """
    Convet NWB data type to AQNWB BaseDataType.

    Parameters:
    dtype (str): The NWB data type.

    Returns:
    str: The corresponding AqNWB BaseDataType.
    """
    type_mapping = {
        "text": "DSTR",
        "ascii": "DSTR",
        "utf": "DSTR",
        "utf8": "DSTR",
        "utf-8": "DSTR",
        "float": "F32",
        "float32": "F32",
        "double": "F64",
        "float64": "F64",
        "int": "I32",
        "int8": "I8",
        "int16": "I16",
        "int32": "I32",
        "int64": "I64",
        "uint": "U32",
        "uint8": "U8",
        "uint16": "U16",
        "uint32": "U32",
        "uint64": "U64",
        "bool": "I8",
        "isodatetime": "DSTR",
        "datatime": "DSTR",
    }
    # Reference type
    if isinstance(dtype, dict) and "reftype" in dtype:
        target_type = dtype.get("target_type", "Unknown")
        return f"<REF::{target_type}>"
    # Compound type
    elif isinstance(dtype, list):
        compound_type = ", ".join(dt["dtype"] for dt in dtype)
        return f"<COMPOUND({compound_type})>"
    else:
        return "AQNWB::IO::BaseDataType::" + type_mapping.get(dtype, "F32")


def get_cpp_type(dtype: str) -> str:
    """
    Convert NWB data type to C++ type.

    Parameters:
    dtype (str): The NWB data type.

    Returns:
    str: The corresponding C++ type.
    """
    type_mapping = {
        "text": "std::string",
        "ascii": "std::string",
        "utf": "std::string",
        "utf8": "std::string",
        "utf-8": "std::string",
        "float": "float",
        "float32": "float",
        "double": "double",
        "float64": "double",
        "int": "int32_t",
        "int8": "int8_t",
        "int16": "int16_t",
        "int32": "int32_t",
        "int64": "int64_t",
        "uint": "uint32_t",
        "uint8": "uint8_t",
        "uint16": "uint16_t",
        "uint32": "uint32_t",
        "uint64": "uint64_t",
        "bool": "bool",
        "isodatetime": "std::string",
        "datatime": "std::string",
    }
    # Undefined dtype
    if dtype is None:
        return "std::any"

    # Handle compound types (dictionaries with name and dtype)
    if isinstance(dtype, list):
        # This is a compound type, return a generic type
        return "std::any"

    # Handle reference types
    if isinstance(dtype, dict) and "reftype" in dtype:
        if dtype["reftype"] == "object":
            target_type = dtype.get("target_type", "RegisteredType")
            return f"std::shared_ptr<{target_type}>"

    return type_mapping.get(dtype, "std::string")  # Default to string for unknown types


def parse_schema_file(file_path: str) -> Tuple[SpecNamespace, Dict[str, Spec]]:
    """
    Parse a schema file and return the namespace and data types using PyNWB.

    Parameters:
    file_path (str): Path to the schema file.

    Returns:
    Tuple[SpecNamespace, Dict[str, Spec]]: The namespace and a dictionary of neurodata types.
    """
    # Find the namespace file
    namespace_path = file_path
    if not (
        namespace_path.endswith(".namespace.yaml")
        or namespace_path.endswith(".namespace.json")
    ):
        # Try to find the namespace file
        if os.path.isdir(file_path):
            # If a directory is provided, look for namespace files in it
            for filename in os.listdir(file_path):
                if filename.endswith(".namespace.yaml") or filename.endswith(
                    ".namespace.json"
                ):
                    namespace_path = os.path.join(file_path, filename)
                    break
        else:
            # If a schema file is provided, try to find the corresponding namespace file
            dir_path = os.path.dirname(file_path)
            base_name = os.path.basename(file_path).split(".")[0]
            for ext in [".namespace.yaml", ".namespace.json"]:
                potential_path = os.path.join(dir_path, f"{base_name}{ext}")
                if os.path.exists(potential_path):
                    namespace_path = potential_path
                    break

    # Load the namespace data to get the namespace name
    if namespace_path.endswith(".json"):
        with open(namespace_path, "r") as f:
            namespace_data = json.load(f)
    else:  # Assume YAML
        with open(namespace_path, "r") as f:
            yaml = YAML(typ="safe")
            namespace_data = yaml.load(f)

    namespace_name = namespace_data["namespaces"][0]["name"]

    type_map = get_type_map()
    type_map.load_namespaces(namespace_path)
    namespace = type_map.namespace_catalog.get_namespace(namespace_name)

    # Get all the neurodata types in the namespace
    neurodata_types = {}
    # Get all specs from the catalog
    for spec_name in namespace.catalog.get_registered_types():
        spec = namespace.catalog.get_spec(spec_name)
        if hasattr(spec, "neurodata_type_def") and spec.neurodata_type_def is not None:
            neurodata_types[spec.neurodata_type_def] = spec

    return namespace, neurodata_types


def collect_neurodata_types(namespace: SpecNamespace) -> Dict[str, Spec]:
    """
    Collect all neurodata types from the namespace.

    Parameters:
    namespace (SpecNamespace): The namespace object.

    Returns:
    Dict[str, Spec]: A dictionary of neurodata types.
    """
    neurodata_types = {}

    for spec_name in namespace.catalog.get_spec_names():
        spec = namespace.catalog.get_spec(spec_name)
        if spec.get("neurodata_type_def", None) is not None:
            neurodata_types[spec.neurodata_type_def] = spec

    return neurodata_types


def generate_header_file(
    namespace: SpecNamespace, neurodata_type: Spec, all_types: Dict[str, Spec]
) -> str:
    """
    Generate C++ header file for a neurodata type.

    Parameters:
    namespace (SpecNamespace): The namespace object.
    neurodata_type (Spec): The neurodata type spec.
    all_types (Dict[str, Spec]): A dictionary of all neurodata types.

    Returns:
    str: The generated C++ header file content.
    """
    namespace_name = namespace.name
    cpp_namespace_name = namespace_name.upper().replace("-", "_")
    type_name = neurodata_type.neurodata_type_def
    class_name = type_name

    # Get documentation
    doc = getattr(neurodata_type, "doc", "No documentation provided")

    # Determine parent class
    parent_type = getattr(neurodata_type, "neurodata_type_inc", None)
    if parent_type:
        parent_class = parent_type
        # Check if parent is from a different namespace
        if "/" in parent_type:
            parent_namespace, parent_type = parent_type.split("/")
            parent_class = f"{parent_namespace}::{parent_type}"
    else:
        # Default parent class based on neurodata_type
        if isinstance(neurodata_type, DatasetSpec):
            parent_class = "AQNWB::NWB::Data"
        else:
            parent_class = "AQNWB::NWB::Container"

    # Start building the header file
    header = """#pragma once

#include <memory>
#include <string>
#include <vector>
#include "nwb/RegisteredType.hpp"
#include "io/ReadIO.hpp"
#include "io/BaseIO.hpp"
"""

    # Add additional includes based on parent class
    header += "// TODO: Confirm the parent class type used and include path\n"
    if parent_type:
        if parent_type == "data":
            header += '#include "nwb/hdmf/base/Data.hpp"\n'
        elif "/" in parent_type:
            # External namespace, need to include appropriate header
            pass
        else:
            # Same namespace, include the parent header
            header += f'#include "{parent_type}.hpp"\n'
    else:
        if "Data" in parent_class:
            header += '#include "nwb/hdmf/base/Data.hpp"\n'
        else:
            header += '#include "nwb/hdmf/base/Container.hpp"\n'

    # Include the namespace header
    schema_header = namespace_name.replace("-", "_") + ".hpp"
    header += f'#include "spec/{schema_header}"\n'
    header += "using namespace AQNWB::IO;\n"

    # Get attributes, datasets, and groups
    header_initalize_src, _ = render_initalize_method(
        class_name=class_name,
        parent_class_name=parent_class,
        neurodata_type=neurodata_type,
    )
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
    {class_name}(
        const std::string& path,
        std::shared_ptr<AQNWB::IO::BaseIO> io);
    
    {header_initalize_src}
"""

    # Get attributes, datasets, and groups
    attributes = neurodata_type.get("attributes", [])
    datasets = neurodata_type.get("datasets", [])
    groups = neurodata_type.get("groups", [])

    # Add DEFINE_FIELD and DEFINE_REGISTERED_FIELD macros
    header += "\n"
    header += "    // Define read methods\n"
    header += "    // TODO: Check all macro definition details. E.g. fix paths, types, and check for duplicates inherited from parent\n"

    # Place DEFINE_FIELD , DEFINE_REGISTERED field definitions that are commented out because they are inherted at the end
    commented_fields = []

    def is_commented_field_def(input_field: str) -> bool:
        """
        Internal helper function ot check if a DEFINE_FIELD or DEFINE_REGISTERED_FIELD
        definition created via the render_define_field and render_registered_field
        methods has been commented out. This is used to be able to collect commented
        fields such that they can be added at the end rather than intermixed with other fields.
        """
        lines = input_field.split("\n")
        if len(lines) > 0:
            return input_field.replace(" ", "").startswith("/*")
        return False

    # Add fields for attributes
    for attr in attributes:
        attr_name = attr.name
        doc = attr.get("doc", "No documentation provided").replace("\n", " ")
        fieldDef = render_define_field(
            field_name=attr_name,
            field_type="AttributeField",
            dtype=get_cpp_type(attr.dtype),
            doc=doc,
            is_inherited=neurodata_type.is_inherited_spec(attr),
            is_overridden=neurodata_type.is_overridden_spec(attr),
        )
        if is_commented_field_def(fieldDef):
            commented_fields.append(fieldDef)
        else:
            header += fieldDef

    # Add fields for datasets
    for dataset in datasets:
        dataset_name = dataset.name
        doc = dataset.get("doc", "No documentation provided").replace("\n", " ")
        if dataset.get("neurodata_type_inc", None) is not None:
            fieldDef = render_define_registered_field(
                field_name=dataset_name,
                neurodata_type=dataset["neurodata_type_inc"],
                doc=doc,
                is_inherited=neurodata_type.is_inherited_spec(dataset),
                is_overridden=neurodata_type.is_overridden_spec(dataset),
            )
        else:
            fieldDef = render_define_field(
                field_name=dataset_name,
                field_type="DatasetField",
                dtype=get_cpp_type(dataset.dtype),
                doc=doc,
                is_inherited=neurodata_type.is_inherited_spec(dataset),
                is_overridden=neurodata_type.is_overridden_spec(dataset),
            )
        if is_commented_field_def(fieldDef):
            commented_fields.append(fieldDef)
        else:
            header += fieldDef

        for attr in dataset.get("attributes", []):
            doc = attr.get("doc", "No documentation provided").replace("\n", " ")
            fieldDef = render_define_field(
                field_name=attr.name,
                field_type="AttributeField",
                dtype=get_cpp_type(attr.dtype),
                doc=doc,
                field_parent=dataset_name,
                is_inherited=neurodata_type.is_inherited_spec(attr),
                is_overridden=neurodata_type.is_overridden_spec(attr),
            )
            if is_commented_field_def(fieldDef):
                commented_fields.append(fieldDef)
            else:
                header += fieldDef

    # Add fields for groups
    for group in groups:
        group_name = group.name
        doc = group.get("doc", "No documentation provided").replace("\n", " ")
        if group.get("neurodata_type_inc", None) is not None:
            fieldDef = render_define_registered_field(
                field_name=group_name,
                neurodata_type=group["neurodata_type_inc"],
                doc=doc,
                is_inherited=neurodata_type.is_inherited_spec(group),
                is_overridden=neurodata_type.is_overridden_spec(group),
            )
            if is_commented_field_def(fieldDef):
                commented_fields.append(fieldDef)
            else:
                header += fieldDef
        else:
            # TODO Groups without a type are just containers for fields that should be
            #      exposed directly via the parent neurodata_type. To do this we would need
            #      to recursibely go through all the contents of the current group
            #      and essentially repeat the whole process of adding read methods for
            #      datasets, attributes, and groups. For now we are just adding a note for
            #      the user to indicate that the fields for this group are missing.
            header += f'    // TODO Add missing read definitions for the contents of the untyped group "{group_name}"\n'
    if len(commented_fields) > 0:
        header += "    // TODO: The following fields have been commented because they should have been inherited from the parent class.\n"
        header += "    //       They are included here for your convenience so you can decide which fields may still need be defined here.\n"
        for fieldDef in commented_fields:
            header += fieldDef

    # Add REGISTER_SUBCLASS macro
    header += f"""
    REGISTER_SUBCLASS(
        {class_name},
        AQNWB::SPEC::{cpp_namespace_name}::namespaceName)
}};

}} // namespace {cpp_namespace_name}
"""

    return header


def generate_implementation_file(
    namespace: SpecNamespace, neurodata_type: Spec, all_types: Dict[str, Spec]
) -> str:
    """
    Generate C++ implementation file for a neurodata type.

    Parameters:
    namespace (SpecNamespace): The namespace object.
    neurodata_type (Spec): The neurodata type spec.
    all_types (Dict[str, Spec]): A dictionary of all neurodata types.

    Returns:
    str: The generated C++ implementation file content.
    """
    namespace_name = namespace.name
    cpp_namespace_name = "AQNWB::SPEC::" + namespace_name.upper().replace("-", "_")
    type_name = neurodata_type.neurodata_type_def
    class_name = type_name

    # Determine parent class
    parent_type = getattr(neurodata_type, "neurodata_type_inc", None)
    if parent_type:
        parent_class = parent_type
        # Check if parent is from a different namespace
        if "/" in parent_type:
            parent_namespace, parent_type = parent_type.split("/")
            parent_class = f"{parent_namespace}::{parent_type}"
    else:
        # Default parent class based on neurodata_type
        if isinstance(neurodata_type, DatasetSpec):
            parent_class = "AQNWB::NWB::Data"
        else:
            parent_class = "AQNWB::NWB::Container"

    # Get attributes, datasets, and groups
    _, cpp_initalize_src = render_initalize_method(
        class_name=class_name,
        parent_class_name=parent_class,
        neurodata_type=neurodata_type,
    )

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

{cpp_initalize_src}
"""

    return impl


def main() -> None:
    """
    Main function to parse arguments and generate code.

    This function sets up argument parsing, parses the schema file, optionally overrides the namespace,
    creates the output directory, and generates code for each neurodata type.

    Parameters:
    None

    Returns:
    None
    """
    parser = argparse.ArgumentParser(
        description="Generate C++ code from NWB schema files."
    )
    parser.add_argument(
        "schema_file", help="Path to the namespace schema file (JSON or YAML)"
    )
    parser.add_argument("output_dir", help="Directory to output the generated code")

    args = parser.parse_args()

    try:
        logger.info(f"Parsing schema file: {args.schema_file}")
        namespace, neurodata_types = parse_schema_file(args.schema_file)
        logger.info(f"Successfully parsed schema file: {args.schema_file}")
    except Exception as e:
        logger.error(f"Failed to parse schema file {args.schema_file}: {e}")
        return

    # Create output directory if it doesn't exist
    try:
        logger.info(f"Creating output directory: {args.output_dir}")
        os.makedirs(args.output_dir, exist_ok=True)
        logger.info(f"Successfully created output directory: {args.output_dir}")
    except Exception as e:
        logger.error(f"Failed to create output directory {args.output_dir}: {e}")
        return

    # Generate code for each neurodata type
    for type_name, neurodata_type in neurodata_types.items():
        class_name = type_name
        logger.info(f"Processing neurodata_type: {class_name}")
        try:
            header_file_name = f"{class_name}.hpp"
            logger.info(f"    Generating header file: {header_file_name}")
            header_file = generate_header_file(
                namespace, neurodata_type, neurodata_types
            )
            header_path = os.path.join(args.output_dir, header_file_name)
            with open(header_path, "w") as f:
                f.write(header_file)
        except Exception as e:
            logger.error(f"    Failed to generate header file {header_path}: {e}")

        try:
            cpp_file_name = f"{class_name}.cpp"
            logger.info(f"    Generating implementation file: {cpp_file_name}")
            impl_file = generate_implementation_file(
                namespace, neurodata_type, neurodata_types
            )
            impl_path = os.path.join(args.output_dir, cpp_file_name)
            with open(impl_path, "w") as f:
                f.write(impl_file)
        except Exception as e:
            logger.error(f"   Failed to generate implementation file {impl_path}: {e}")

    logger.info("Script execution completed.")


if __name__ == "__main__":
    main()
