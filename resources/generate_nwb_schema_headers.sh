#!/usr/bin/env bash
###############################################################################
# generate_nwb_schema_headers.sh
#
# Generate C++ header files from the latest *release* of the NWB and HDMF schema definitions.
#
# This script clones the NWB and HDMF schema repositories, checks out the latest
# tagged release in each repo, runs the generate_spec_files.py script to generate
# C++ header files, and copies them to src/spec.
#
# Usage:
#   bash resources/generate_nwb_schema_headers.sh
#
# Optional environment variables:
#   PYTHON     - Python interpreter to use (default: python)
#   NWB_REPO   - URL of the NWB schema git repository
#   HDMF_REPO  - URL of the HDMF common schema git repository
#
# Prerequisites:
#   - git
#   - Python 3 (with required dependencies for generate_spec_files.py)
#
# The script will:
#   1. Create a temporary working directory.
#   2. Clone the NWB and HDMF schema repositories.
#   3. Check out the latest tagged release in each repository.
#   4. Run generate_spec_files.py for both schemas.
#   5. Copy the generated .hpp files to src/spec.
#   6. Clean up the temporary directory automatically.
#
# Example:
#   PYTHON=python3 bash resources/generate_nwb_schema_headers.sh
#
# For more details, see the developer documentation.
###############################################################################

set -euo pipefail

# Logging function
log() { echo "[INFO] $*"; }

# Check dependencies
for cmd in git "${PYTHON:-python3}"; do
    command -v "$cmd" >/dev/null 2>&1 || { echo "[ERROR] $cmd not found"; exit 1; }
done

# Configurable variables
PYTHON=${PYTHON:-python}
NWB_REPO=${NWB_REPO:-https://github.com/NeurodataWithoutBorders/nwb-schema.git}
HDMF_REPO=${HDMF_REPO:-https://github.com/hdmf-dev/hdmf-common-schema.git}

# Create temp dir and ensure cleanup
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT
log "Created temp dir: $TMPDIR"

# Clone repositories
log "Cloning NWB schema..."
git clone "$NWB_REPO" "$TMPDIR/nwb-schema"
(
    cd "$TMPDIR/nwb-schema"
    git fetch --tags
    LATEST_TAG=$(git tag --sort=-v:refname | head -n1)
    if [ -n "$LATEST_TAG" ]; then
        log "Checking out NWB schema latest release: $LATEST_TAG"
        git checkout "$LATEST_TAG"
    else
        log "No tags found in NWB schema repo, using default branch."
    fi
)

log "Cloning HDMF common schema..."
git clone "$HDMF_REPO" "$TMPDIR/hdmf-common-schema"
(
    cd "$TMPDIR/hdmf-common-schema"
    git fetch --tags
    LATEST_TAG=$(git tag --sort=-v:refname | head -n1)
    if [ -n "$LATEST_TAG" ]; then
        log "Checking out HDMF common schema latest release: $LATEST_TAG"
        git checkout "$LATEST_TAG"
    else
        log "No tags found in HDMF common schema repo, using default branch."
    fi
)

# Output directory
OUTDIR="$TMPDIR/generated_headers"
mkdir -p "$OUTDIR"

# Script location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Generate headers
log "Generating NWB core schema headers..."
"$PYTHON" "$SCRIPT_DIR/generate_spec_files.py" "$TMPDIR/nwb-schema/core" "$OUTDIR"

log "Generating HDMF common schema headers..."
"$PYTHON" "$SCRIPT_DIR/generate_spec_files.py" "$TMPDIR/hdmf-common-schema/common" "$OUTDIR"

# Copy to destination
DEST_DIR="$SCRIPT_DIR/../src/spec"
mkdir -p "$DEST_DIR"
cp "$OUTDIR"/*.hpp "$DEST_DIR"

log "Header files generated and copied to $DEST_DIR."
