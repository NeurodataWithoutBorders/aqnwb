#!/usr/bin/env python3
"""
Example implementation of automatic SWIG interface generation for TimeSeries class.

This script demonstrates how the proposed SWIG binding generation would work
by analyzing the TimeSeries class and generating appropriate SWIG interface files.
"""

import json
import re
from pathlib import Path
from typing import Dict, List, Set, Tuple


class MacroAnalyzer:
    """Analyzes C++ header files to extract macro-generated method information."""
    
    def __init__(self):
        self.attribute_fields = []
        self.dataset_fields = []
        self.registered_fields = []
    
    def analyze_header(self, header_content: str) -> Dict:
        """
        Analyze a C++ header file to extract macro-generated field information.
        
        Args:
            header_content: Content of the C++ header file
            
        Returns:
            Dictionary containing extracted field information
        """
        # Extract DEFINE_ATTRIBUTE_FIELD macros
        attr_pattern = r'DEFINE_ATTRIBUTE_FIELD\(\s*(\w+),\s*([^,]+),\s*"([^"]+)",\s*([^)]+)\)'
        attr_matches = re.findall(attr_pattern, header_content, re.MULTILINE | re.DOTALL)
        
        for match in attr_matches:
            method_name, data_type, field_path, description = match
            self.attribute_fields.append({
                'method_name': method_name.strip(),
                'data_type': data_type.strip(),
                'field_path': field_path.strip(),
                'description': description.strip()
            })
        
        # Extract DEFINE_DATASET_FIELD macros
        dataset_pattern = r'DEFINE_DATASET_FIELD\(\s*(\w+),\s*(\w+),\s*([^,]+),\s*"([^"]+)",\s*([^)]+)\)'
        dataset_matches = re.findall(dataset_pattern, header_content, re.MULTILINE | re.DOTALL)
        
        for match in dataset_matches:
            read_name, write_name, data_type, field_path, description = match
            self.dataset_fields.append({
                'read_method': read_name.strip(),
                'write_method': write_name.strip(),
                'data_type': data_type.strip(),
                'field_path': field_path.strip(),
                'description': description.strip()
            })
        
        # Extract DEFINE_REGISTERED_FIELD macros
        reg_pattern = r'DEFINE_REGISTERED_FIELD\(\s*(\w+),\s*([^,]+),\s*"([^"]+)",\s*([^)]+)\)'
        reg_matches = re.findall(reg_pattern, header_content, re.MULTILINE | re.DOTALL)
        
        for match in reg_matches:
            method_name, registered_type, field_path, description = match
            self.registered_fields.append({
                'method_name': method_name.strip(),
                'registered_type': registered_type.strip(),
                'field_path': field_path.strip(),
                'description': description.strip()
            })
        
        return {
            'attribute_fields': self.attribute_fields,
            'dataset_fields': self.dataset_fields,
            'registered_fields': self.registered_fields
        }


class SwigInterfaceGenerator:
    """Generates SWIG interface files from analyzed C++ class information."""
    
    def __init__(self):
        self.dependencies = set()
    
    def generate_interface(self, class_info: Dict, macro_info: Dict, output_path: Path) -> None:
        """
        Generate a SWIG interface file for a given class.
        
        Args:
            class_info: Information about the class (name, namespace, etc.)
            macro_info: Information extracted from macro analysis
            output_path: Path where to write the generated .i file
        """
        class_name = class_info['name']
        namespace = class_info.get('namespace', 'AQNWB::NWB')
        include_path = class_info.get('include_path', f'nwb/base/{class_name}.hpp')
        
        # Collect all referenced types for shared_ptr declarations
        self._collect_dependencies(macro_info)
        
        content = self._generate_interface_content(
            class_name, namespace, include_path, macro_info
        )
        
        with open(output_path, 'w') as f:
            f.write(content)
    
    def _collect_dependencies(self, macro_info: Dict) -> None:
        """Collect all types that need shared_ptr declarations."""
        # Add common base types
        self.dependencies.update([
            'AQNWB::IO::BaseIO',
            'AQNWB::IO::BaseRecordingData',
            'AQNWB::IO::ReadDataWrapper'
        ])
        
        # Add types from registered fields
        for field in macro_info.get('registered_fields', []):
            self.dependencies.add(field['registered_type'])
    
    def _generate_interface_content(self, class_name: str, namespace: str, 
                                  include_path: str, macro_info: Dict) -> str:
        """Generate the complete SWIG interface file content."""
        
        content = f"""// Auto-generated SWIG interface for {class_name}
// Generated by generate_timeseries_swig.py
// Provides default methods + type-specific alternatives

%module aqnwb_{class_name.lower()}

%{{
#include "{include_path}"
%}}

// Include standard SWIG libraries
%include <std_string.i>
%include <std_vector.i>
%include <std_shared_ptr.i>
%include <std_any.i>

// Shared pointer declarations for referenced types
"""
        
        for dep in sorted(self.dependencies):
            content += f"%shared_ptr({dep})\n"
        
        content += f"\n// Template instantiations for macro-generated methods\n"
        content += f"// Provides both default convenience and type-specific performance\n\n"
        
        # Generate template instantiations for attribute fields
        for field in macro_info.get('attribute_fields', []):
            method_name = field['method_name']
            data_type = field['data_type']
            content += f"// {method_name} - default type: {data_type}\n"
            content += f"%template({method_name}) "
            content += f"{namespace}::{class_name}::{method_name}<{data_type}>;\n"
        
        # Generate template instantiations for dataset fields with multiple types
        for field in macro_info.get('dataset_fields', []):
            read_method = field['read_method']
            data_type = field['data_type']
            content += f"\n// {read_method} - default type: {data_type}, with type-specific alternatives\n"
            
            # Default type instantiation (clean method name)
            content += f"%template({read_method}) "
            content += f"{namespace}::{class_name}::{read_method}<{data_type}>;\n"
            
            # Add all data types supported by BaseRecordingData for data fields
            if 'data' in field['field_path'].lower():
                # Complete list from BaseDataType::Type enum in BaseIO.hpp
                supported_types = [
                    'uint8_t',    # T_U8 - Unsigned 8-bit integer
                    'uint16_t',   # T_U16 - Unsigned 16-bit integer  
                    'uint32_t',   # T_U32 - Unsigned 32-bit integer
                    'uint64_t',   # T_U64 - Unsigned 64-bit integer
                    'int8_t',     # T_I8 - Signed 8-bit integer
                    'int16_t',    # T_I16 - Signed 16-bit integer
                    'int32_t',    # T_I32 - Signed 32-bit integer
                    'int64_t',    # T_I64 - Signed 64-bit integer
                    'float',      # T_F32 - 32-bit floating point
                    'double',     # T_F64 - 64-bit floating point
                    'std::string' # T_STR/V_STR - String types
                ]
                for alt_type in supported_types:
                    if alt_type != data_type:  # Don't duplicate the default type
                        sanitized = self._sanitize_type(alt_type).title()
                        content += f"%template({read_method}{sanitized}) "
                        content += f"{namespace}::{class_name}::{read_method}<{alt_type}>;\n"
        
        # Generate template instantiations for registered fields
        for field in macro_info.get('registered_fields', []):
            method_name = field['method_name']
            reg_type = field['registered_type']
            content += f"\n// {method_name} - registered type: {reg_type}\n"
            content += f"%template({method_name}) "
            content += f"{namespace}::{class_name}::{method_name}<{reg_type}>;\n"
        
        content += f"\n// Include the actual header file\n"
        content += f"%include \"{include_path}\"\n"
        
        return content
    
    def _sanitize_type(self, type_name: str) -> str:
        """Sanitize type names for use in template instantiation names."""
        # Remove namespace qualifiers and special characters
        sanitized = re.sub(r'::', '_', type_name)
        sanitized = re.sub(r'[<>]', '_', sanitized)
        sanitized = re.sub(r'[,\s]', '', sanitized)
        return sanitized


def simulate_timeseries_analysis():
    """
    Simulate the analysis of TimeSeries class and generate SWIG interface.
    This demonstrates what the actual implementation would do.
    """
    
    # Simulated TimeSeries header content (based on actual aqnwb TimeSeries.hpp)
    timeseries_header = '''
    class TimeSeries : public Container
    {
    public:
        REGISTER_SUBCLASS(TimeSeries, "core")
        
        // Define the data fields to expose for lazy read access
        DEFINE_ATTRIBUTE_FIELD(readDescription,
                               std::string,
                               "description",
                               Description of the series)

        DEFINE_ATTRIBUTE_FIELD(readComments,
                               std::string,
                               "comments",
                               Human-readable comments about the TimeSeries)

        DEFINE_DATASET_FIELD(readData, recordData, std::any, "data", The main data)

        DEFINE_ATTRIBUTE_FIELD(readDataConversion,
                               float,
                               "data/conversion",
                               Scalar to multiply each element in data)

        DEFINE_ATTRIBUTE_FIELD(readDataUnit,
                              std::string,
                              "data/unit",
                              Base unit of measurement)

        DEFINE_DATASET_FIELD(readTimestamps,
                             recordTimestamps,
                             double,
                             "timestamps",
                             Timestamps for samples stored in data)

        DEFINE_DATASET_FIELD(readControl,
                             recordControl,
                             uint8_t,
                             "control",
                             Numerical labels for each time point)
    };
    '''
    
    # Analyze the header
    analyzer = MacroAnalyzer()
    macro_info = analyzer.analyze_header(timeseries_header)
    
    # Class information (would come from schema analysis in real implementation)
    class_info = {
        'name': 'TimeSeries',
        'namespace': 'AQNWB::NWB',
        'include_path': 'nwb/base/TimeSeries.hpp',
        'parent': 'Container'
    }
    
    # Generate SWIG interface
    generator = SwigInterfaceGenerator()
    output_path = Path('swig/interfaces/TimeSeries.i')
    generator.generate_interface(class_info, macro_info, output_path)
    
    print(f"Generated SWIG interface: {output_path}")
    print(f"Found {len(macro_info['attribute_fields'])} attribute fields")
    print(f"Found {len(macro_info['dataset_fields'])} dataset fields")
    print(f"Found {len(macro_info['registered_fields'])} registered fields")
    
    return macro_info, class_info


if __name__ == "__main__":
    print("TimeSeries SWIG Interface Generation Example")
    print("=" * 50)
    
    macro_info, class_info = simulate_timeseries_analysis()
    
    print("\nExtracted Macro Information:")
    print("-" * 30)
    
    print("\nAttribute Fields:")
    for field in macro_info['attribute_fields']:
        print(f"  - {field['method_name']}: {field['data_type']} -> {field['field_path']}")
    
    print("\nDataset Fields:")
    for field in macro_info['dataset_fields']:
        print(f"  - {field['read_method']}/{field['write_method']}: {field['data_type']} -> {field['field_path']}")
    
    print("\nRegistered Fields:")
    for field in macro_info['registered_fields']:
        print(f"  - {field['method_name']}: {field['registered_type']} -> {field['field_path']}")
    
    print(f"\nSWIG interface file generated successfully!")
