# aq-nwb

This is the aq-nwb project.

# Requirements
* A C++17-compliant compiler
* CMake `>= 3.15`
* HDF5
* Boost
* Doxygen (optional, documentation building is skipped if missing)


# Building and installing

## Build

Here are the steps for building in release mode with a multi-configuration generator:

```sh
cmake -S . -B build
cmake --build build --config Release
```

Note, if you are using custom installs of HDF5 or BOOST that are not being detected 
automatically by cmake, you can specify `HDF5_ROOT` and `BOOST_ROOT` environment variables to 
point to install directories of HDF5 and BOOST respectively. 


## Install

Here is the command for installing the release mode artifacts with a
multi-configuration generator:

```sh
cmake --install build --config Release
```

# Developing

Build system targets that are only useful for developers of this project are
hidden if the `aq-nwb_DEVELOPER_MODE` option is disabled. Enabling this
option makes tests and other developer targets and options available. You can enable
the option when configuring the build by adding ``-Daq-nwb_DEVELOPER_MODE=ON``, e.g.,:

```sh
cmake -S . -B build -Daq-nwb_DEVELOPER_MODE=ON
```

### Configure, build and test

You can configure, build and test the project respectively with the following commands from the project root on
any operating system with any build system:

```sh
cmake --preset=dev
cmake --build --preset=dev
ctest --preset=dev
```

### Developer mode targets

Additional targets can be invoked when in development mode using the commands below

```sh
cmake --build --preset=dev --target=<name of the target>
```

#### Target options
- `docs` : builds to documentation using Doxygen.
- `format-check` and `format-fix`: run the clang-format tool on the codebase to check errors and to fix them respectively.
- `run-exe`: runs the executable target `aq-nwb_exe`.
- `spell-check` and `spell-fix`: run the codespell tool on the codebase to check errors and to fix them respectively.
