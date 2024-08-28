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
            fo.write('#include <string>\n\n')
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
                
                # convert to json
                spec_file = (spec_dir / s['source']).with_suffix('.json')
                print(f'Generating file {spec_file}')
                with open(spec_file, 'w') as fo:
                    json.dump(spec, fo, separators=(',', ':'),)    

                # convert to cpp string
                with open(header_file, 'a') as fo:
                    var_name = s['source'].replace('.yaml', '').replace('.', '_')
                    fo.write(f'const std::string {var_name} = R"delimiter(\n{json.dumps(spec, separators=(',', ':'))})delimiter";\n\n')
                    var_names.append(var_name)

        # reformat schema sources for namespace file
        schema = list()
        for s in ns['schema']:
            if 'source' in s:
                s = {'source': s['source'].split('.yaml')[0]}
            schema.append(s)
        ns['schema'] = schema

        # convert namespace json
        ns_file = (spec_dir / file.name).with_suffix('.json')
        ns_output = {'namespaces': [ns]}
        print(f'Generating file {ns_file}')
        with open(ns_file, 'w') as fo:
            json.dump(ns_output, fo, separators=(',', ':'),)
        
        # convert to cpp variables
        with open(header_file, 'a') as fo:
            fo.write(f'const std::string namespaces = R"delimiter(\n{json.dumps(ns_output, separators=(',', ':'))})delimiter";\n\n')
            fo.write('void registerVariables(std::map<std::string, const std::string*>& registry) {\n')
            fo.write(''.join([f'    registry["{name.replace('_', '.')}"] = &{name};\n' for name in var_names]))
            fo.write(f'    registry["namespace"] = &namespaces;\n')
            fo.write(f'}};\n}}  // namespace AQNWB::spec::{ns['name'].replace('-', '_')}\n')