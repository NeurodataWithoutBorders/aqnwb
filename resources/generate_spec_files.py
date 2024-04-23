import json

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

        # load and convert schema files
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
