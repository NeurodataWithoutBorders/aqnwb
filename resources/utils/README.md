# AqNWB Utilities

Command-line utilities for AqNWB development.

## Installation

### Using uv (Recommended)

The utilities use inline script metadata (PEP 723) and can be run directly with `uv`:

```bash
# Install uv if you haven't already
brew install uv

# Run utilities directly without installation
uv run resources/utils/aqnwb_utils.py generate-spec <schema_dir> <output_dir>
uv run resources/utils/aqnwb_utils.py generate-types <namespace_file> <output_dir>
```

### Traditional Installation

You can also install the package if you prefer:

```bash
# Using uv
uv pip install ./resources/utils

# Or using pip
pip install ./resources/utils

# Then use the installed command
aqnwb-utils generate-spec <schema_dir> <output_dir>
```

## Usage

### Direct execution with uv (no installation required)

```bash
# Generate spec files
uv run resources/utils/aqnwb_utils.py generate-spec <schema_dir> <output_dir>

# Generate neurodata types
uv run resources/utils/aqnwb_utils.py generate-types <namespace_file> <output_dir>

# Generate types with test app
uv run resources/utils/aqnwb_utils.py generate-types --generate-test-app <namespace_file> <output_dir>
```

### After installation

```bash
# Generate spec files
aqnwb-utils generate-spec <schema_dir> <output_dir>

# Generate neurodata types
aqnwb-utils generate-types <namespace_file> <output_dir>

# Generate types with test app
aqnwb-utils generate-types --generate-test-app <namespace_file> <output_dir>
```

## Available Commands

- `generate-spec`: Generate C++ header files from NWB schema files
- `generate-types`: Generate C++ classes from neurodata types defined in schema files

## Benefits of using uv

- **No installation required**: Run scripts directly with their dependencies automatically managed
- **Isolated environments**: Each script run uses its own isolated environment
- **Fast**: uv is significantly faster than pip for dependency resolution and installation
- **Reproducible**: Dependencies are specified in the script itself using PEP 723 metadata
