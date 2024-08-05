# AqNWB

AqNWB is a C++ API for acquiring neurophysiological data directly into the NWB (Neurodata Without Borders) format.
Our goal is to provide a lightweight API to integrate with existing acquisition systems.

Please note, AqNWB is currently under development and should not yet be used in practice.

Below is a high-level overview of the project structure and capabilities we are targeting: 

![Project Overview](resources/images/aqnwb_objective_500px.png)


# Requirements
* A C++17-compliant compiler
* CMake `>= 3.15`
* HDF5
* Boost
* Additional requirements for building the documentation (optional)
    * Doxygen
    * Graphviz
* Additional requirements for developers (mode `dev`)
    * cppcheck
    * clang-format

# Building and installing

## Build

Here are the steps for building in release mode with a multi-configuration generator:

```sh
cmake -S . -B build
cmake --build build --config Release
```

Note, if you are using custom installations of HDF5 or BOOST that are not being detected 
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
- `format-check` and `format-fix`: run the clang-format tool on the codebase to check errors and to fix them respectively.
- `spell-check` and `spell-fix`: run the codespell tool on the codebase to check errors and to fix them respectively.
- `docs` : builds to documentation using Doxygen. (Note: run `cmake --preset=dev -DBUILD_DOCS=ON` before building to add docs target)

# Code of Conduct

This project and everyone participating in it is govered by our [code of conduct guidelines](./.github/CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

# LICENSE

AqNWB Copyright (c) 2024, The Regents of the University of California,
through Lawrence Berkeley National Laboratory (subject to receipt of any
required approvals from the U.S. Dept. of Energy). All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

(1) Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

(2) Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

(3) Neither the name of the University of California, Lawrence Berkeley
National Laboratory, U.S. Dept. of Energy nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You are under no obligation whatsoever to provide any bug fixes, patches,
or upgrades to the features, functionality or performance of the source
code ("Enhancements") to anyone; however, if you choose to make your
Enhancements available either publicly, or directly to Lawrence Berkeley
National Laboratory, without imposing a separate written license agreement
for such Enhancements, then you hereby grant the following license: a
non-exclusive, royalty-free perpetual license to install, use, modify,
prepare derivative works, incorporate into other computer software,
distribute, and sublicense such enhancements or derivative works thereof,
in binary and source code form.

# COPYRIGHT

AqNWB Copyright (c) 2024, The Regents of the University of California, 
through Lawrence Berkeley National Laboratory (subject to receipt of any
required approvals from the U.S. Dept. of Energy). All rights reserved.

If you have questions about your rights to use or distribute this software,
please contact Berkeley Lab's Intellectual Property Office at
IPO@lbl.gov.

NOTICE.  This Software was developed under funding from the U.S. Department
of Energy and the U.S. Government consequently retains certain rights.  As
such, the U.S. Government has been granted for itself and others acting on
its behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
Software to reproduce, distribute copies to the public, prepare derivative 
works, and perform publicly and display publicly, and to permit others to do so.
