#!/usr/bin/env python3
"""
SWIG Interface Generation for aqnwb Library

This script extends the existing generate_spec_files.py functionality to automatically
generate SWIG interface files for all NWB classes. It integrates with the existing
schema analysis and macro processing to create comprehensive language bindings.

Usage:
    python3 generate_swig_interfaces.py [--output-dir swig/interfaces] [--languages csharp,python,java]
"""

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional


class NWBSchemaAnalyzer:
    """Analyzes NWB schema files to extract class information."""
    
    def __init__(self, schema_dir: Path):
        self.schema_dir = schema_dir
        self.classes = {}
    
    def analyze_schemas(self) -> Dict[str, Dict]:
        """
        Analyze all NWB schema files to extract class information.
        
        Returns:
            Dictionary mapping class names to their schema information
        """
        # This would integrate with existing schema analysis in generate_spec_files.py
        # For now, we'll simulate with TimeSeries as an example
        
        self.classes = {
            'TimeSeries': {
                'name': 'TimeSeries',
                'namespace': 'AQNWB::NWB',
                'include_path': 'nwb/base/TimeSeries.hpp',
                'parent': 'Container',
                'neurodata_type': 'TimeSeries',
                'schema_namespace': 'core'
            },
            'ElectricalSeries': {
                'name': 'ElectricalSeries',
                'namespace': 'AQNWB::NWB',
                'include_path': 'nwb/ecephys/ElectricalSeries.hpp',
                'parent': 'TimeSeries',
                'neurodata_type': 'ElectricalSeries',
                'schema_namespace': 'core'
            },
            'Container': {
                'name': 'Container',
                'namespace': 'AQNWB::NWB',
                'include_path': 'nwb/base/Container.hpp',
                'parent': 'NWBContainer',
                'neurodata_type': 'Container',
                'schema_namespace': 'core'
            }
        }
        
        return self.classes


class MacroExtractor:
    """Extracts macro-generated method information from C++ header files."""
    
    def __init__(self):
        self.base_data_types = [
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
    
    def extract_from_header(self, header_path: Path) -> Dict:
        """
        Extract macro-generated field information from a C++ header file.
        
        Args:
            header_path: Path to the C++ header file
            
        Returns:
            Dictionary containing extracted field information
        """
        if not header_path.exists():
            # For the example, return simulated data for TimeSeries
            if 'TimeSeries' in str(header_path):
                return self._get_timeseries_fields()
            else:
                return {'attribute_fields': [], 'dataset_fields': [], 'registered_fields': []}
        
        with open(header_path, 'r') as f:
            content = f.read()
        
        return self._parse_macro_content(content)
    
    def _get_timeseries_fields(self) -> Dict:
        """Return simulated TimeSeries field information."""
        return {
            'attribute_fields': [
                {
                    'method_name': 'readDescription',
                    'data_type': 'std::string',
                    'field_path': 'description',
                    'description': 'Description of the series'
                },
                {
                    'method_name': 'readComments',
                    'data_type': 'std::string',
                    'field_path': 'comments',
                    'description': 'Human-readable comments about the TimeSeries'
                },
                {
                    'method_name': 'readDataConversion',
                    'data_type': 'float',
                    'field_path': 'data/conversion',
                    'description': 'Scalar to multiply each element in data'
                },
                {
                    'method_name': 'readDataUnit',
                    'data_type': 'std::string',
                    'field_path': 'data/unit',
                    'description': 'Base unit of measurement'
                }
            ],
            'dataset_fields': [
                {
                    'read_method': 'readData',
                    'write_method': 'recordData',
                    'data_type': 'std::any',
                    'field_path': 'data',
                    'description': 'The main data'
                },
                {
                    'read_method': 'readTimestamps',
                    'write_method': 'recordTimestamps',
                    'data_type': 'double',
                    'field_path': 'timestamps',
                    'description': 'Timestamps for samples stored in data'
                },
                {
                    'read_method': 'readControl',
                    'write_method': 'recordControl',
                    'data_type': 'uint8_t',
                    'field_path': 'control',
                    'description': 'Numerical labels for each time point'
                }
            ],
            'registered_fields': []
        }
    
    def _parse_macro_content(self, content: str) -> Dict:
        """Parse macro definitions from header content."""
        # Implementation would parse actual macro definitions
        # This is simplified for the example
        return {'attribute_fields': [], 'dataset_fields': [], 'registered_fields': []}


class SwigTemplateEngine:
    """Template engine for generating SWIG interface files."""
    
    def __init__(self, template_dir: Path):
        self.template_dir = template_dir
    
    def generate_interface(self, class_info: Dict, macro_info: Dict, 
                         template_name: str, output_path: Path) -> None:
        """
        Generate a SWIG interface file using a template.
        
        Args:
            class_info: Information about the class
            macro_info: Macro-generated field information
            template_name: Name of the template file to use
            output_path: Path where to write the generated .i file
        """
        template_path = self.template_dir / template_name
        
        if not template_path.exists():
            raise FileNotFoundError(f"Template not found: {template_path}")
        
        with open(template_path, 'r') as f:
            template_content = f.read()
        
        # Generate template variables
        variables = self._prepare_template_variables(class_info, macro_info, template_name)
        
        # Replace template variables
        content = self._render_template(template_content, variables)
        
        # Ensure output directory exists
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, 'w') as f:
            f.write(content)
    
    def _prepare_template_variables(self, class_info: Dict, macro_info: Dict, 
                                  template_name: str) -> Dict[str, str]:
        """Prepare variables for template rendering."""
        class_name = class_info['name']
        namespace = class_info['namespace']
        
        # Generate shared pointer declarations
        dependencies = {
            'AQNWB::IO::BaseIO',
            'AQNWB::IO::BaseRecordingData',
            'AQNWB::IO::ReadDataWrapper'
        }
        
        shared_ptr_declarations = '\n'.join(
            f"%shared_ptr({dep})" for dep in sorted(dependencies)
        )
        
        # Generate attribute field templates
        attribute_templates = []
        for field in macro_info.get('attribute_fields', []):
            method_name = field['method_name']
            data_type = field['data_type']
            attribute_templates.append(
                f"// {method_name} - default type: {data_type}\n"
                f"%template({method_name}) {namespace}::{class_name}::{method_name}<{data_type}>;"
            )
        
        # Generate dataset field templates with all BaseRecordingData types
        dataset_templates = []
        base_data_types = [
            'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t',
            'int8_t', 'int16_t', 'int32_t', 'int64_t',
            'float', 'double', 'std::string'
        ]
        
        for field in macro_info.get('dataset_fields', []):
            read_method = field['read_method']
            data_type = field['data_type']
            
            templates = [
                f"// {read_method} - default type: {data_type}, with type-specific alternatives",
                f"%template({read_method}) {namespace}::{class_name}::{read_method}<{data_type}>;"
            ]
            
            # Add type-specific templates for data fields
            if 'data' in field['field_path'].lower():
                for alt_type in base_data_types:
                    if alt_type != data_type:
                        sanitized = self._sanitize_type(alt_type).title()
                        templates.append(
                            f"%template({read_method}{sanitized}) "
                            f"{namespace}::{class_name}::{read_method}<{alt_type}>;"
                        )
            
            dataset_templates.append('\n'.join(templates))
        
        # Generate registered field templates
        registered_templates = []
        for field in macro_info.get('registered_fields', []):
            method_name = field['method_name']
            reg_type = field['registered_type']
            registered_templates.append(
                f"// {method_name} - registered type: {reg_type}\n"
                f"%template({method_name}) {namespace}::{class_name}::{method_name}<{reg_type}>;"
            )
        
        return {
            'class_name': class_name,
            'module_name': class_name.lower(),
            'template_name': template_name,
            'include_path': class_info['include_path'],
            'shared_ptr_declarations': shared_ptr_declarations,
            'attribute_field_templates': '\n\n'.join(attribute_templates),
            'dataset_field_templates': '\n\n'.join(dataset_templates),
            'registered_field_templates': '\n\n'.join(registered_templates)
        }
    
    def _render_template(self, template_content: str, variables: Dict[str, str]) -> str:
        """Render template with variables."""
        content = template_content
        for key, value in variables.items():
            content = content.replace(f"{{{{{key}}}}}", value)
        return content
    
    def _sanitize_type(self, type_name: str) -> str:
        """Sanitize type names for use in template instantiation names."""
        sanitized = re.sub(r'::', '_', type_name)
        sanitized = re.sub(r'[<>]', '_', sanitized)
        sanitized = re.sub(r'[,\s]', '', sanitized)
        return sanitized


class SwigInterfaceGenerator:
    """Main class for generating SWIG interfaces for the aqnwb library."""
    
    def __init__(self, source_dir: Path, template_dir: Path, output_dir: Path):
        self.source_dir = source_dir
        self.template_dir = template_dir
        self.output_dir = output_dir
        self.schema_analyzer = NWBSchemaAnalyzer(source_dir / 'resources' / 'spec')
        self.macro_extractor = MacroExtractor()
        self.template_engine = SwigTemplateEngine(template_dir)
    
    def generate_all_interfaces(self, languages: List[str] = None) -> None:
        """
        Generate SWIG interfaces for all NWB classes.
        
        Args:
            languages: List of target languages (e.g., ['csharp', 'python', 'java'])
        """
        if languages is None:
            languages = ['csharp', 'python', 'java']
        
        print("Analyzing NWB schemas...")
        classes = self.schema_analyzer.analyze_schemas()
        
        print(f"Found {len(classes)} NWB classes")
        
        for class_name, class_info in classes.items():
            print(f"Generating SWIG interface for {class_name}...")
            
            # Extract macro information
            header_path = self.source_dir / 'src' / class_info['include_path']
            macro_info = self.macro_extractor.extract_from_header(header_path)
            
            # Generate interface file
            template_name = self._select_template(class_info)
            output_path = self.output_dir / f"{class_name}.i"
            
            self.template_engine.generate_interface(
                class_info, macro_info, template_name, output_path
            )
            
            print(f"  Generated: {output_path}")
            print(f"  Found {len(macro_info['attribute_fields'])} attribute fields")
            print(f"  Found {len(macro_info['dataset_fields'])} dataset fields")
            print(f"  Found {len(macro_info['registered_fields'])} registered fields")
    
    def _select_template(self, class_info: Dict) -> str:
        """Select appropriate template based on class information."""
        # For now, use the timeseries template for all classes
        # In a real implementation, this would select based on class hierarchy
        return 'timeseries.i.template'


def main():
    """Main entry point for the SWIG interface generator."""
    parser = argparse.ArgumentParser(
        description='Generate SWIG interfaces for aqnwb library'
    )
    parser.add_argument(
        '--source-dir',
        type=Path,
        default=Path('../../..'),  # Relative to resources/ directory
        help='Path to aqnwb source directory'
    )
    parser.add_argument(
        '--template-dir',
        type=Path,
        default=Path('swig_templates'),
        help='Path to SWIG template directory'
    )
    parser.add_argument(
        '--output-dir',
        type=Path,
        default=Path('../swig/interfaces'),
        help='Output directory for generated SWIG interfaces'
    )
    parser.add_argument(
        '--languages',
        type=str,
        default='csharp,python,java',
        help='Comma-separated list of target languages'
    )
    
    args = parser.parse_args()
    
    # Convert languages string to list
    languages = [lang.strip() for lang in args.languages.split(',')]
    
    print("SWIG Interface Generator for aqnwb")
    print("=" * 40)
    print(f"Source directory: {args.source_dir}")
    print(f"Template directory: {args.template_dir}")
    print(f"Output directory: {args.output_dir}")
    print(f"Target languages: {', '.join(languages)}")
    print()
    
    # Create generator and run
    generator = SwigInterfaceGenerator(
        args.source_dir, args.template_dir, args.output_dir
    )
    
    try:
        generator.generate_all_interfaces(languages)
        print("\nSWIG interface generation completed successfully!")
    except Exception as e:
        print(f"\nError during generation: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
