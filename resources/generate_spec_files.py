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
        namespace = yaml.load(file)

    # get all the sources
    for i, ns in enumerate(namespace['namespaces']):
        spec_dir = Path(f"./resources/spec/{ns['name']}/{ns['version']}")
        spec_dir.mkdir(parents=True, exist_ok=True)

        # create header file
        header_file = Path(f"./src/spec/{ns['name'].replace('-', '_')}.hpp").resolve()
        with open(header_file, 'w') as fo:
            fo.write('#pragma once\n\n')
            fo.write('#include <string>\n#include <string_view>\n#include <array>\n\n')
            fo.write(f'namespace AQNWB::spec::{ns['name'].replace('-', '_')}\n{{\n\n')
            fo.write(f'const std::string version = "{ns["version"]}";\n\n')

        # load and convert schema files
        var_names = []
        for s in ns['schema']:
            if 'source' in s:
                # load file
                schema_file = file.parent / s['source']
                with open(schema_file) as f:
                    spec = yaml.load(schema_file)

                # convert to cpp string
                print(f'Generating file {header_file} - {s["source"]}')
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
            fo.write(f'constexpr std::array<std::pair<std::string_view, std::string_view>, {len(var_names) + 1}> specVariables {{{{\n')
            fo.write(''.join([f'  {{"{name.replace('_', '.')}", {name}}},\n' for name in var_names]))
            fo.write('  {"namespace", namespaces}\n')
            fo.write(f'}}}};\n}}  // namespace AQNWB::spec::{ns['name'].replace('-', '_')}\n')