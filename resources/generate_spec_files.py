import json
import os

from pathlib import Path
from ruamel.yaml import YAML

# TODO - setup git submodule or cloning
schema_dir = Path('./resources/schema/')

for file in schema_dir.rglob(r"*namespace.yaml"):
    # load file
    yaml = YAML(typ='safe')
    with open(file) as f:
        namespace = yaml.load(f)

    # get all the sources
    for i, ns in enumerate(namespace['namespaces']):
        spec_dir = Path(f"./resources/spec/{ns['name']}/{ns['version']}")
        spec_dir.mkdir(parents=True, exist_ok=True)

        # create header file
        header_file = Path(f"./src/spec/{ns['name'].replace('-', '_')}.hpp").resolve()
        with open(header_file, 'w') as fo:
            fo.write('#pragma once\n\n')
            fo.write('#include <array>\n#include <string>\n#include <string_view>\n\n')
            fo.write(f'namespace AQNWB::SPEC::{ns["name"].upper().replace("-", "_")}\n{{\n\n')
            fo.write(f'const std::string version = "{ns["version"]}";\n\n')

        # load and convert schema files
        var_names = []
        for s in ns['schema']:
            if 'source' in s:
                # load file
                schema_file = file.parent / s['source']
                with open(schema_file) as f:
                    spec = yaml.load(f)

                # convert to cpp string
                print(f'Generating file {header_file} - {s["source"]}')
                json_str = json.dumps(spec, separators=(',', ':'))
                chunk_size = 16640  # Adjust the chunk size as needed
                if len(json_str) > chunk_size:
                    # Split string into chunks if needed
                    chunks = [json_str[i:i + chunk_size] for i in range(0, len(json_str), chunk_size)]
                    
                    var_name = s['source'].replace('.yaml', '').replace('.', '_')
                    chunk_var_names = []
                    for i, chunk in enumerate(chunks):
                        chunk_var_name = f'{var_name}_part{i}'
                        with open(header_file, 'a') as fo:
                            fo.write(f'constexpr std::string_view {chunk_var_name} = R"delimiter({chunk})delimiter";\n')
                        chunk_var_names.append(chunk_var_name)

                    # Concatenate chunks at compile-time
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
                    var_names.append(var_name)
                else:
                    with open(header_file, 'a') as fo:
                        var_name = s['source'].replace('.yaml', '').replace('.', '_')
                        fo.write(f'constexpr std::string_view {var_name} = R"delimiter(\n{json.dumps(spec, separators=(',', ':'))})delimiter";\n\n')
                        var_names.append(var_name)
                    
                    


        # reformat schema sources for namespace file
        schema = list()
        for s in ns['schema']:
            if 'source' in s:
                s = {'source': s['source'].split('.yaml')[0]}
            schema.append(s)
        ns['schema'] = schema

        # convert to cpp variables
        ns_output = {'namespaces': [ns]}
        with open(header_file, 'a') as fo:
            fo.write(f'constexpr std::string_view namespaces = R"delimiter(\n{json.dumps(ns_output, separators=(',', ':'))})delimiter";\n\n')
            fo.write(f'constexpr std::array<std::pair<std::string_view, std::string_view>, {len(var_names) + 1}>\n    specVariables {{{{\n')
            fo.write(''.join([f'  {{"{name.replace("_", ".")}", {name}}},\n' for name in var_names]))
            fo.write('  {"namespace", namespaces}\n')
            fo.write(f'}}}};\n}}  // namespace AQNWB::SPEC::{ns["name"].upper().replace("-", "_")}\n')