# AqNWB Copilot Instructions

## Repository Overview

**AqNWB** is a C++ API for acquiring neurophysiological data directly into the NWB (Neurodata Without Borders) format. This is a professional, production-quality codebase under active development by NeurodataWithoutBorders.

### Key Information
- **Language**: C++ (C++17 minimum, supports C++17/20/23)
- **Build System**: CMake (≥3.15)
- **Main Dependencies**: HDF5 (with C++ bindings), formerly Boost (removed in recent versions)
- **Test Framework**: Catch2 (v3.5.3)
- **Documentation**: Doxygen
- **Purpose**: Lightweight API to integrate neurophysiology data acquisition with existing systems

## Project Structure

```
aqnwb/
├── src/                          # Source code
│   ├── Channel.cpp/hpp           # Channel definitions
│   ├── Types.hpp                 # Type definitions
│   ├── Utils.hpp                 # Utility functions (UUID, time, endian)
│   ├── io/                       # I/O implementations
│   │   ├── BaseIO.hpp/cpp        # Abstract base I/O
│   │   ├── ReadIO.hpp            # Read operations
│   │   ├── RecordingObjects.hpp  # Recording container management
│   │   ├── nwbio_utils.hpp       # I/O utilities
│   │   └── hdf5/                 # HDF5-specific implementations
│   │       ├── HDF5IO.hpp/cpp
│   │       ├── HDF5RecordingData.hpp/cpp
│   │       └── HDF5ArrayDataSetConfig.hpp/cpp
│   ├── nwb/                      # NWB schema implementations
│   │   ├── NWBFile.hpp/cpp       # Main NWB file interface
│   │   ├── RegisteredType.hpp/cpp # Base type with registration system
│   │   ├── base/                 # NWB base types (NWBData, NWBDataInterface, NWBContainer, ProcessingModule, TimeSeries)
│   │   ├── device/               # Device types
│   │   ├── ecephys/              # Electrophysiology types (ElectricalSeries, SpikeEventSeries)
│   │   ├── file/                 # File-level types (ElectrodeGroup, ElectrodesTable)
│   │   ├── hdmf/                 # HDMF types (Container, Data, DynamicTable, etc.)
│   │   └── misc/                 # Miscellaneous types (AnnotationSeries)
│   └── spec/                     # Namespace registry for schema handling
├── tests/                        # Unit tests and examples
│   ├── test*.cpp                 # Test files (one per module)
│   └── examples/                 # Example code (also tested)
├── demo/                         # Demonstration projects
│   ├── inspect_electrical_series/ # Demo reading/analyzing NWB files
│   └── labmetadata_extension_demo/ # Extension integration demo
├── docs/                         # Doxygen documentation
│   ├── pages/userdocs/          # User documentation
│   └── pages/devdocs/           # Developer documentation
├── resources/                    # Resource files
│   ├── utils/                   # Python utilities for schema/code generation
│   │   ├── aqnwb_utils.py       # CLI interface
│   │   ├── generate_spec_files.py
│   │   └── schematype_to_aqnwb.py
│   ├── schema/                  # (gitignored) Generated schema files
│   └── images/                  # Documentation images
├── cmake/                        # CMake modules
├── .github/workflows/           # CI/CD workflows
└── CMakeLists.txt               # Main build configuration
```

## Architecture & Key Concepts

### RegisteredType System
- **Core Pattern**: All NWB types inherit from `RegisteredType` and use a factory pattern
- **Memory Management**: Objects use `std::shared_ptr` and are automatically registered with `RecordingObjects`
- **Creation**: Always use `Type::create(io)` factory methods, never direct constructors
- **Finalization**: Call `finalize()` before closing files to ensure data is written

### I/O Architecture
- **BaseIO**: Abstract base class for I/O operations
- **HDF5IO**: Concrete implementation using HDF5
- **RecordingObjects**: Container tracking all RegisteredType objects for a recording session
- **I/O Object Ownership**:
  - **User Ownership**: The user creates and owns the I/O object (e.g., `HDF5IO`) as a `std::shared_ptr`
  - **Weak Pointers in RegisteredTypes**: Each `RegisteredType` stores a `std::weak_ptr` to the I/O object
  - **No Circular References**: Using `weak_ptr` prevents RegisteredType objects from keeping the I/O alive, avoiding circular dependencies and memory leaks
  - **Safe Access**: RegisteredType objects use `getIO()` to obtain a `shared_ptr` from the `weak_ptr` before accessing the I/O
  - **IO Owns RecordingObjects**: The I/O object owns a `shared_ptr` to `RecordingObjects`, which tracks all RegisteredType objects
- **Recording Workflow**:
  1. Create IO object (user retains ownership)
  2. Create NWBFile and related types using factory methods (automatically registered with IO's RecordingObjects)
  3. Write data during acquisition
  4. Call `stopRecording()` which finalizes all objects and clears caches

## Build System

### CMake Presets
AqNWB uses CMake presets extensively. Key presets:
- `ci-macos`, `ci-ubuntu`, `ci-windows`: CI builds for each platform
- `ci-coverage`: Build with coverage instrumentation
- `ci-sanitize`: Build with address/undefined behavior sanitizers
- `dev`: Developer mode (requires `CMakeUserPresets.json` - see docs)

### Configuration Options
- `aqnwb_DEVELOPER_MODE`: Enable developer targets (tests, linting, etc.) - Default: OFF
- `AQNWB_CXX_STANDARD`: C++ standard version (17, 20, or 23) - Default: 17
- `BUILD_SHARED_LIBS`: Build shared libraries - Default: OFF
- `BUILD_DOCS`: Build Doxygen documentation - Default: OFF
- `ENABLE_COVERAGE`: Enable coverage reporting - Default: OFF

### Standard Build Workflow

**User Build** (library only):
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix=/path/to/install
```

**Developer Build** (with tests):
```bash
# Option 1: Using presets (recommended)
cmake --preset=ci-<os>  # os = ubuntu, macos, or windows
cmake --build build --config Release
ctest --test-dir build --output-on-failure

# Option 2: Manual configuration
cmake -S . -B build -Daqnwb_DEVELOPER_MODE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest
```

**Custom HDF5 Location**:
```bash
cmake --preset=ci-ubuntu -DCMAKE_PREFIX_PATH=/path/to/hdf5
```

**Build with C++23**:
```bash
cmake --preset=ci-macos -DAQNWB_CXX_STANDARD=23
```

### Important Build Notes
- **HDF5 is PUBLIC**: HDF5 headers and libraries are exported to consumers
- **Boost Removed**: Boost dependency was removed in version 0.2.0+ (replaced with custom utilities in `src/Utils.hpp`)
- **Cross-Platform**: Builds on Linux, macOS, and Windows (MSVC/vcpkg on Windows)
- **Parallel Builds**: Use `-j N` with cmake --build for faster builds

## Testing

### Running Tests

**Quick test run**:
```bash
cd build
ctest --output-on-failure
```

**With presets**:
```bash
ctest --preset=dev  # Requires CMakeUserPresets.json
```

**Parallel testing**:
```bash
ctest -j 4 --output-on-failure
```

**Run specific test**:
```bash
ctest -R testNWBFile --verbose
```

### Test Structure
- **Unit Tests**: Located in `tests/test*.cpp` files
- **Examples**: In `tests/examples/` - serve both as tests and documentation examples
- **Test Naming**: `testModuleName.cpp` tests the corresponding module
- **Framework**: Catch2 v3.5.3 with CTest integration
- **Test Data**: Generated `.nwb` files in `build/tests/data/` (gitignored)

### Test Best Practices
- Tests are comprehensive and cover core functionality
- Example tests use Doxygen markers (`//! [section_name]`) for doc inclusion
- Tests validate against NWB schema using nwbinspector (Python)
- DO NOT remove or skip tests without strong justification

## Code Quality & Linting

### clang-format
**Check formatting**:
```bash
cmake --build build --target format-check
```

**Auto-fix formatting**:
```bash
cmake --build build --target format-fix
```

**Direct usage**:
```bash
cmake -D FORMAT_COMMAND=clang-format -P cmake/lint.cmake
```

Configuration: `.clang-format` (Chromium-based style with customizations)

### clang-tidy
Configuration: `.clang-tidy`
- Enables most checks except overly aggressive ones
- Configured for C++17 compatibility
- Used in CI but not required for local development

### codespell
**Check spelling**:
```bash
cmake --build build --target spell-check
```

**Auto-fix spelling**:
```bash
cmake --build build --target spell-fix
```

Configuration: `.codespellrc`
- Custom dictionary in `.codespell_ignore`
- Skips generated files and dependencies

### Code Style Guidelines
- Use modern C++ practices (C++17+)
- Prefer smart pointers over raw pointers
- Follow existing naming conventions (see `.clang-tidy` for identifier naming)
- Add Doxygen comments for public APIs
- Minimal comments in implementation unless explaining complex logic

## Dependencies

### Required Runtime Dependencies
- **HDF5**: Required with C++ bindings (`libhdf5-dev` on Ubuntu)
  - Must support the C++ interface
  - Headers and libraries are transitively exported

### Build-Time Dependencies
- **CMake**: ≥3.15
- **C++ Compiler**: C++17 support minimum
  - GCC 7+, Clang 5+, MSVC 2017+, AppleClang (Xcode 10+)

### Development Dependencies
- **Catch2**: v3.5.3 for testing
  - Manual build/install on Linux required (see CI workflows)
  - Available via brew on macOS, vcpkg on Windows
- **Doxygen**: For documentation generation (optional)
- **Graphviz**: For documentation diagrams (optional)
- **clang-format**: For code formatting (optional but recommended)
- **codespell**: For spell checking (optional but recommended)
- **cppcheck**: Static analysis (optional)

### Python Utilities
The `resources/utils/` directory contains Python utilities for code generation:
- **Use `uv` (recommended)**: Scripts have inline dependencies (PEP 723)
  ```bash
  uv run resources/utils/aqnwb_utils.py generate-spec <args>
  ```
- **Traditional install**: `pip install ./resources/utils`

## CI/CD Workflows

### GitHub Actions Workflows
Located in `.github/workflows/`:

1. **tests.yml**: Main test workflow
   - Runs on Ubuntu, macOS, Windows
   - Includes sanitizer checks (ASAN/UBSAN)
   - Validates NWB files with nwbinspector
   - Tests C++23 compatibility

2. **lint.yml**: Code formatting checks
   - Runs clang-format validation
   - Fails on formatting violations

3. **codespell.yml**: Spelling checks
   - Validates spelling across codebase
   - Uses codespell GitHub action

4. **coverage.yml**: Code coverage
   - Generates coverage reports with lcov
   - Uploads to Codecov

5. **doxygen-gh-pages.yml**: Documentation deployment
   - Builds and deploys docs to GitHub Pages

6. **build-demo.yml**: Demo project validation
   - Ensures demo projects build correctly

7. **generate-classes.yml**, **upgrade_schema.yml**, **python-utils.yml**: 
   - Automation for schema and code generation

### CI Notes
- **Merge Commits**: CI tests temporary merge commits for PRs
- **Platform-Specific**: Different dependency installation per platform
- **Vcpkg on Windows**: Uses vcpkg for dependency management
- **Artifacts**: Test-generated `.nwb` files are uploaded for validation

## Common Development Workflows

### Making Code Changes
1. **Setup**: Build in developer mode
   ```bash
   cmake --preset=ci-<os>
   cmake --build build
   ```

2. **Make changes**: Edit source files in `src/`

3. **Format code**: 
   ```bash
   cmake --build build --target format-fix
   ```

4. **Run tests**:
   ```bash
   cd build && ctest --output-on-failure
   ```

5. **Check spelling**:
   ```bash
   cmake --build build --target spell-check
   ```

### Adding New NWB Types
1. **Generate from schema** (preferred):
   ```bash
   uv run resources/utils/aqnwb_utils.py generate-types <namespace_file> <output_dir>
   ```
   
   **Note**: Generated code provides a starting point but will likely require additional updates:
   - Review and modify the generated classes to implement full functionality
   - Copy the generated files to the appropriate location in the AqNWB source tree (e.g., `src/nwb/`)
   - Add the new source files to `CMakeLists.txt`
   - Create corresponding test files

2. **Manual creation**:
   - Inherit from appropriate base class (RegisteredType, TimeSeries, etc.)
   - Use `REGISTER_SUBCLASS` macro
   - Implement required virtual methods
   - Add to CMakeLists.txt
   - Create corresponding test file
   
   **For more details**, see the [RegisteredType documentation](https://nwb.org/aqnwb/registered_type_page.html) or the [registered_types.dox](https://github.com/NeurodataWithoutBorders/aqnwb/blob/main/docs/pages/devdocs/registered_types.dox) file in the repository.

3. **Add tests**: Create `tests/testYourType.cpp`
   
   **For comprehensive details on RegisteredType**, see the [RegisteredType documentation](https://nwb.org/aqnwb/registered_type_page.html) or the [registered_types.dox](https://github.com/NeurodataWithoutBorders/aqnwb/blob/main/docs/pages/devdocs/registered_types.dox) file.

### Adding Schema Extensions

To integrate a new schema namespace with AqNWB (e.g., to support an extension to NWB):

1. **Get the schema files**: Download or create the schema for the namespace in YAML format
   - For creating new extensions, see the [NWB Extension Tutorial](https://nwb-overview.readthedocs.io/en/latest/extensions_tutorial/extensions_tutorial_home.html)

2. **Convert schema to C++**: Run the `generate-spec` command on your schema files
   ```bash
   uv run resources/utils/aqnwb_utils.py generate-spec <schema_dir> <output_dir>
   ```
   This generates C++ header files with namespace definitions and registration

3. **Include generated headers**: In your C++ code, include the generated header files
   - Headers automatically register the namespace with `NamespaceRegistry` using the `REGISTER_NAMESPACE` macro
   - `NWBFile::initialize` will automatically cache your schema in the NWB file

4. **Implement RegisteredType classes**: Define appropriate interfaces for the `neurodata_type`s in your namespace
   - Follow the [RegisteredType tutorial](https://nwb.org/aqnwb/registered_type_page.html)
   - Use `generate-types` command for skeleton class generation from schema
   - Key points:
     - Inherit from appropriate base class (RegisteredType, Data for datasets, Container for groups)
     - Use `REGISTER_SUBCLASS` macro in header and `REGISTER_SUBCLASS_IMPL` in cpp
     - Use field definition macros: `DEFINE_DATASET_FIELD`, `DEFINE_ATTRIBUTE_FIELD`, `DEFINE_REGISTERED_FIELD`

5. **Example demo**: See `demo/labmetadata_extension_demo/` for a complete working example

**For comprehensive details**, see the [Integrating Extensions documentation](https://nwb.org/aqnwb/integrating_extensions_page.html) or the [integrating_extensions.dox](https://github.com/NeurodataWithoutBorders/aqnwb/blob/main/docs/pages/devdocs/integrating_extensions.dox) file.

### Adding Tests
1. Create new test file: `tests/testFeature.cpp`
2. Use Catch2 syntax: `TEST_CASE`, `SECTION`, `REQUIRE`
3. Add to `tests/CMakeLists.txt` if needed (usually auto-discovered)
4. Run: `ctest -R testFeature`

### Updating Documentation
1. **Code documentation**: Add Doxygen comments to headers
2. **User/Dev docs**: Edit files in `docs/pages/userdocs/` or `docs/pages/devdocs/`
3. **Build docs locally**:
   ```bash
   cmake -S . -B build -DBUILD_DOCS=ON
   cmake --build build --target docs
   # Output in build/docs/html/
   ```

## Troubleshooting Common Issues

### Issue: HDF5 Not Found
**Symptoms**: CMake fails with "Could not find HDF5"

**Solutions**:
1. Install HDF5 with C++ bindings:
   - Ubuntu: `sudo apt-get install libhdf5-dev`
   - macOS: `brew install hdf5`
   - Windows: Use vcpkg

2. If installed in non-standard location:
   ```bash
   cmake --preset=ci-<os> -DCMAKE_PREFIX_PATH=/path/to/hdf5
   ```

3. **DO NOT** use `HDF5_ROOT` environment variable (deprecated)

### Issue: Catch2 Not Found (Linux)
**Symptoms**: CMake fails with "Could not find Catch2"

**Solution**: Manually install Catch2:
```bash
git clone https://github.com/catchorg/Catch2.git
cd Catch2
git checkout v3.5.3
cmake -Bbuild -H. -DBUILD_TESTING=OFF
sudo cmake --build build/ --target install
```

### Issue: Format Check Fails in CI
**Symptoms**: lint.yml workflow fails

**Solution**: Run locally and fix:
```bash
cmake --build build --target format-fix
git add -u
git commit -m "Fix formatting"
```

### Issue: Test Data Not Found
**Symptoms**: Tests fail with file not found errors

**Cause**: Test data is generated at runtime in `build/tests/data/`

**Solution**:
- Ensure tests are run from correct directory
- Check `.gitignore` - test data is intentionally not committed
- Re-run tests to regenerate data

### Issue: Build Fails on Windows
**Symptoms**: MSVC compilation errors or missing dependencies

**Solutions**:
1. Use vcpkg for dependencies:
   ```bash
   vcpkg install hdf5[cpp]:x64-windows catch2:x64-windows
   ```

2. Use Visual Studio 2022 generator (required by presets)

3. Ensure vcpkg toolchain file is set:
   ```bash
   cmake --preset=ci-windows  # Preset includes toolchain file path
   ```

**Note**: Older versions (pre-0.2.0) required Boost. If working with older code, also install:
   ```bash
   vcpkg install boost-date-time:x64-windows boost-endian:x64-windows boost-uuid:x64-windows boost-multi-array:x64-windows
   ```

### Issue: Boost Not Found (Older Versions)
**Note**: Boost dependency was removed in recent versions. If working with older code:

**Solution**: Update to latest version or install boost:
- Ubuntu: `sudo apt-get install libboost-all-dev`
- macOS: `brew install boost`

### Issue: Tests Pass Locally But Fail in CI
**Causes**:
1. **Merge commit**: CI tests PR merged with target branch
   - **Solution**: Merge/rebase with main branch

2. **Platform differences**: Different OS/compiler
   - **Solution**: Check platform-specific CI logs, test on that platform

3. **Missing files**: Files not committed
   - **Solution**: Check `git status`, commit all changes

### Issue: Sanitizer Failures
**Symptoms**: `ci-sanitize` job fails with ASAN/UBSAN errors

**Approach**:
1. Build locally with sanitizers:
   ```bash
   cmake --preset=ci-sanitize
   cmake --build build/sanitize
   cd build/sanitize && ctest
   ```

2. Common issues:
   - Memory leaks: Check smart pointer usage
   - Use-after-free: Check object lifetimes
   - Undefined behavior: Check type conversions, array bounds

## Working with NWB Schema

### Schema Files
- **Location**: Retrieved when needed from git in resources/utils/generate_nwb_schema_headers.sh
- **In Source Location:**  Schemas are translated statically to C++ and are available in src/spec/core.hpp, hdmf_common.hpp, and hdmf_experimental.hpp
- **Namespace**: Defined in namespace YAML files

### Generating Code from Schema
```bash
# Generate spec files (C++ headers from schema)
uv run resources/utils/aqnwb_utils.py generate-spec <schema_dir> <output_dir>

# Generate neurodata type classes
uv run resources/utils/aqnwb_utils.py generate-types <namespace_file> <output_dir>
```

### Extending with Custom Types
See developer documentation: `docs/pages/devdocs/integrating_extensions.dox`

## Key Files and Their Purpose

- **CMakeLists.txt**: Main build configuration, defines library target
- **CMakePresets.json**: Build presets for different platforms/configurations
- **.clang-format**: Code formatting rules
- **.clang-tidy**: Static analysis configuration
- **.codespellrc**: Spelling checker configuration
- **.gitignore**: Excludes build artifacts, test data, schema files, IDE configs
- **CHANGELOG.md**: Version history and changes
- **src/nwb/RegisteredType.hpp**: Base class for all NWB types
- **src/io/BaseIO.hpp**: Abstract I/O interface
- **src/nwb/NWBFile.hpp**: Main NWB file interface

## Quick Reference Commands

```bash
# Configure for development
cmake --preset=ci-ubuntu  # or ci-macos, ci-windows

# Build
cmake --build build -j 4

# Test
cd build && ctest -j 4 --output-on-failure

# Format code
cmake --build build --target format-fix

# Check spelling
cmake --build build --target spell-fix

# Build documentation
cmake -S . -B build -DBUILD_DOCS=ON
cmake --build build --target docs

# Install
cmake --install build --prefix=/path/to/install

# Clean build
rm -rf build
```

## Important Notes for AI Agents

1. **Always use factory methods**: Never construct RegisteredType subclasses directly
2. **Memory management matters**: All NWB types are `shared_ptr`, tracked by RecordingObjects
3. **Finalization is required**: Call `finalize()` on objects before closing files
4. **Test thoroughly**: Run full test suite before completing changes
5. **Format before commit**: Always run `format-fix` target
6. **Spell check**: Run `spell-fix` target to avoid typos in code and documentation
7. **cppcheck**: Run `cppcheck` target for static code analysis to catch potential issues
8. **Respect .gitignore**: Don't commit build artifacts, test data, or IDE configs
9. **CI is authoritative**: Local passes don't guarantee CI passes (especially formatting)
10. **Documentation is code**: Example code in tests must work and compile
11. **Schema-driven**: Many classes are generated from NWB schema
12. **Cross-platform**: Changes must work on Linux, macOS, and Windows
13. **Update CHANGELOG.md**: Document your changes in CHANGELOG.md under the "Unreleased" section following the existing format

## Additional Resources

- **Documentation**: https://nwb.org/aqnwb/
- **Repository**: https://github.com/NeurodataWithoutBorders/aqnwb
- **NWB Format**: https://nwb.org/
- **Issues**: GitHub Issues for bug reports and feature requests
- **Contributing**: See CODE_OF_CONDUCT.md and developer docs
