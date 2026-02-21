#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.8"
# dependencies = ["hdmf", "pynwb", "ruamel.yaml"]
# ///
"""
Script to generate C++ code from NWB schema files.

This script takes a NWB schema file (JSON or YAML) as input and generates C++ code
for interacting with the neurodata types defined in the schema using AqNWB.
"""

import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

import argparse
import datetime
import json
import re
from pathlib import Path
from copy import deepcopy
from ruamel.yaml import YAML
from typing import Dict, List, Tuple
from hdmf.spec import DatasetSpec, GroupSpec, AttributeSpec, SpecNamespace
from hdmf.spec.spec import Spec
from pynwb import get_type_map


def render_define_registered_field(
    field_name: str,
    field_path: str,
    neurodata_type: str,
    referenced_namespace: str,
    doc: str,
    is_inherited: bool = False,
    is_overridden: bool = False
) -> str:
    """
    Return string for DEFINE_REGISTERED_FIELD macro.

    Parameters:
    field_name (str): The name of the field. Can be None for anonymous types.
    field_path (str): Path to the field.
    neurodata_type (str): The neurodata type of the field.
    referenced_namespace (str): The namespace of the referenced neurodata type.
    doc (str): Documentation string for the field.
    is_inherited (bool): Indicates whether the field is inherited from the parent
    is_overridden (bool): Indicates wheterh the field overrides a field from the parent

    Returns:
    str: A string representing the DEFINE_REGISTERED_FIELD macro.
    """
    # Clean up documentation string - remove quotes and commas that would break macro
    doc_string = doc.replace('"', "").replace("'", "").replace(",", " -")
    unmodified_from_parent = is_inherited and not is_overridden
    re = ""

    func_name = f"read{snake_to_camel(field_name)}" if field_name else ""
    
    if unmodified_from_parent:
        re += "    /*\n"
        re += f"    // {field_path} inherited from parent neurodata_type\n"
    if is_inherited:
        re += f"    // name={field_path}, neurodata_type={neurodata_type}: This field is_inheritted={is_inherited}, is_overridden={is_overridden} from the parent neurodata_type\n"
    
    if field_name is not None:
        re += "    DEFINE_REGISTERED_FIELD(\n"
        re += f"        {func_name},\n"
        re += f"        {referenced_namespace}::{neurodata_type},\n"
        re += f'        "{field_path}",\n'
        re += f'        "{doc_string}")\n'
        if unmodified_from_parent:
            re += "    */\n"
    else:
        if unmodified_from_parent:
            re += "    */\n"
        re += f"""
    /*
    * @brief Read an arbitrary {neurodata_type} object owned by this object
    *
    * For {neurodata_type} objects defined in the schema with a fixed name
    * the corresponding DEFINE_REGISTERED_FIELD read functions are preferred
    * because they help avoid the need for specifying the specific name
    * and data type to use.
    *
    * @return The {neurodata_type} object representing the object or a nullptr if the
    * object doesn't exist
    */
    /*
    std::shared_ptr<{neurodata_type}> read{neurodata_type}(const std::string& objectName)
    {{
        std::string parentPath = AQNWB::mergePaths(m_path, "{field_path}");
        std::string objectPath = AQNWB::mergePaths(parentPath, objectName);
        if (m_io->objectExists(objectPath)) {{
        return std::make_shared<{neurodata_type}>(objectPath, m_io);
        }}
        return nullptr;
    }}
    */
"""
    
    return re


def render_define_unnamed_registered_field(
    field_prefix_path: str,
    neurodata_type: str,
    referenced_namespace: str,
    doc: str,
    is_inherited: bool = False,
    is_overridden: bool = False
) -> str:
    """
    Return string for DEFINE_UNNAMED_REGISTERED_FIELD macro.

    Parameters:
    field__prefix_path (str): Prefix path to the field.
    neurodata_type (str): The neurodata type of the field.
    referenced_namespace (str): The namespace of the referenced neurodata type.
    doc (str): Documentation string for the field.
    is_inherited (bool): Indicates whether the field is inherited from the parent
    is_overridden (bool): Indicates wheterh the field overrides a field from the parent

    Returns:
    str: A string representing the DEFINE_UNNAMED_REGISTERED_FIELD macro.
    """
    # Clean up documentation string - remove quotes and commas that would break macro
    doc_string = doc.replace('"', "").replace("'", "").replace(",", " -")
    unmodified_from_parent = is_inherited and not is_overridden
    re = ""
    func_prefix_func_str = snake_to_camel(field_prefix_path.replace("/", "_")) if field_prefix_path is not None else ""
    read_func_name = f"read{func_prefix_func_str}{neurodata_type}"
    create_func_name = f"create{func_prefix_func_str}{neurodata_type}"
    field_prefix_str = str(field_prefix_path) if field_prefix_path is not None else ""
    # Add prefix text
    if unmodified_from_parent:
        re += "    /*\n"
        re += f"    // {field_prefix_path}/<{neurodata_type}> inherited from parent neurodata_type\n"
    if is_inherited:
        re += f"    // prefix_path={field_prefix_path}, neurodata_type={neurodata_type}: This field is_inheritted={is_inherited}, is_overridden={is_overridden} from the parent neurodata_type\n"
    # Add the defintion of the field
    re += "    DEFINE_UNNAMED_REGISTERED_FIELD(\n"
    re += f"        {read_func_name},\n"
    re += f"        {create_func_name},\n"
    re += f"        {referenced_namespace}::{neurodata_type},\n"
    re += f'        "{field_prefix_str}",\n'
    re += f'        "{doc_string}")\n'
    # Add postfix text and return
    if unmodified_from_parent:
        re += "    */\n"
   
    return re


def render_define_attribute_field(
    field_path: str,
    dtype: str,
    doc: str,
    is_inherited: bool = False,
    is_overridden: bool = False,
) -> str:
    """
    Return string for DEFINE_ATTRIBUTE_FIELD macro.

    Parameters:
    field_path (str): Path to the field.
    dtype (str): C++ data type to use by default for read.
    doc (str): Documentation string to use for the field.
    is_inherited (bool): Indicates whether the field is inherited from the parent
    is_overridden (bool): Indicates wheterh the field overrides a field from the parent

    Returns:
    str: A string representing the DEFINE_FIELD macro.
    """
    if field_path is None:
        return "" # Should not happen for attributes
    func_name = f"read{snake_to_camel(field_path.replace('/', '_'))}"
    # Clean up documentation string - remove quotes and commas that would break macro
    doc_string = doc.replace('"', "").replace("'", "").replace(",", " -")
    re = ""
    if is_inherited and not is_overridden:
        re += "    /*\n"
        re += f"    // {field_path} inherited from parent neurodata_type\n"
    if is_overridden:
        re += f"    // {field_path} overrides inherited field from parent neurodata_type\n"

    re += "    DEFINE_ATTRIBUTE_FIELD(\n"
    re += f"        {func_name},\n"
    re += f"        {dtype},\n"
    re += f'        "{field_path}",\n'
    re += f'        "{doc_string}")\n'
    if is_inherited and not is_overridden:
        re += "    */\n"
    return re


def render_define_referenced_registered_field(
    field_path: str,
    registered_type: str,
    doc: str,
    is_inherited: bool = False,
    is_overridden: bool = False,
) -> str:
    """
    Return string for DEFINE_REFERENCED_REGISTERED_FIELD macro.

    Parameters:
    field_path (str): Path to the field.
    registered_type (str): Class to use as the RegisteredType that is being referenced.
    doc (str): Documentation string to use for the field.
    is_inherited (bool): Indicates whether the field is inherited from the parent
    is_overridden (bool): Indicates wheterh the field overrides a field from the parent

    Returns:
    str: A string representing the DEFINE_FIELD macro.
    """
    if field_path is None:
        return "" # Should not happen for attributes
    func_name = f"read{snake_to_camel(field_path.replace('/', '_'))}"
    # Clean up documentation string - remove quotes and commas that would break macro
    doc_string = doc.replace('"', "").replace("'", "").replace(",", " -")
    re = ""
    if is_inherited and not is_overridden:
        re += "    /*\n"
        re += f"    // {field_path} inherited from parent neurodata_type\n"
    if is_overridden:
        re += f"    // {field_path} overrides inherited field from parent neurodata_type\n"

    re += "    DEFINE_REFERENCED_REGISTERED_FIELD(\n"
    re += f"        {func_name},\n"
    re += f"        {registered_type},\n"
    re += f'        "{field_path}",\n'
    re += f'        "{doc_string}")\n'
    if is_inherited and not is_overridden:
        re += "    */\n"
    return re


def render_define_dataset_field(
    field_path: str,
    dtype: str,
    doc: str,
    is_inherited: bool = False,
    is_overridden: bool = False,
) -> str:
    """
    Return string for DEFINE_DATASET_FIELD macro.

    Parameters:
    field_path (str): Path to the field.
    dtype (str): C++ data type to use by default for read.
    doc (str): Documentation string to use for the field.
    is_inherited (bool): Indicates whether the field is inherited from the parent
    is_overridden (bool): Indicates wheterh the field overrides a field from the parent

    Returns:
    str: A string representing the DEFINE_FIELD macro.
    """
    if field_path is None:
        logger.warning("Missing field_path in render_define_dataset_field")
        return "" # Should not happen for datasets
    func_name = snake_to_camel(field_path.replace('/', '_'))
    read_func_name = f"read{func_name}"
    record_func_name = f"record{func_name}"
    # Clean up documentation string - remove quotes and commas that would break macro
    doc_string = doc.replace('"', "").replace("'", "").replace(",", " -")
    re = ""
    if is_inherited and not is_overridden:
        re += "    /*\n"
        re += f"    // {field_path} inherited from parent neurodata_type\n"
    if is_overridden:
        re += f"    // {field_path} overrides inherited field from parent neurodata_type\n"

    re += "    DEFINE_DATASET_FIELD(\n"
    re += f"        {read_func_name},\n"
    re += f"        {record_func_name},\n"    
    re += f"        {dtype},\n"
    re += f'        "{field_path}",\n'
    re += f'        "{doc_string}")\n'
    if is_inherited and not is_overridden:
        re += "    */\n"
    return re


def get_initialize_method_parameters(neurodata_type: Spec, type_to_namespace_map: Dict[str, str], sorted: bool = True) -> List[Dict]:
    """
    Internal helper function to create a list of all the parameters for the initialize method.

    For each parameter, the function collects the original spec, c++ data type, variable name to use,
    and default value.

    NOTE: For completeness, the returned list also includes groups that we own that do not have a neurodata_type.
    Those groups are not passed as parameters to the initialize method, but they need to be initialized
    in the body of the initialize method. We can therefore use the returned list to also generate the
    initialization code for those groups. To distinguish those groups from parameters that are passed
    to the initialize method, the 'variable_name' (as well as the 'cpp_type_str' and 'cpp_default_value')
    field is set to None for those groups.

    Parameters:
    neurodata_type (Spec): The neurodata type to render
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.
    sorted (bool): If false, keep the order as they are found in the iteration through the schema.
                   If true, sort the parameter list such that we have: 
                   1. required parameters with no default value
                   2. required parameters with default value
                   3. optional RegisteredType instance parameters with empty default values

    Returns:
    List[Dict]: A list of dictionaries, where each dictionary describes a parameter.
    """
    if neurodata_type is None:
        return []

    # Collect all specs that define parameters that we need to include in the initialize call
    parameter_list = []
    # Next we iterate through all the objects we own directly. This includes recursion through all
    # objects that do not have a neurodata_type, to make sure we initialize all the attributes, datasets,
    # and groups they own as well
    sub_objects_to_process = [(neurodata_type, None, "" ),]
    visited = set()
    while sub_objects_to_process:
        ## 1: Add all sub-objects to the list of objects to process

        # Get the next object to process
        obj, parent, path_prefix = sub_objects_to_process.pop(0)
        # Ensure we don't process objects twice (although this should not happen in a valid schema)
        if id(obj) in visited:
            continue
        visited.add(id(obj))

        # Process the object and add any sub-objects to the list of objects to process
        current_parent = obj if obj != neurodata_type else None
        
        # Determine the path for the children of the current object.
        # We extend the path for two types of containers:
        # 1. Untyped Groups: These are structural groups (e.g., 'general') that don't have a
        #    specific C++ class. We need to traverse into them to find the actual parameters.
        #    The check 'obj.data_type is None' identifies these.
        # 2. Named Datasets: Any dataset with a name can have its own attributes. This is
        #    especially important for DynamicTable columns, which are datasets that contain
        #    other attributes. This ensures that attributes of a column get the correct
        #    path prefix (e.g., 'spike_times/resolution').
        path_for_children = path_prefix
        if (isinstance(obj, GroupSpec) and obj.data_type is None and obj.name) or \
           (isinstance(obj, DatasetSpec) and obj.name):
             path_for_children = f"{path_prefix}/{obj.name}" if path_prefix else obj.name

        # If the object has a data_type, it's a self-contained parameter. We should not traverse
        # its children, as they will be handled by that type's own constructor. We only
        # traverse into untyped groups or the top-level neurodata_type itself.
        is_typed_boundary = isinstance(obj, (DatasetSpec, GroupSpec)) and obj.data_type is not None
        if not is_typed_boundary or obj == neurodata_type:
            if hasattr(obj, "attributes"):
                for attr in obj.attributes:
                    sub_objects_to_process.append((attr, current_parent, path_for_children))
            if hasattr(obj, "datasets"):
                for dataset in obj.datasets:
                    sub_objects_to_process.append((dataset, current_parent, path_for_children))
            if hasattr(obj, "groups"):
                for group in obj.groups:
                    sub_objects_to_process.append((group, current_parent, path_for_children))

        # If the object is the neurodata_type itself
        if obj == neurodata_type:
            # In the special case that we are a neurodata_type that defines a Dataset, then we want
            # the dataset itself to appear as a parameter, since we need to configure the dataset
            # via an ArrayDatasetConfig. To achieve this we are creating a dummy spec to represent
            # our dataset and process it as a regular parameter in one of the next iterations.
            # NOTE: This approach of "faking" a spec is a bit hacky but it works and avoids custom logic throughout the code
            if isinstance(obj, DatasetSpec):
                copySpec = DatasetSpec(
                    doc=obj.doc, 
                    dtype=obj.dtype,
                    name="data",
                    shape=obj.shape,
                    dims=obj.dims)
                sub_objects_to_process.append((copySpec, None, ""))
            # If the object is the neurodata_type itself, we just use it to iterate
            # but we don't add it as a parameter itself
            continue
        
        # If the object is an untyped group, we just use it to iterate
        # but we don't add it as a parameter itself
        if isinstance(obj, GroupSpec) and obj.data_type is None:
            continue

        ## 2: Process the current object and add it to the parameter list

        # Name the parameter using lowerCamelCase consistently, with the parent name first
        # (to reflect the path order in the file, e.g. general/experimenter -> generalExperimenter).
        # When there is no parent, the field name alone is used in lowerCamelCase.
        # When the object has no schema name, a "Param" prefix plus the data type name is used.
        if parent is not None:
            parent_lower_camel = snake_to_camel(parent.name, lower_camel=True)
            if obj.name is not None:
                variable_name = f"{parent_lower_camel}{snake_to_camel(obj.name)}"
            else:
                variable_name = f"{parent_lower_camel}Param{obj.data_type}"
        else:
            if obj.name is not None:
                variable_name = snake_to_camel(obj.name, lower_camel=True)
            else:
                variable_name = f"param{obj.data_type}"
        
        # Determine the full path for the object
        obj_path = obj.name
        if path_prefix and obj.name:
            obj_path = f"{path_prefix}/{obj.name}"
        elif path_prefix:
            obj_path = path_prefix

        # Determine C++ type information and default value
        cpp_type_str = ""
        default_value = None
        optional_registered_type = False
        # Datasets with a neurodata_type_inc need to be created by the user first
        if isinstance(obj, (DatasetSpec, GroupSpec)) and (obj.data_type is not None):
            # Determine the neurodata_type name
            type_name = obj.data_type
            full_type_name = type_name
            if type_name in type_to_namespace_map:
                actual_cpp_namespace_name = to_cpp_namespace_name(type_to_namespace_map[type_name])
                full_type_name = f"{actual_cpp_namespace_name}::{type_name}"
            # If the dataset is a single object, then we pass it as a shared_ptr, otherwise
            # we pass a vector of shared_ptrs
            if obj.name is not None or obj.quantity in ["1", "?", "one_or_zero"]:
                cpp_type_str = f"const std::shared_ptr<{full_type_name}>&"
                if not obj.required:
                    default_value = "nullptr"
                    optional_registered_type = True
            else:
                cpp_type_str = f"const std::vector<std::shared_ptr<{full_type_name}>>&"
                if not obj.required:
                    default_value = "{}"
                    optional_registered_type = True
        # Datasets should be available for acquistion and therefore use ArrayDataSetConfig to
        # allow the user configure the dataset. The special case are scalar datasets with a
        # default value as those should be written on initialize directly in the same way
        # attributes are being created directly.
        elif isinstance(obj, DatasetSpec) and not (getattr(obj, "default_value", None) is not None or getattr(obj, "value", None) is not None):
            if not obj.required:
                cpp_type_str = f"const std::optional<AQNWB::IO::ArrayDataSetConfig>"
                default_value = "std::nullopt"
            else:
                cpp_type_str = f"const AQNWB::IO::ArrayDataSetConfig&"
        # Groups we own with no neurodata_type are not passed as parameters
        elif isinstance(obj, GroupSpec):
            variable_name = None
            cpp_type_str = None
            default_value = None
        # Attributes and datasets with a default value should be passed by value
        else:
            obj_cpp_type = get_cpp_type(obj.dtype)
            if obj.shape is None:
                if obj_cpp_type == "std::string":
                    cpp_type_str = f"const {obj_cpp_type}&"
                else:
                    cpp_type_str = f"{obj_cpp_type}"
            else:
                cpp_type_str = f"const std::vector<{obj_cpp_type}>&"
            # Determine default value from the spec if available
            default_value = getattr(obj, "default_value", None)
            if default_value is None:
                default_value = getattr(obj, "value", None)
            if default_value is not None:
                # Scalar default value
                if obj.shape is None:
                    # Scalar string default value
                    if obj_cpp_type == "std::string":
                        default_value = f'"{default_value}"'
                    # Scalar numeric default value
                    else:
                        default_value = str(default_value)
                # Default value is a list
                else:
                    # TODO This does not handle multi-dimensional default values
                    # Default value is a list of strings
                    if "std::string" in obj_cpp_type:
                        default_value = str([f'"{val}"' for val in default_value]).replace("[", "}").replace("]", "}")
                    # Default value is a list of numbers
                    else:
                        default_value = str(default_value).replace("[", "}").replace("]", "}")

        parameter_list.append({
            'spec': obj,
            'parent_spec': parent,
            'variable_name': variable_name,
            'cpp_type': cpp_type_str,
            'cpp_default_value': default_value,
            'path': obj_path,
            'optional_registered_type': optional_registered_type, 
            'value_fixed': getattr(obj, "value", None) is not None
        })

    # Sort the parameter list so that we have:
    # 1. required parameters with no default value
    # 2. required parameters with default value
    # 3. optional neurodata_type instance parameters with empty default values
    if sorted:
        parameter_list.sort(
            key=lambda item: (
                item['cpp_default_value'] is not None,  # no default value first
                item['optional_registered_type'],  # required parameters first
                getattr(item['spec'], "default_value", None) is None  and getattr(item['spec'], "value", None) is None # optional parameters with a set default or fixed value first
            )
        )

    return parameter_list


def render_initialize_method_signature(neurodata_type: Spec, type_to_namespace_map: Dict[str, str], for_call: bool = False, add_default_values: bool = False) -> str:
    """
    Internal helper function to create the initialize method signature
    Parameters:
    neurodata_type (Spec): The neurodata type to render
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.
    for_call (bool): If true, generate the arguments for a function call, otherwise the signature.
    add_default_values (bool): If true, add default values to the parameter string if available.

    Returns:
    str: The function signature for the initialize method
    """
    # Get the list of parameters
    all_initialize_params = get_initialize_method_parameters(
        neurodata_type=neurodata_type,
        type_to_namespace_map=type_to_namespace_map,
        sorted=True  # Sort parameters such that optional parameters are last
    )

    # Now we can create the function signature and add all the parameters
    funcSignature = "initialize("
    for param in all_initialize_params:
        # Skip parameters that are groups we own that do not have a neurodata_type
        if param['variable_name'] is None:
            continue
        # Skip parameters for which the value is fixed in the schema as they should not be setable by the user
        if param['value_fixed']:
            continue
        if for_call:
            param_string = f"\n       {param['variable_name']},"
        else:
            param_string = "\n       "
            # Define the parameter signature
            param_string += f"{param['cpp_type']} {param['variable_name']}"
            if add_default_values and param['cpp_default_value'] is not None:
                param_string += f" = {param['cpp_default_value']}"
            param_string += ","
        # comment optional RegisteredType parameters as they are usually not be handled in the
        # initialize by the user but are commonly created afterwards via corrsponding create methods
        if param['optional_registered_type']:
            # Comment out the parameter and add a note so the developer can decide what to do
            param_string = param_string.replace("       ", "       // ", 1)
            param_string += " // Optional RegisteredTypes are usually created after initialize"
            # Optional neurodata_types are listed last so we need to remove the "," from the
            # last uncommented parameter to make sure we have valid C++ code
            funcSignature = funcSignature.rstrip(',')
        funcSignature += param_string
    funcSignature = funcSignature.rstrip(",") + "\n    )"
    return funcSignature


def render_initialize_method_header(
    neurodata_type: Spec,
    type_to_namespace_map: Dict[str, str]
) -> str:
    """
    Render the header portion of the initialize method for a neurodata_type

    Parameters:
    neurodata_type (Spec): Spec of the neurodata_type to render
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.

    Returns:
    str : The string with the method signature for the header
    """
    funcSignature = render_initialize_method_signature(
        neurodata_type=neurodata_type,
        type_to_namespace_map=type_to_namespace_map,
        for_call=False,
        add_default_values=True)
    headerSrc = f"""
    // TODO: Update the initialize method as appropriate.
    /**
     * @brief Initialize the object
     */
    Status {funcSignature};
"""
    return headerSrc


def render_initialize_method_cpp(
    class_name: str,
    parent_class_name: str,
    neurodata_type: Spec,
    type_to_namespace_map: Dict[str, str],
    parent_neurodata_type: Spec = None,
) -> str:
    """
    Render the initialize method for a neurodata_type

    Parameters:
    class_name (str): Name of the class
    parent_class_name (str): Name of the parent class
    neurodata_type (Spec): Spec of the neurodata_type to render
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.
    parent_neurodata_type (Spec): Optional spec of the parent neurodata_type

    Returns:
    str : The string with the method implementation for the cpp file.
    """
    ### Define internal helper functions
    def attr_init(
        attr: Spec, param: dict, is_inherited: bool, is_overridden: bool
    ) -> str:
        """
        Internal helper function to create suggested initializion code for an attribute

        Parameters:
        attr (Spec): The attribute to render
        param (dict): The parameter dictionary for the attribute generated by get_initialize_method_parameters
        is_inherited (bool): Is this an inherited attribute
        is_overriden (bool): Is this an overrriden attribute

        Returns:
        str: The suggested initialization code for the attribute
        """
        cpp_param_var_name = param['variable_name']
        re = f"    // TODO: Initialize {param['path']} attribute"
        if param['value_fixed']:
            re += f"with fixed value {param['cpp_type']} {param['cpp_default_value']}"
        if is_inherited or is_overridden:
            re += f". This attribute is_inheritted={is_inherited}, is_overridden={is_overridden}"
        re += "\n"
        # add example initializtion hints
        attr_cpp_type = get_cpp_type(attr.dtype)
        attr_base_type = get_basedata_type(attr.dtype)
        
        # Determine parent path and attribute name from the full path
        path_parts = param['path'].rsplit('/', 1)
        if len(path_parts) > 1:
            parent_rel_path, attr_name = path_parts
            parent_path_str = f'AQNWB::mergePaths(m_path, "{parent_rel_path}")'
        else:
            attr_name = param['path']
            parent_path_str = 'm_path'
        # Define the create call
        if attr_cpp_type == "std::string":
            re += f'    // m_io->createAttribute({cpp_param_var_name}, {parent_path_str}, "{attr_name}");\n\n'
        elif attr.shape is None:
            re += f'    // m_io->createAttribute({attr_base_type}, &{cpp_param_var_name}, {parent_path_str}, "{attr_name}");\n\n'
        return re

    def dataset_init(dataset: Spec, param: dict, is_inherited: bool, is_overridden: bool) -> str:
        """
        Internal helper function to create suggested initializion code for a dataset

        Parameters:
        dataset (Spec): The dataset to render
        param (dict): The parameter dictionary for the dataset generated by get_initialize_method_parameters
        is_inherited (bool): Is this an inherited attribute
        is_overriden (bool): Is this an overrriden attribute

        Returns:
        str: The suggested initialization code for the dataset
        """
        if param['path'] is None:
            return f"    // NOTE: Anonymous dataset of type {dataset.data_type if hasattr(dataset, 'data_type') else 'unknown'} passed as parameter {param['variable_name']}. No initialization needed here.\n\n"
        
        cpp_param_var_name = param['variable_name']
        pathVarName = f"{snake_to_camel(param['path'].replace('/', '_'))}Path"
        re = f"    // TODO: Initialize {param['path']} dataset"
        if param['value_fixed']:
            re += f" with fixed value {param['cpp_type']} {param['cpp_default_value']}"
        if is_inherited or is_overridden:
            re += f". This dataset is_inheritted={is_inherited}, is_overridden={is_overridden}"
        re += "\n"
        re += f'    // auto {pathVarName} = AQNWB::mergePaths(m_path, "{param["path"]}");\n'
        # add example initializtion hints
        dataset_cpp_type = param['cpp_type']
        if dataset_cpp_type == "const std::string&":
            re += f"    // m_io->createStringDataSet({pathVarName}, {cpp_param_var_name})\n"
        else:
            if "ArrayDataSetConfig" in dataset_cpp_type:
                    re += f"    // std::unique_ptr<IO::BaseRecordingData> {cpp_param_var_name}Data = m_io->createArrayDataSet({cpp_param_var_name}, {pathVarName});\n"
            else:
                re += f"    // create scalar dataset at {pathVarName} with {'default' if not param['value_fixed'] else 'fixed'} value {param['cpp_type']} {param['cpp_default_value']}\n"
        re += "\n"
        return re
    
    ### Get all the parameters for the initialize method
    all_initialize_params = get_initialize_method_parameters(
        neurodata_type=neurodata_type,
        type_to_namespace_map=type_to_namespace_map,
        sorted=False)

    ### Render variables with fixed values that are omitted from the initialize function signature
    ### but which may be needed for calling the parent constructor (were they may not be fixed)
    # Set the fixed variable value if necessary
    fixed_value_inits = ""
    parent_initialize_params = get_initialize_method_parameters(
        neurodata_type=parent_neurodata_type,
        type_to_namespace_map=type_to_namespace_map,
        sorted=False)
    for param in all_initialize_params:
        if param['value_fixed']:
            is_inherited = neurodata_type.is_inherited_spec(param['spec'])
            is_overridden = neurodata_type.is_overridden_spec(param['spec']) 
            # Special case were we modify a Dataset inherited from the parent. E.g,. in IZeroClampSeries
            # we set the value for the "bias_current" dataset to 0. In this case the parent expects a
            # ArrayDataSetConfig which we cannot set to 0. To ensure the code compiles we use the parents
            # definition here and add a not for the developer to fix this in the code
            if is_inherited and is_overridden and isinstance(param['spec'], DatasetSpec):
                for parent_parm in parent_initialize_params:
                    if parent_parm['variable_name'] == param['variable_name']:
                        fixed_value_inits += f"    {parent_parm['cpp_type']} {param['variable_name']} = {parent_parm['cpp_default_value']};"
                        fixed_value_inits += f" // TODO: Value should be fixed to {param['cpp_type']} {param['cpp_default_value']}. "
                        break
            # Regular case were we have a variable for which we are setting the fixed value
            else:
                fixed_value_inits  += f"    {param['cpp_type']} {param['variable_name']} = {param['cpp_default_value']}; // "
            fixed_value_inits += f"This {'dataset' if isinstance(param['spec'], DatasetSpec) else 'attribute'} field is_inheritted={is_inherited}, is_overridden={is_overridden}\n"
    if len(fixed_value_inits) > 0:
        fixed_value_inits = f"// Initalize fixed field values\n{fixed_value_inits}"

    ### Render the initalize function signature 
    funcSignature = render_initialize_method_signature(
        neurodata_type=neurodata_type,
        type_to_namespace_map=type_to_namespace_map,
        for_call=False,
        add_default_values=False)

    ### Render the parents initalize call
    if parent_neurodata_type is not None:
        parent_initialize_signature = render_initialize_method_signature(
            neurodata_type=parent_neurodata_type,
            type_to_namespace_map=type_to_namespace_map,
            for_call=True,
            add_default_values=False)
        parent_initialize_call = f"Status parentInitStatus = {parent_class_name}::{parent_initialize_signature};\n"
        parent_initialize_call += f"    initStatus = initStatus && parentInitStatus;"
    else:
        parent_initialize_call = "// TODO: Call the parents initialize method if applicable.\n"
        parent_initialize_call += f"    // Status parentInitStatus {parent_class_name}::initialize()"
        parent_initialize_call += f"    // initStatus = initStatus && parentInitStatus;"

    #### Render initialization for attributes, datasets and groups
    initialize_fields_src = ""
    for param in all_initialize_params:
        spec = param['spec']
        is_inherited = neurodata_type.is_inherited_spec(spec)
        is_overridden = neurodata_type.is_overridden_spec(spec)
        
        if isinstance(spec, GroupSpec):
            if param['variable_name'] is None: # Untyped, named group we own
                path_var_name = f"{snake_to_camel(param['path'].replace('/', '_'))}Path"
                initialize_fields_src += f"    // TODO: Initialize {param['path']} group\n"
                initialize_fields_src += f'    // auto {path_var_name} = AQNWB::mergePaths(m_path, "{param["path"]}");\n'
                initialize_fields_src += f"    // m_io->createGroup({path_var_name});\n\n"
            else: # Typed group passed as parameter
                if param['optional_registered_type']:
                    initialize_fields_src += f"    // TODO: Optional RegisteredType {param['cpp_type']} passed as parameter {param['variable_name']}. Usually created after initialize.\n\n"
                else:
                    initialize_fields_src += f"    // TODO: Required RegisteredType {param['cpp_type']} passed as parameter {param['variable_name']}\n\n"
        elif isinstance(spec, DatasetSpec):
            initialize_fields_src += dataset_init(
                dataset=spec,
                param=param,
                is_inherited=is_inherited,
                is_overridden=is_overridden
            )
        else: # Attribute
            initialize_fields_src += attr_init(
                attr=spec,
                param=param,
                is_inherited=is_inherited,
                is_overridden=is_overridden,
            )
    initialize_fields_src = initialize_fields_src.rstrip("\n") # Remove extra endlines

    ### Generate cpp source
    cppSrc = f"""
// Initialize the object
Status {class_name}::{funcSignature}
{{
    Status initStatus = Status::Success;

    {fixed_value_inits}
    // Call parent initialize method. 
    {parent_initialize_call};
    
    // Initialize attributes, datasets, and groups
    {initialize_fields_src}
    
    return initStatus;
}}
"""

    return cppSrc


def snake_to_camel(name: str, lower_camel: bool = False) -> str:
    """
    Convert snake_case to CamelCase or lowerCamelCase.

    Parameters:
    name (str): The snake_case string to convert.
    lower_camel (bool): If True, return lowerCamelCase (first letter lowercase).
                        If False (default), return CamelCase (all words capitalized).

    Returns:
    str: The converted CamelCase or lowerCamelCase string.
    """
    if name is not None:
        camel = "".join(word.title() for word in re.split("[_-]", name))
        if lower_camel and camel:
            return camel[0].lower() + camel[1:]
        return camel
    else:
        return None


def to_cpp_namespace_name(name: str) -> str:
    """
    Convert a namespace name to a C++ namespace name by replacing hyphens with underscores and converting to uppercase.

    Parameters:
    name (str): The original namespace name.

    Returns:
    str: The converted C++ namespace name.
    """
    return name.upper().replace("-", "_")


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


def get_schema_subfolder_name(schema_file_path: Path) -> str:
    """
    Generate subfolder name based on schema file name with specific rules:
    1) Ignore file extension (e.g., .yaml)
    2) Ignore "nwb." prefix on filename
    3) Replace remaining "." with "_"

    Parameters:
    schema_file_path (Path): Path to the schema file.

    Returns:
    str: The subfolder name.
    """
    if schema_file_path is None:
        return ""

    # Get the filename without extension
    filename = schema_file_path.stem
    
    # Remove "nwb." prefix if present
    if filename.startswith("nwb."):
        filename = filename[4:]  # Remove "nwb." prefix
    
    # Replace remaining dots with underscores
    subfolder_name = filename.replace(".", "_")
    
    return subfolder_name


def get_referenced_types(neurodata_type: Spec, type_to_namespace_map: Dict[str, str]) -> set[str]:
    """
    Get all referenced neurodata types from a given neurodata type.

    Parameters:
    neurodata_type (Spec): The neurodata type spec.
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.

    Returns:
    set[str]: A list of referenced neurodata type names.
    """
    referenced_types = []
    # Get the list of parameters
    all_initialize_params = get_initialize_method_parameters(
        neurodata_type=neurodata_type,
        type_to_namespace_map=type_to_namespace_map
    )
    # Check each parameter if it is (or stores) a reference to another neurodata_type
    for param in all_initialize_params:
        spec = param['spec']
        # Check if the spec is a reference to another neurodata_type
        if isinstance(spec, (DatasetSpec, GroupSpec)) and spec.data_type is not None:
            referenced_types.append(spec.data_type)
        # Check if the spec's dtype is a reference type
        if hasattr(spec, 'dtype') and isinstance(spec.dtype, dict) and spec.dtype.get("reftype", None) == "object":
            target_type = spec.dtype.get("target_type", None)
            if target_type is not None:
                referenced_types.append(target_type)
    return set(referenced_types)  # Return unique types only


def parse_schema_file(file_path: Path) -> Tuple[SpecNamespace, Dict[str, Spec], Dict[str, Path], Dict[str, str]]:
    """
    Parse a schema file and return the namespace and data types using PyNWB.

    Parameters:
    file_path (Path): Path to the schema file.

    Returns:
    Tuple[SpecNamespace, Dict[str, Spec], Dict[str, Path], Dict[str, str]]: The namespace, a dictionary of neurodata types,
    a dictionary mapping neurodata type names to their source schema file paths, and a dictionary mapping neurodata type names to their original namespace names.
    """
    # Find the namespace file
    namespace_path = file_path
    if not (
        file_path.name.endswith(".namespace.yaml")
        or file_path.name.endswith(".namespace.json")
    ):
        # Try to find the namespace file
        if file_path.is_dir():
            # If a directory is provided, look for namespace files in it
            for filename in file_path.iterdir():
                if filename.name.endswith(".namespace.yaml") or filename.name.endswith(
                    ".namespace.json"
                ):
                    namespace_path = filename
                    break
        else:
            # If a schema file is provided, try to find the corresponding namespace file
            dir_path = file_path.parent
            base_name = file_path.stem
            for ext in [".namespace.yaml", ".namespace.json"]:
                potential_path = dir_path / f"{base_name}{ext}"
                if potential_path.exists():
                    namespace_path = potential_path
                    break

    # Load the namespace data to get the namespace name
    if namespace_path.name.endswith(".json"):
        with open(namespace_path, "r") as f:
            namespace_data = json.load(f)
    else:  # Assume YAML
        with open(namespace_path, "r") as f:
            yaml = YAML(typ="safe")
            namespace_data = yaml.load(f)

    namespace_name = namespace_data["namespaces"][0]["name"]

    type_map = get_type_map()
    type_map.load_namespaces(str(namespace_path))
    namespace = type_map.namespace_catalog.get_namespace(namespace_name)

    # Get all the neurodata types in the namespace and track their source files and namespaces
    neurodata_types = {}
    type_to_file_map = {}
    type_to_namespace_map = {}
    
    # Get all specs from the catalog
    for spec_name in namespace.catalog.get_registered_types():
        spec = namespace.catalog.get_spec(spec_name)
        if hasattr(spec, "neurodata_type_def") and spec.neurodata_type_def is not None:
            neurodata_types[spec.neurodata_type_def] = spec
            
            # Get the source file for this spec using the catalog method
            source_file = namespace.catalog.get_spec_source_file(spec_name)
            if source_file:
                type_to_file_map[spec.neurodata_type_def] = Path(source_file)
            else:
                # No specific source file was found
                type_to_file_map[spec.neurodata_type_def] = None
            
            # Determine the actual namespace this type belongs to
            # Check if this type comes from an included namespace
            type_namespace = namespace.name  # Default to current namespace
            
            # Look through all namespaces in the catalog to find the original namespace
            for ns_name in type_map.namespace_catalog.namespaces:
                ns = type_map.namespace_catalog.get_namespace(ns_name)
                if ns != namespace and spec_name in ns.catalog.get_registered_types():
                    # This type is originally from a different namespace
                    type_namespace = ns_name
                    break
            
            type_to_namespace_map[spec.neurodata_type_def] = type_namespace

    return namespace, neurodata_types, type_to_file_map, type_to_namespace_map


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
        if spec.get(spec.def_key, None) is not None:
            neurodata_types[spec.neurodata_type_def] = spec

    return neurodata_types


def generate_header_file(
    namespace: SpecNamespace, neurodata_type: Spec, all_types: Dict[str, Spec], type_to_file_map: Dict[str, Path], type_to_namespace_map: Dict[str, str]
) -> str:
    """
    Generate C++ header file for a neurodata type.

    Parameters:
    namespace (SpecNamespace): The namespace object.
    neurodata_type (Spec): The neurodata type spec.
    all_types (Dict[str, Spec]): A dictionary of all neurodata types.
    type_to_file_map (Dict[str, Path]): Mapping of types to their source schema files.
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.

    Returns:
    str: The generated C++ header file content.
    """
    namespace_name = namespace.name
    cpp_namespace_name = to_cpp_namespace_name(namespace_name)
    type_name = neurodata_type.neurodata_type_def
    actual_cpp_namespace_name = to_cpp_namespace_name(type_to_namespace_map[type_name])
    class_name = type_name
    is_included_type = actual_cpp_namespace_name != cpp_namespace_name

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
            parent_class = to_cpp_namespace_name(type_to_namespace_map[parent_class]) + "::" + parent_class
    else:
        # TODO: Handle special case when we create HDMF_COMMON::Data and HDMF_COMMON::Container
        # Default parent class based on neurodata_type
        if isinstance(neurodata_type, DatasetSpec):
            parent_class = "AQNWB::NWB::Data"
        else:
            parent_class = "AQNWB::NWB::Container"
    
    if class_name in ["Container", "Data"]:
        parent_class = "AQNWB::NWB::RegisteredType" # special case for container type to avoid registering Container as a subclass of Container

    # Start building the header file
    header = """#pragma once

// Common STL includes
#include <memory>
#include <string>
#include <vector>
#include <optional>
// Base AqNWB includes for IO and RegisteredType
#include "nwb/RegisteredType.hpp"
#include "io/ReadIO.hpp"
#include "io/BaseIO.hpp"
"""

    # Add additional includes based on parent class
    header += "// Include for parent type\n"
    if parent_type:
        if parent_type == "data":
            header += '#include "nwb/hdmf/base/Data.hpp"\n'
        elif "/" in parent_type:
            # External namespace, need to include appropriate header
            pass
        else:
            # Same namespace or different namespace, need to find the correct include path
            if parent_type in type_to_file_map and parent_type in type_to_namespace_map:
                parent_namespace = type_to_namespace_map[parent_type]
                parent_source_file = type_to_file_map[parent_type]
                parent_subfolder = get_schema_subfolder_name(parent_source_file)
                parent_include_path = f"{parent_namespace}/{parent_subfolder}/{parent_type}.hpp"
                header += f'#include "{parent_include_path}"\n'
            else:
                # Fallback to simple include
                header += f'#include "{parent_type}.hpp"\n'
    else:
        # TODO: Handle special case when we create HDMF_COMMON::Data and HDMF_COMMON::Container
        # No parent type specified, use default based on whether it's a Dataset or Group
        if "Data" in parent_class:
            header += '#include "nwb/hdmf/base/Data.hpp"\n'
        else:
            header += '#include "nwb/hdmf/base/Container.hpp"\n'

    # Determine additional includes required for DEFINE_REGISTERED_FIELD macro definitions 
    # for groups and datasets, and links to other neurodata_types referenced via attributes
    referenced_types = get_referenced_types(neurodata_type, type_to_namespace_map)
    if len(referenced_types) > 0:
        header += "// Includes for types that are referenced and used\n"
        # Add includes for all referenced types
        for ref_type in referenced_types:
            ref_namespace = type_to_namespace_map.get(ref_type, namespace.name)
            ref_subfolder = get_schema_subfolder_name(type_to_file_map.get(ref_type, None))
            ref_include_path = f"{ref_namespace}/{ref_subfolder}/{ref_type}.hpp"
            header += f'#include "{ref_include_path}"\n'

    # Include the namespace header
    header += "// Include for the namespace schema header\n"
    schema_header = type_to_namespace_map.get(type_name, namespace_name).replace("-", "_") + ".hpp"
    header += f'#include "spec/{schema_header}"\n'

    # Get parent neurodata_type spec
    parent_neurodata_type_spec = all_types.get(parent_type) if parent_type else None

    # Get attributes, datasets, and groups
    header_initialize_src = render_initialize_method_header(
        neurodata_type=neurodata_type,
        type_to_namespace_map=type_to_namespace_map,
    )
    header += f"""
namespace {actual_cpp_namespace_name} {{

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

    /**
     * @brief Virtual destructor.
     */
    virtual ~{class_name}() override {{}}
    
    {header_initialize_src}
"""

    # Add DEFINE_FIELD and DEFINE_REGISTERED_FIELD macros
    header += "\n"
    header += "    // Define read methods\n"

    # Place DEFINE_FIELD , DEFINE_REGISTERED field definitions that are commented out because they are inherted at the end
    commented_fields = []

    def is_commented_field_def(input_field: str) -> bool:
        """
        Internal helper function ot check if a DEFINE_FIELD or DEFINE_REGISTERED_FIELD
        definition created via the render_define_attribute_field, render_define_dataset_field,
        or render_registered_field methods has been commented out. This is used to be able to 
        collect commented fields such that they can be added at the end rather than intermixed
        with other fields.
        """
        lines = input_field.split("\n")
        if len(lines) > 0:
            return input_field.replace(" ", "").startswith("/*")
        return False
    
    # Get all the parameters for the initialize method
    all_initialize_params = get_initialize_method_parameters(
        neurodata_type=neurodata_type,
        type_to_namespace_map=type_to_namespace_map,
        sorted=False  # Do not sort to keep original order from the spec
    )

    # Add fields for attributes, datasets, and groups
    for param in all_initialize_params:
        spec = param['spec']
        doc = spec.get("doc", "No documentation provided").replace("\n", " ")
        is_inherited=neurodata_type.is_inherited_spec(spec)
        is_overridden=neurodata_type.is_overridden_spec(spec)
        field_path = param.get('path', spec.name)
        fieldDef = ""
        # Group/Dataset with a neurodata_type
        if (isinstance(spec, (GroupSpec, DatasetSpec))) and spec.data_type is not None:
            referenced_type = spec.data_type
            referenced_namespace = to_cpp_namespace_name(type_to_namespace_map.get(referenced_type, namespace.name))
            if spec.name is not None:
                fieldDef = render_define_registered_field(
                    field_name=spec.name,
                    field_path=field_path,
                    neurodata_type=referenced_type,
                    referenced_namespace=referenced_namespace,
                    doc=doc,
                    is_inherited=is_inherited,
                    is_overridden=is_overridden)
            else:
                fieldDef = render_define_unnamed_registered_field(
                    field_prefix_path=field_path,
                    neurodata_type=referenced_type,
                    referenced_namespace=referenced_namespace,
                    doc=doc,
                    is_inherited=is_inherited,
                    is_overridden=is_overridden)
        # Dataset (wihout a neurodata_type)
        elif isinstance(spec, DatasetSpec):
            fieldDef = render_define_dataset_field(
                field_path=field_path,
                dtype=get_cpp_type(spec.dtype),
                doc=doc,
                is_inherited=is_inherited,
                is_overridden=is_overridden,
            )
        # Attribute
        elif isinstance(spec, AttributeSpec): 
            target_type = None
            if isinstance(spec.dtype, dict) and "reftype" in spec.dtype:
                target_type = spec.dtype.get("target_type", None)
            # if the attribute stores a reference to a RegisteredType
            if target_type is not None:
                fieldDef = render_define_referenced_registered_field(
                    field_path=field_path,
                    registered_type=target_type,
                    doc=doc,
                    is_inherited=is_inherited,
                    is_overridden=is_overridden
                )
            # else read as a regular attribute:
            else:
                fieldDef = render_define_attribute_field(
                    field_path=field_path,
                    dtype=get_cpp_type(spec.dtype),
                    doc=doc,
                    is_inherited=is_inherited,
                    is_overridden=is_overridden
                )
        
        if fieldDef:
            if is_commented_field_def(fieldDef):
                commented_fields.append(fieldDef)
            else:
                header += fieldDef + "\n"
    if len(commented_fields) > 0:
        header += "    // TODO: The following fields have been commented because they should have been inherited from the parent class.\n"
        header += "    //       They are included here for your convenience so you can decide which fields may still need be defined here.\n"
        for fieldDef in commented_fields:
            header += fieldDef +"\n"

    # Add REGISTER_SUBCLASS macro
    # Extract just the class name from parent_class (remove namespace)
    parent_class_name = parent_class.split("::")[-1] if "::" in parent_class else parent_class
    
    if is_included_type:
        header += f"""
        REGISTER_SUBCLASS(
            {class_name},
            {parent_class_name},
            "{actual_cpp_namespace_name}")  // TODO: Use namespace from schema header
        """
    else:
        header += f"""
        REGISTER_SUBCLASS(
            {class_name},
            {parent_class_name},
            AQNWB::SPEC::{actual_cpp_namespace_name}::namespaceName)
        """
        
    header += f"""
}};

}} // namespace {cpp_namespace_name}
"""

    return header


def generate_implementation_file(
    namespace: SpecNamespace, neurodata_type: Spec, all_types: Dict[str, Spec], type_to_namespace_map: Dict[str, str]
) -> str:
    """
    Generate C++ implementation file for a neurodata type.

    Parameters:
    namespace (SpecNamespace): The namespace object.
    neurodata_type (Spec): The neurodata type spec.
    all_types (Dict[str, Spec]): A dictionary of all neurodata types.
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.

    Returns:
    str: The generated C++ implementation file content.
    """
    type_name = neurodata_type.neurodata_type_def
    cpp_namespace_name = to_cpp_namespace_name(type_to_namespace_map[type_name])
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

    if class_name in ["Container", "Data"]:
        parent_class = "AQNWB::NWB::RegisteredType" # special case for container type to avoid registering Container as a subclass of Container

    # Get parent neurodata_type spec
    parent_neurodata_type_spec = all_types.get(parent_type) if parent_type else None

    # Get attributes, datasets, and groups
    cpp_initialize_src = render_initialize_method_cpp(
        class_name=class_name,
        parent_class_name=parent_class,
        neurodata_type=neurodata_type,
        type_to_namespace_map=type_to_namespace_map,
        parent_neurodata_type=parent_neurodata_type_spec
    )

    # Start building the implementation file
    impl = f"""#include "{class_name}.hpp"
#include "Utils.hpp"

using namespace {cpp_namespace_name};
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL({class_name})

// Constructor
{class_name}::{class_name}(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
    : {parent_class}(path, io)
{{
}}

{cpp_initialize_src}
"""

    return impl


def generate_test_app_cmake(output_dir: Path, app_name: str, cpp_files: List[str], script_dir: Path) -> str:
    """
    Generate CMakeLists.txt for the test app.

    Parameters:
    output_dir (Path): The output directory path.
    app_name (str): Name of the test app.
    cpp_files (List[str]): List of generated .cpp files.
    script_dir (Path): Directory where the schematype_to_aqnwb.py script is located.

    Returns:
    str: The generated CMakeLists.txt content.
    """
    # Convert file paths to use forward slashes for CMake
    cpp_files_cmake = [f.replace("\\", "/") for f in cpp_files]
    
    cmake_content = f"""cmake_minimum_required(VERSION 3.15)
project({app_name} VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find aqnwb package. The aqnwb_DIR must be set on the command line
# e.g. -Daqnwb_DIR=/path/to/aqnwb/install/lib/cmake/aqnwb
find_package(aqnwb REQUIRED)

# Find HDF5
find_package(HDF5 REQUIRED COMPONENTS CXX)

# Generated source files
set(GENERATED_SOURCES"""

    for cpp_file in cpp_files_cmake:
        cmake_content += f'\n    "${{CMAKE_CURRENT_SOURCE_DIR}}/../{cpp_file}"'

    cmake_content += f"""
)

# Add the executable
add_executable({app_name} 
    main.cpp
    ${{GENERATED_SOURCES}}
)

# Include directories
target_include_directories({app_name} PRIVATE 
    "${{CMAKE_CURRENT_SOURCE_DIR}}/.."
    "${{CMAKE_CURRENT_SOURCE_DIR}}/../spec"
    ${{HDF5_INCLUDE_DIRS}}
)

# Link libraries
target_link_libraries({app_name} 
    aqnwb::aqnwb
    ${{HDF5_CXX_LIBRARIES}}
)

# If on Windows, link bcrypt
if(WIN32)
    target_link_libraries({app_name} bcrypt)
endif()

# Set the output directory
set_target_properties({app_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${{CMAKE_CURRENT_BINARY_DIR}}/bin
)

# Install rules
install(TARGETS {app_name}
    RUNTIME DESTINATION bin
)
"""
    return cmake_content


def generate_test_app_main(neurodata_types: Dict[str, Spec], type_to_namespace_map: Dict[str, str], type_to_file_map: Dict[str, Path]) -> str:
    """
    Generate main.cpp for the test app that instantiates all types.

    Parameters:
    neurodata_types (Dict[str, Spec]): Dictionary of all neurodata types.
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.
    type_to_file_map (Dict[str, Path]): Mapping of types to their source schema files.

    Returns:
    str: The generated main.cpp content.
    """
    # Collect includes and namespace information
    includes = set()
    namespace_types = {}
    
    for type_name, spec in neurodata_types.items():
        namespace = type_to_namespace_map.get(type_name, "core")
        cpp_namespace = to_cpp_namespace_name(namespace)
        
        if cpp_namespace not in namespace_types:
            namespace_types[cpp_namespace] = []
        namespace_types[cpp_namespace].append(type_name)
        
        # Generate correct include path accounting for folder structure
        source_file = type_to_file_map.get(type_name, Path("core.yaml"))
        subfolder_name = get_schema_subfolder_name(source_file)
        include_path = f"{namespace}/{subfolder_name}/{type_name}.hpp"
        includes.add(f'#include "{include_path}"')
    
    # Generate the main.cpp content
    main_content = """#include <iostream>
#include <memory>
#include <string>
#include "io/hdf5/HDF5IO.hpp"
#include "Utils.hpp"
#include "nwb/RegisteredType.hpp"

"""
    
    # Add all includes
    for include in sorted(includes):
        main_content += include + "\n"
    
    main_content += """
int main(int argc, char* argv[])
{
    std::cout << "Starting C++ class compilation test..." << std::endl;
    
    // Create a temporary HDF5 file for testing
    std::string testFilePath = "test_compilation.nwb";
    
    try {
        // Create an IO object
        auto io = AQNWB::createIO("HDF5", testFilePath);
        io->open(AQNWB::IO::FileMode::Overwrite);
        
        std::cout << "Testing instantiation of all generated C++ classes:" << std::endl;
        
"""
    
    # Generate instantiation code for each type
    for cpp_namespace, types in namespace_types.items():
        main_content += f"        // Testing {cpp_namespace} namespace types\n"
        for type_name in types:
            main_content += f"""        try {{
            std::string {type_name.lower()}_path = "/test_{type_name.lower()}";
            auto {type_name.lower()}_obj = AQNWB::NWB::RegisteredType::create<{cpp_namespace}::{type_name}>({type_name.lower()}_path, io);
            std::cout << "  ✓ {type_name} instantiated successfully" << std::endl;
        }} catch (const std::exception& e) {{
            std::cout << "  ✗ {type_name} instantiation failed: " << e.what() << std::endl;
        }}

"""
    
    main_content += """        // Close the IO
        io->close();
        
        std::cout << "Compilation test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
"""
    
    return main_content


def generate_test_app_readme(app_name: str, schema_file: str) -> str:
    """
    Generate README.md for the test app.

    Parameters:
    app_name (str): Name of the test app.
    schema_file (str): Path to the schema file used.

    Returns:
    str: The generated README.md content.
    """
    readme_content = f"""# {app_name}

This is an automatically generated test application to verify that all C++ classes generated from the NWB schema can be compiled and instantiated.

## Generated from Schema
- Schema file: `{schema_file}`
- Generated on: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

## Purpose
This test app serves to:
1. Verify that all generated C++ classes compile without errors
2. Test basic instantiation of each class using `std::make_shared<TypeName>(path, io)`
3. Provide a simple compilation check for the generated code

## Building and Running

### Prerequisites
- CMake 3.15 or higher
- C++17 compatible compiler
- HDF5 library
- Built aqnwb library

### Build Instructions
```bash
mkdir build
cd build
cmake ..
make
```

### Run the Test
```bash
./bin/{app_name}
```

The application will attempt to instantiate all generated C++ classes and report success or failure for each type.

## Notes
- This is a minimal test that only checks compilation and basic instantiation
- The test creates a temporary HDF5 file for testing purposes
- Each class is instantiated with a test path and shared IO object
- The test does not perform full functional testing of the classes
"""
    return readme_content


def generate_test_app(
    output_dir: Path, 
    schema_file: str, 
    neurodata_types: Dict[str, Spec], 
    type_to_namespace_map: Dict[str, str],
    type_to_file_map: Dict[str, Path],
    cpp_files: List[str]
) -> None:
    """
    Generate a complete test application for verifying compilation of generated classes.

    Parameters:
    output_dir (Path): The output directory containing generated files.
    schema_file (str): Path to the schema file used.
    neurodata_types (Dict[str, Spec]): Dictionary of all neurodata types.
    type_to_namespace_map (Dict[str, str]): Mapping of types to their namespaces.
    type_to_file_map (Dict[str, Path]): Mapping of types to their source schema files.
    cpp_files (List[str]): List of generated .cpp files relative to output_dir.

    Returns:
    None
    """
    app_name = "schema_compilation_test"
    test_app_dir = output_dir / "test_app"
    
    try:
        # Create test app directory
        test_app_dir.mkdir(parents=True, exist_ok=True)
        logger.info(f"Created test app directory: {test_app_dir}")
        
        logger.info(f"Using {len(cpp_files)} .cpp files for test app")
        
        # Generate CMakeLists.txt
        script_dir = Path(__file__).parent
        cmake_content = generate_test_app_cmake(output_dir, app_name, cpp_files, script_dir)
        cmake_path = test_app_dir / "CMakeLists.txt"
        with open(cmake_path, "w") as f:
            f.write(cmake_content)
        logger.info(f"Generated CMakeLists.txt: {cmake_path}")
        
        # Generate main.cpp
        main_content = generate_test_app_main(neurodata_types, type_to_namespace_map, type_to_file_map)
        main_path = test_app_dir / "main.cpp"
        with open(main_path, "w") as f:
            f.write(main_content)
        logger.info(f"Generated main.cpp: {main_path}")
        
        # Generate README.md
        readme_content = generate_test_app_readme(app_name, schema_file)
        readme_path = test_app_dir / "README.md"
        with open(readme_path, "w") as f:
            f.write(readme_content)
        logger.info(f"Generated README.md: {readme_path}")
        
        logger.info(f"Test application generated successfully in: {test_app_dir}")
        
    except Exception as e:
        logger.error(f"Failed to generate test application: {e}")


def setup_parser(parser):
    """
    Set up argument parser for the script.
    """
    parser.add_argument(
        "schema_file", help="Path to the namespace schema file (JSON or YAML)"
    )
    parser.add_argument("output_dir", help="Directory to output the generated code")
    parser.add_argument(
        "--generate-test-app",
        action="store_true",
        help="Generate a test application to verify compilation of all generated classes"
    )

def main(args) -> None:
    """
    Main function to parse arguments and generate code.

    This function sets up argument parsing, parses the schema file, optionally overrides the namespace,
    creates the output directory, and generates code for each neurodata type.

    Parameters:
    None

    Returns:
    None
    """
    try:
        logger.info(f"Parsing schema file: {args.schema_file}")
        namespace, neurodata_types, type_to_file_map, type_to_namespace_map = parse_schema_file(Path(args.schema_file))
        logger.info(f"Successfully parsed schema file: {args.schema_file}")
    except Exception as e:
        logger.error(f"Failed to parse schema file {args.schema_file}: {e}")
        return

    # Create output directory if it doesn't exist
    try:
        logger.info(f"Creating output directory: {args.output_dir}")
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        logger.info(f"Successfully created output directory: {args.output_dir}")
    except Exception as e:
        logger.error(f"Failed to create output directory {args.output_dir}: {e}")
        return

    # Track generated files for test app
    generated_cpp_files = []
    
    # Generate code for each neurodata type
    for type_name, neurodata_type in neurodata_types.items():
        class_name = type_name
        logger.info(f"Processing neurodata_type: {class_name}")
        
        # Get the original namespace for this type (not the including namespace)
        original_namespace = type_to_namespace_map.get(type_name, namespace.name)
        
        # Create namespace folder for the original namespace
        namespace_folder = output_dir / original_namespace
        try:
            namespace_folder.mkdir(parents=True, exist_ok=True)
            logger.info(f"    Created namespace folder: {original_namespace}")
        except Exception as e:
            logger.error(f"    Failed to create namespace folder {original_namespace}: {e}")
            continue
        
        # Get the source schema file for this type and create subfolder within namespace folder
        source_file = type_to_file_map.get(type_name, Path(args.schema_file))
        subfolder_name = get_schema_subfolder_name(source_file)
        type_output_dir = namespace_folder / subfolder_name
        
        # Create the subfolder if it doesn't exist
        try:
            type_output_dir.mkdir(parents=True, exist_ok=True)
            logger.info(f"    Created subfolder: {original_namespace}/{subfolder_name}")
        except Exception as e:
            logger.error(f"    Failed to create subfolder {original_namespace}/{subfolder_name}: {e}")
            continue
        
        try:
            header_file_name = f"{class_name}.hpp"
            logger.info(f"    Generating header file: {header_file_name}")
            header_file = generate_header_file(
                namespace, neurodata_type, neurodata_types, type_to_file_map, type_to_namespace_map
            )
            header_path = type_output_dir / header_file_name
            with open(header_path, "w") as f:
                f.write(header_file)
        except Exception as e:
            logger.error(f"    Failed to generate header file {header_path}: {e}")

        try:
            cpp_file_name = f"{class_name}.cpp"
            logger.info(f"    Generating implementation file: {cpp_file_name}")
            impl_file = generate_implementation_file(
                namespace, neurodata_type, neurodata_types, type_to_namespace_map
            )
            impl_path = type_output_dir / cpp_file_name
            with open(impl_path, "w") as f:
                f.write(impl_file)
            
            # Track the generated cpp file relative to output_dir for test app
            relative_cpp_path = impl_path.relative_to(output_dir)
            generated_cpp_files.append(str(relative_cpp_path))
            
        except Exception as e:
            logger.error(f"   Failed to generate implementation file {impl_path}: {e}")
    logger.info(f"Generated {len(neurodata_types)} neurodata types in total.")

    # Generate test app if requested
    if args.generate_test_app:
        logger.info("Generating test application...")
        generate_test_app(
            output_dir=output_dir,
            schema_file=args.schema_file,
            neurodata_types=neurodata_types,
            type_to_namespace_map=type_to_namespace_map,
            type_to_file_map=type_to_file_map,
            cpp_files=generated_cpp_files
        )

    # Generate test app if requested
    if args.generate_test_app:
        logger.info("Generating test application...")
        generate_test_app(
            output_dir=output_dir,
            schema_file=args.schema_file,
            neurodata_types=neurodata_types,
            type_to_namespace_map=type_to_namespace_map,
            type_to_file_map=type_to_file_map,
            cpp_files=generated_cpp_files
        )

    logger.info("Script execution completed.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate C++ code from NWB schema files."
    )
    setup_parser(parser)
    args = parser.parse_args()
    main(args)
