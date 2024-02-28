import json

from pathlib import Path
from ruamel.yaml import YAML

# TODO - setup git submodule or cloning
schema_dir = Path('./resources/spec/core/2.7.0')

# create spec strings
for file in schema_dir.rglob(r"*.yaml"):
    # load file
    yaml = YAML(typ='safe')
    with open(file) as f:
        spec = yaml.load(file)

    # convert to json
    outfile = file.with_suffix('.json')
    print(f'Generating file {outfile}')
    with open(outfile, 'w') as fo:
        json.dump(spec, fo, separators=(',', ':'),)