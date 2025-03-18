import json
import argparse
from pathlib import Path
from ruamel.yaml import YAML
import logging
from typing import List, Dict, Union

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def load_file(filepath: Path) -> Union[Dict, List]:
    """
    Load a YAML or JSON file and return its content.
    
    Args:
        filepath (Path): Path to the file.
    
    Returns:
        dict or list: Parsed content of the file.
    """
    if filepath.suffix in ['.yaml', '.yml']:
        yaml = YAML(typ='safe')
        try:
            with open(filepath) as f:
                return yaml.load(f)
        except Exception as e:
            logger.error(f"Failed to load YAML file {filepath}: {e}")
            raise
    elif filepath.suffix == '.json':
        try:
            with open(filepath) as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load JSON file {filepath}: {e}")
            raise
    else:
        logger.error(f"Unsupported file type for {filepath}")
        raise ValueError(f"Unsupported file type for {filepath}")

def generate_header_file(ns: Dict, header_file: Path, var_names: List[str], var_contents: Dict[str, str]) -> None:
    """
    Generate the C++ header file for a given namespace.
    
    Args:
        ns (dict): Namespace information.
        header_file (Path): Path to the output header file.
        var_names (List[str]): List of variable names.
        var_contents (Dict[str, str]): Dictionary of variable contents.
    """
    logger.info(f"Generating header file: {header_file}")
    with open(header_file, 'w') as fo:
        fo.write('#pragma once\n\n')
        fo.write('#include <array>\n#include <string>\n#include <string_view>\n\n')
        fo.write(f'namespace AQNWB::SPEC::{ns["name"].upper().replace("-", "_")}\n{{\n\n')
        fo.write(f'const std::string version = "{ns["version"]}";\n\n')
        for name in var_names:
            fo.write(f'constexpr std::string_view {name} = R"delimiter(\n{var_contents[name]})delimiter";\n\n')
        fo.write(f'constexpr std::string_view namespaces = R"delimiter(\n{json.dumps({"namespaces": [ns]}, separators=(",", ":"))})delimiter";\n\n')
        fo.write(f'constexpr std::array<std::pair<std::string_view, std::string_view>, {len(var_names) + 1}>\n    specVariables {{{{\n')
        fo.write(''.join([f'  {{"{name.replace("_", ".")}", {name}}},\n' for name in var_names]))
        fo.write('  {"namespace", namespaces}\n')
        fo.write(f'}}}};\n}}  // namespace AQNWB::SPEC::{ns["name"].upper().replace("-", "_")}\n')

def process_schema_file(schema_file: Path, header_file: Path, var_names: List[str], var_contents: Dict[str, str], chunk_size: int) -> None:
    """
    Process a single schema file and append the generated C++ code to the header file.
    
    Args:
        schema_file (Path): Path to the schema file.
        header_file (Path): Path to the output header file.
        var_names (List[str]): List of variable names.
        var_contents (Dict[str, str]): Dictionary of variable contents.
        chunk_size (int): Size of the chunks for splitting large JSON strings.
    """
    logger.info(f"Processing schema file: {schema_file}")
    try:
        spec = load_file(schema_file)
    except Exception as e:
        logger.error(f"Failed to process schema file {schema_file}: {e}")
        return

    json_str = json.dumps(spec, separators=(',', ':'))
    var_name = schema_file.stem.replace('.', '_')
    if len(json_str) > chunk_size:
        chunks = [json_str[i:i + chunk_size] for i in range(0, len(json_str), chunk_size)]
        chunk_var_names = []
        for i, chunk in enumerate(chunks):
            chunk_var_name = f'{var_name}_part{i}'
            with open(header_file, 'a') as fo:
                fo.write(f'constexpr std::string_view {chunk_var_name} = R"delimiter({chunk})delimiter";\n')
            chunk_var_names.append(chunk_var_name)

        with open(header_file, 'a') as fo:
            fo.write(f'constexpr std::array<std::string_view, {len(chunks)}> {var_name}_parts = {{{", ".join(chunk_var_names)}}};\n')
            fo.write(f'constexpr std::size_t {var_name}_total_length = []() {{\n')
            fo.write(f'    std::size_t length = 0;\n')
            fo.write(f'    for (const auto& part : {var_name}_parts) {{\n')
            fo.write(f'        length += part.size();\n')
            fo.write(f'    }}\n')
            fo.write(f'    return length;\n')
            fo.write(f'}}();\n')
            fo.write(f'constexpr auto {var_name}_concatenate = []() {{\n')
            fo.write(f'    std::array<char, {var_name}_total_length + 1> result{{}};\n')
            fo.write(f'    std::size_t pos = 0;\n')
            fo.write(f'    for (const auto& part : {var_name}_parts) {{\n')
            fo.write(f'        for (char c : part) {{\n')
            fo.write(f'            result[pos++] = c;\n')
            fo.write(f'        }}\n')
            fo.write(f'    }}\n')
            fo.write(f'    result[pos] = \'\\0\';\n')
            fo.write(f'    return result;\n')
            fo.write(f'}}();\n')
            fo.write(f'constexpr std::string_view {var_name}({var_name}_concatenate.data(), {var_name}_concatenate.size() - 1);\n\n')
    else:
        with open(header_file, 'a') as fo:
            fo.write(f'constexpr std::string_view {var_name} = R"delimiter(\n{json_str})delimiter";\n\n')
    var_names.append(var_name)
    var_contents[var_name] = json_str

def process_namespace_file(namespace_file: Path, output_dir: Path, chunk_size: int) -> None:
    """
    Process a namespace file and generate the corresponding C++ header files.
    
    Args:
        namespace_file (Path): Path to the namespace file.
        output_dir (Path): Directory to output the generated header files.
        chunk_size (int): Size of the chunks for splitting large JSON strings.
    """
    logger.info(f"Processing namespace file: {namespace_file}")
    try:
        namespace = load_file(namespace_file)
    except Exception as e:
        logger.error(f"Failed to process namespace file {namespace_file}: {e}")
        return

    for ns in namespace['namespaces']:
        header_file = output_dir / f"{ns['name'].replace('-', '_')}.hpp"
        var_names = []
        var_contents = {}
        for s in ns['schema']:
            if 'source' in s:
                schema_file = namespace_file.parent / s['source']
                schema_file = schema_file.with_suffix('.json')  # Ensure the file has the correct extension
                process_schema_file(schema_file, header_file, var_names, var_contents, chunk_size)
        generate_header_file(ns, header_file, var_names, var_contents)

def process_schema_files(schema_dir: Path, output_dir: Path, chunk_size: int) -> None:
    """
    Process schema files in the specified directory and generate corresponding C++ header files.

    Args:
        schema_dir (Path): Directory containing the schema files.
        output_dir (Path): Directory to output the generated header files.
        chunk_size (int): Size of the chunks for splitting large JSON strings.
    """
    logger.info(f"Starting to process schema files in directory: {schema_dir}")
    for file in schema_dir.rglob(r"*namespace.*"):
        if file.suffix in ['.yaml', '.yml', '.json']:
            process_namespace_file(file, output_dir, chunk_size)
    logger.info(f"Finished processing schema files in directory: {schema_dir}")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process schema files.')
    parser.add_argument('schema_dir', type=Path, nargs='?', default=Path('./resources/schema/'), help='Directory containing the schema files')
    parser.add_argument('output_dir', type=Path, nargs='?', default=Path('./src/spec/'), help='Directory to output the generated header files')
    parser.add_argument('--chunk-size', type=int, default=16000, help='Size of the chunks for splitting large JSON strings')
    args = parser.parse_args()

    process_schema_files(args.schema_dir, args.output_dir, args.chunk_size)
