# AqNWB LabMetaData Extension Demo

This demo app shows how we can implement and use an extension to NWB via AqNWB. Specifically, this
example shows how we can:
1. Create C++ header files for extension schema
2. Integrate extension schema with AqNWB (incl. automatic caching of the schema on write)
3. Implement `RegisteredType` classes to represent the new `neurodata_type`s defined in the extension
4. Use the extension for writing and reading data

## Key steps for creating this example

The value of this app is not just the code itself, but rather to understand the steps it takes to
implement this app. To implement this example app we needed to:

1. **Define the data schema:** Here we use the schema from [ndx-labmetadata-example](https://github.com/NeurodataWithoutBorders/ndx-labmetadata-example/)
    as an example. See the tutorial on [Extending NWB](https://nwb-overview.readthedocs.io/en/latest/extensions_tutorial/extensions_tutorial_home.html)
    to learn more about how to create extensions and see the [Extensions for lab-specific metadata](https://nwb-overview.readthedocs.io/en/latest/extensions_tutorial/extension_examples/labmetadata_extension.html)
    in particular for details on how to create the [ndx-labmetadata-example](https://github.com/NeurodataWithoutBorders/ndx-labmetadata-example/) extension schema.
    For convenience we downloaded just the schema YAML files to the `demo/labmetadata_extension_demo/spec` folder.  
2. **Convert the schema files to C++**:  Next we ran the `resources/generate_spec_files.py` script on the schema files to generate the necessary C++ header files.
    ```
    mkdir demo/labmetadata_extension_demo/src
    python resources/generate_spec_files.py  demo/labmetadata_extension_demo/spec  demo/labmetadata_extension_demo/src
    ``` 
    This generated a new header file `ndx_labmetadata_example.hpp` in the `demo/labmetadata_extension_demo/src` folder. 

3. **Create new RegisteredType classes:** Next we implemented a `RegisteredType` class for every `neurodata_type` defined in the schema
   following the tutorial on [Implementing a new Neurodata Type](https://neurodatawithoutborders.github.io/aqnwb/registered_type_page.html).
   Specifically, we implemented the `LabMetaDataExtensionExample.hpp/cpp` source files. A few key steps to look  out for:
   * **Use the right base class:**
       * At the very least our class must inherit from `` AQNWB::NWB::RegisteredType``
       * When the `neurodata_type` is a dataset then it is often useful to inherit from `Data` defined in `#include "nwb/hdmf/base/Data.hpp"` to inherit its base ``initialize`` and read methods.
       * When the `neurodata_type` is a group  then it is often useful to inherit from  `Container` defined in `#include "nwb/hdmf/base/Container.hpp"` to inherit its base ``initialize`` and read methods. Since our extensions is a group, this is what we are using. 
       * **Note:** Technically we also should have created a class for `LabMetaData` itself and then inherited from it. However since `LabMetaData` itself is only an empty group and does not add any fields or features, we can just use `Container` as our base class instead. The
       key purpose of the `LabMetaData` type is that it has a reserved space in the `/general` group in the NWB file.
    * **Ensure REGISTER_SUBCLASS is called in the header file**
    * **Ensure REGISTER_SUBCLASS_IMPL is called in the cpp file**
    * Implement the setup of your data type for write in a function called `initialize` (and make sure to call the `initialize` of your parent class).
    * Include appropriate `DEFINE_FIELD` and `DEFINE_REGISTERED_FIELD` definitions to simplify read 
   
4. **Use your extension in your application:** Here we create a simple `main.cpp` example script that demonstrated the use 
   of our new ` LabMetaDataExtensionExample` type to write and read the example. 

## Building and Running the Demo

### Prerequisites

Before building the demo, ensure you have the following dependencies installed:
- CMake (version 3.15 or higher)
- C++ compiler with C++17 support
- HDF5 library with C++ support
- Boost library

### Build Instructions

1. **Build the main AqNWB library**  Make sure you have built the main aqnwb project first 
   following the instructions in the AqNWB documentation

2. **Build the LabMetaData Extension Demo**:
   ```bash
   cd /path/to/aqnwb/demo/labmetadata_extension_demo
   mkdir build
   cd build
   cmake ..
   make
   ```

   If you need to specify custom paths to dependencies, you can use the following CMake variables:
   ```bash
   cmake .. -DAQNWB_DIR=/path/to/aqnwb/build/dev -DHDF5_DIR=/path/to/hdf5 -DBOOST_ROOT=/path/to/boost
   ```

3. **Run the demo**:
   ```bash
   ./bin/labmetadata_extension_demo
   ```

   This will:
   - Create a new NWB file named "test.nwb"
   - Create a LabMetaDataExtensionExample object with tissue preparation details
   - Write the object to the file
   - Read the object back from the file
   - Display the tissue preparation information

### Troubleshooting

If you encounter build errors:

1. **Check dependency paths**: Ensure that the paths to AqNWB, HDF5, and Boost are correctly set in the CMake command or in the CMakeLists.txt file.

2. **Check compiler compatibility**: Make sure your compiler supports C++17 features.

3. **Library linking issues**: If you encounter linking errors, verify that the AqNWB library has been built successfully and that the path to it is correct.
