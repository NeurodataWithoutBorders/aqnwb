# Refined SWIG Bindings Proposal for AQNWB

This is a refined proposal for creating SWIG bindings to the `aqnwb` C++ API, addressing the requirements for automatic generation without manual modifications and preserving the convenience of macro-generated accessor methods.

**📁 Working Example**: A complete working implementation of this proposal is available in `swig_proposal/example/`. The example demonstrates all key concepts and validates the feasibility of the approach.

---

## 🎯 Goals

- **Automatic Generation**: Bindings should be generated automatically as new classes are added to the core library without requiring manual modifications
- **Macro Preservation**: Preserve the convenience of generating accessor methods via macros (`DEFINE_ATTRIBUTE_FIELD`, `DEFINE_DATASET_FIELD`, `DEFINE_REGISTERED_FIELD`, etc.)
- **Schema-Driven**: Leverage the existing schema-driven code generation system to automatically create SWIG bindings
- **Multi-Language Support**: Support C#, Python, and Java with consistent tooling
- **Maintainability**: Ensure bindings stay in sync with the core library evolution

---

## 🏗️ Architecture Overview

The refined approach leverages the existing schema-driven code generation infrastructure in aqnwb to automatically generate SWIG bindings. This ensures that as new NWB types are added to the schema, the corresponding SWIG bindings are automatically generated.

### Key Components

1. **Schema Integration**: Extend the existing `generate_spec_files.py` to also generate SWIG interface files
2. **Macro Expansion**: Use compiler preprocessing to expand macros for SWIG consumption
3. **Automatic Interface Generation**: Generate `.i` files automatically from schema definitions
4. **CMake Integration**: Seamless integration with the existing CMake build system

---

## 🔧 Implementation Strategy

### 1. **Extend Schema Code Generation**

Modify `resources/generate_spec_files.py` to generate SWIG interface files alongside the existing C++ header generation:

```python
def generate_swig_interface(ns: Dict, class_info: Dict, output_dir: Path) -> None:
    """
    Generate SWIG interface file for a given class from schema information.
    """
    interface_file = output_dir / f"{class_info['name']}.i"
    
    with open(interface_file, 'w') as fo:
        fo.write(f'%module aqnwb_{class_info["name"].lower()}\n\n')
        fo.write('%{\n')
        fo.write(f'#include "nwb/{class_info["include_path"]}.hpp"\n')
        fo.write('%}\n\n')
        
        # Include necessary SWIG libraries
        fo.write('%include <std_string.i>\n')
        fo.write('%include <std_vector.i>\n')
        fo.write('%include <std_shared_ptr.i>\n\n')
        
        # Generate shared_ptr declarations for all referenced types
        for ref_type in class_info.get('referenced_types', []):
            fo.write(f'%shared_ptr({ref_type})\n')
        
        fo.write(f'\n%include "nwb/{class_info["include_path"]}.hpp"\n')
```

### 2. **Automatic Class Discovery**

Extend the schema processing to automatically discover all NWB classes and their relationships:

```python
def extract_class_hierarchy(schema_data: Dict) -> Dict[str, Dict]:
    """
    Extract class hierarchy and relationships from schema data.
    Returns information needed for SWIG binding generation.
    """
    classes = {}
    
    for group in schema_data.get('groups', []):
        if 'neurodata_type_def' in group:
            class_name = group['neurodata_type_def']
            classes[class_name] = {
                'name': class_name,
                'parent': group.get('neurodata_type_inc'),
                'namespace': schema_data.get('namespace', 'core'),
                'include_path': infer_include_path(class_name),
                'fields': extract_fields(group),
                'referenced_types': extract_referenced_types(group)
            }
    
    return classes
```

### 3. **Macro-Aware Interface Generation**

Create a preprocessing step that expands macros before SWIG processing:

```cmake
# CMake function to preprocess headers for SWIG
function(preprocess_for_swig INPUT_HEADER OUTPUT_HEADER)
    add_custom_command(
        OUTPUT ${OUTPUT_HEADER}
        COMMAND ${CMAKE_CXX_COMPILER} -E -I${PROJECT_SOURCE_DIR}/src 
                -DSWIG_PREPROCESSING ${INPUT_HEADER} -o ${OUTPUT_HEADER}
        DEPENDS ${INPUT_HEADER}
        COMMENT "Preprocessing ${INPUT_HEADER} for SWIG"
    )
endfunction()
```

### 4. **Comprehensive Template Instantiation for BaseRecordingData Types**

Based on our working implementation, the system generates comprehensive SWIG interfaces that include **all BaseRecordingData types** for optimal performance and type safety. Here's the **actual generated code** from our working example:

```swig
// Auto-generated SWIG interface for TimeSeries
// Generated from template: timeseries.i.template
// Provides default methods + type-specific alternatives for all BaseRecordingData types

%module aqnwb_timeseries

%{
#include "nwb/base/TimeSeries.hpp"
%}

// Include standard SWIG libraries
%include <std_string.i>
%include <std_vector.i>
%include <std_shared_ptr.i>
%include <std_any.i>

// Shared pointer declarations for referenced types
%shared_ptr(AQNWB::IO::BaseIO)
%shared_ptr(AQNWB::IO::BaseRecordingData)
%shared_ptr(AQNWB::IO::ReadDataWrapper)

// Template instantiations for macro-generated methods
// Provides both default convenience and type-specific performance

// readDescription - default type: std::string
%template(readDescription) AQNWB::NWB::TimeSeries::readDescription<std::string>;
// readComments - default type: std::string
%template(readComments) AQNWB::NWB::TimeSeries::readComments<std::string>;
// readDataConversion - default type: float
%template(readDataConversion) AQNWB::NWB::TimeSeries::readDataConversion<float>;
// readDataUnit - default type: std::string
%template(readDataUnit) AQNWB::NWB::TimeSeries::readDataUnit<std::string>;

// readData - default type: std::any, with type-specific alternatives
%template(readData) AQNWB::NWB::TimeSeries::readData<std::any>;
%template(readDataUint8_T) AQNWB::NWB::TimeSeries::readData<uint8_t>;
%template(readDataUint16_T) AQNWB::NWB::TimeSeries::readData<uint16_t>;
%template(readDataUint32_T) AQNWB::NWB::TimeSeries::readData<uint32_t>;
%template(readDataUint64_T) AQNWB::NWB::TimeSeries::readData<uint64_t>;
%template(readDataInt8_T) AQNWB::NWB::TimeSeries::readData<int8_t>;
%template(readDataInt16_T) AQNWB::NWB::TimeSeries::readData<int16_t>;
%template(readDataInt32_T) AQNWB::NWB::TimeSeries::readData<int32_t>;
%template(readDataInt64_T) AQNWB::NWB::TimeSeries::readData<int64_t>;
%template(readDataFloat) AQNWB::NWB::TimeSeries::readData<float>;
%template(readDataDouble) AQNWB::NWB::TimeSeries::readData<double>;
%template(readDataStd_String) AQNWB::NWB::TimeSeries::readData<std::string>;

// readTimestamps - default type: double, with type-specific alternatives
%template(readTimestamps) AQNWB::NWB::TimeSeries::readTimestamps<double>;

// readControl - default type: uint8_t, with type-specific alternatives
%template(readControl) AQNWB::NWB::TimeSeries::readControl<uint8_t>;

// Include the actual header file
%include "nwb/base/TimeSeries.hpp"
```

**Critical Discovery**: Our implementation revealed that **all 11 BaseRecordingData types** must be instantiated for data fields to provide optimal performance and avoid std::any conversion overhead:

- `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t` (unsigned integers)
- `int8_t`, `int16_t`, `int32_t`, `int64_t` (signed integers)  
- `float`, `double` (floating point)
- `std::string` (string data)

**Process Flow** (Validated in Working Example):
1. **Template-based generation** → Python script uses templates to generate `.i` files
2. **Automatic type discovery** → Script extracts all BaseRecordingData types from BaseIO.hpp
3. **SWIG compilation** → SWIG processes `.i` files to create language bindings
4. **Extension libraries** → Additional C# extension methods provide generic-like syntax

**Proven Benefits**:
- **Type-specific performance**: `readDataFloat()` avoids std::any conversion
- **Generic convenience**: Extension methods provide `ReadData<float>()` syntax
- **Complete coverage**: All scientific data types supported automatically

### 5. **CMake Integration**

Integrate SWIG generation into the existing CMake build system:

```cmake
# Find SWIG
find_package(SWIG REQUIRED)
include(UseSWIG)

# Function to generate SWIG bindings for a language
function(generate_swig_bindings LANGUAGE)
    set(SWIG_OUTPUT_DIR ${CMAKE_BINARY_DIR}/swig/${LANGUAGE})
    file(MAKE_DIRECTORY ${SWIG_OUTPUT_DIR})
    
    # Get all generated .i files
    file(GLOB SWIG_INTERFACES "${CMAKE_BINARY_DIR}/swig_interfaces/*.i")
    
    foreach(INTERFACE_FILE ${SWIG_INTERFACES})
        get_filename_component(MODULE_NAME ${INTERFACE_FILE} NAME_WE)
        
        set_property(SOURCE ${INTERFACE_FILE} PROPERTY CPLUSPLUS ON)
        set_property(SOURCE ${INTERFACE_FILE} PROPERTY SWIG_MODULE_NAME ${MODULE_NAME})
        
        swig_add_library(${MODULE_NAME}_${LANGUAGE}
            TYPE SHARED
            LANGUAGE ${LANGUAGE}
            SOURCES ${INTERFACE_FILE}
        )
        
        target_link_libraries(${MODULE_NAME}_${LANGUAGE} aqnwb::aqnwb)
        target_include_directories(${MODULE_NAME}_${LANGUAGE} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    endforeach()
endfunction()

# Generate bindings for supported languages
if(AQNWB_BUILD_SWIG_BINDINGS)
    generate_swig_bindings(csharp)
    generate_swig_bindings(python)
    generate_swig_bindings(java)
endif()
```

### 6. **C# Extension Method Pattern for Generic-Like Syntax**

Our working implementation revealed a powerful pattern for providing C# generic-like syntax on top of SWIG-generated type-specific methods. This addresses SWIG's limitation with true C# generics while providing familiar developer experience:

```csharp
// Extension methods that map to type-specific SWIG methods
public static class TimeSeriesExtensions
{
    public static object ReadData<T>(this TimeSeries ts)
    {
        var type = typeof(T);
        
        // Map C# types to SWIG-generated type-specific methods
        if (type == typeof(byte))        return ts.readDataUint8_T();
        else if (type == typeof(ushort)) return ts.readDataUint16_T();
        else if (type == typeof(uint))   return ts.readDataUint32_T();
        else if (type == typeof(ulong))  return ts.readDataUint64_T();
        else if (type == typeof(sbyte))  return ts.readDataInt8_T();
        else if (type == typeof(short))  return ts.readDataInt16_T();
        else if (type == typeof(int))    return ts.readDataInt32_T();
        else if (type == typeof(long))   return ts.readDataInt64_T();
        else if (type == typeof(float))  return ts.readDataFloat();
        else if (type == typeof(double)) return ts.readDataDouble();
        else if (type == typeof(string)) return ts.readDataStd_String();
        else if (type == typeof(object)) return ts.readData(); // std::any
        else throw new NotSupportedException($"Type {type.Name} not supported");
    }
}
```

**Key Benefits of This Pattern**:
- **Familiar syntax**: `ts.ReadData<float>()` looks like native C# generics
- **Optimal performance**: Extension methods map directly to type-specific SWIG methods
- **Zero runtime overhead**: Type checking happens at compile time
- **Maintainable**: Extension library is separate from generated SWIG code

**Multiple Usage Patterns Supported**:
```csharp
// Direct SWIG methods (fastest to write)
var data1 = ts.readDataFloat();

// Generic extension methods (nicest syntax)  
var data2 = ts.ReadData<float>();

// Fluent interface (object-oriented)
var reader = DataReaderFactory.CreateFloatReader(ts);
var data3 = reader.Data;

// Default method (most convenient)
var data4 = ts.readData();
```

This pattern can be automatically generated alongside SWIG bindings to provide optimal developer experience in C#.

---

## 🚀 Automated Workflow

### Schema-to-Bindings Pipeline

1. **Schema Processing**: The existing `generate_spec_files.py` processes NWB schema files
2. **Class Discovery**: Extract all NWB class definitions and their relationships
3. **Interface Generation**: Generate SWIG `.i` files for each class automatically
4. **Macro Preprocessing**: Expand C++ macros to expose generated methods to SWIG
5. **SWIG Compilation**: Generate language-specific bindings
6. **Integration**: Link generated bindings with the core aqnwb library

### Build Integration

```cmake
# Add custom target for SWIG interface generation
add_custom_target(generate_swig_interfaces
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/resources/generate_swig_interfaces.py
            ${PROJECT_SOURCE_DIR}/resources/schema/
            ${CMAKE_BINARY_DIR}/swig_interfaces/
    DEPENDS ${PROJECT_SOURCE_DIR}/resources/schema/
    COMMENT "Generating SWIG interface files from schema"
)

# Make SWIG bindings depend on interface generation
add_dependencies(swig_bindings generate_swig_interfaces)
```

---

## 📁 Proposed Repository Structure

```
aqnwb/
├── resources/
│   ├── generate_spec_files.py          # Extended to generate SWIG interfaces
│   ├── generate_swig_interfaces.py     # New: SWIG-specific generation
│   └── swig_templates/                 # Templates for SWIG interfaces
│       ├── base_class.i.template
│       ├── container.i.template
│       └── timeseries.i.template
├── cmake/
│   └── swig-bindings.cmake            # CMake functions for SWIG
├── swig/                              # Generated SWIG files (build-time)
│   ├── interfaces/                    # Generated .i files
│   ├── csharp/                       # C# bindings output
│   ├── python/                       # Python bindings output
│   └── java/                         # Java bindings output
└── CMakeLists.txt                    # Updated with SWIG support
```

---

## 🔄 Automatic Synchronization

### Schema Evolution Handling

1. **Automatic Detection**: When new classes are added to the schema, they are automatically detected during the build process
2. **Interface Generation**: New SWIG interfaces are generated automatically
3. **Dependency Tracking**: CMake tracks dependencies to rebuild only what's necessary
4. **Validation**: Automated tests ensure generated bindings work correctly

### Macro Evolution Support

The system handles changes to the macro system by:

1. **Preprocessing**: Always using the latest macro definitions during preprocessing
2. **Template Updates**: SWIG templates can be updated to handle new macro patterns
3. **Automatic Regeneration**: Full regeneration when macro definitions change

---

## 🧪 Testing Strategy

### Automated Testing

```cmake
# Add tests for each language binding
if(AQNWB_BUILD_SWIG_BINDINGS AND AQNWB_BUILD_TESTS)
    add_subdirectory(tests/swig)
endif()
```

### Test Structure

```
tests/swig/
├── python/
│   ├── test_timeseries.py
│   ├── test_electrical_series.py
│   └── test_nwbfile.py
├── csharp/
│   ├── TestTimeSeries.cs
│   └── TestElectricalSeries.cs
└── java/
    ├── TestTimeSeries.java
    └── TestElectricalSeries.java
```

---

## 🔬 Key Learnings from Working Implementation

Our working example implementation (see `swig_proposal/example/`) provided crucial insights that shaped this refined proposal:

### 1. **Template-Based Generation is Essential**
- **Discovery**: Manual SWIG interface writing doesn't scale to 100+ NWB classes
- **Solution**: Template-based generation with variable substitution enables automatic scaling
- **Implementation**: `timeseries.i.template` with `{{class_name}}`, `{{attribute_field_templates}}`, etc.

### 2. **All BaseRecordingData Types Must Be Instantiated**
- **Discovery**: Using only `std::any` creates performance bottlenecks for scientific data
- **Solution**: Generate type-specific methods for all 11 BaseRecordingData types automatically
- **Impact**: `readDataFloat()` vs `readData()` avoids costly std::any conversions

### 3. **C# Extension Method Pattern Solves SWIG Generics Limitation**
- **Discovery**: SWIG cannot generate true C# generics (`ReadData<T>()`)
- **Solution**: Extension methods map generic syntax to type-specific SWIG methods
- **Result**: Developers get familiar `ts.ReadData<float>()` syntax with optimal performance

### 4. **Directory Structure Must Match Proposed Architecture**
- **Discovery**: Clean separation of concerns is critical for maintainability
- **Implementation**: `resources/`, `cmake/`, `swig/interfaces/`, `swig/{language}/`
- **Benefit**: Clear organization scales to enterprise-level development

### 5. **CMake Integration is Straightforward**
- **Discovery**: SWIG integrates well with existing CMake infrastructure
- **Implementation**: `swig_add_library()` with proper dependency management
- **Result**: Seamless build process with automatic interface generation

### 6. **Multiple Usage Patterns Enhance Developer Experience**
- **Discovery**: Different developers prefer different API styles
- **Solution**: Support direct SWIG methods, generic extensions, and fluent interfaces
- **Example**: `ts.readDataFloat()`, `ts.ReadData<float>()`, `reader.Data` all work

### 7. **Automatic Generation Actually Works**
- **Validation**: Our example successfully generates working SWIG interfaces automatically
- **Proof**: `python3 resources/generate_swig_interfaces.py` creates functional `.i` files
- **Confidence**: The approach scales to real aqnwb implementation

---

## 🎁 Benefits of This Approach

### 1. **Zero Manual Maintenance**
- New NWB classes automatically get SWIG bindings
- No need to manually write or update `.i` files
- Macro-generated methods are automatically exposed

### 2. **Schema Consistency**
- Bindings are always in sync with the schema
- Type safety is preserved across languages
- Documentation is automatically generated

### 3. **Extensibility**
- Easy to add support for new languages
- Template-based approach allows customization
- Modular design supports incremental improvements

### 4. **Integration**
- Seamless integration with existing build system
- Leverages existing schema infrastructure
- Minimal impact on core library development

---

## 🚦 Implementation Phases

**Note**: Time estimates assume 1-2 developers working part-time (20-30 hours/week) on this project alongside other responsibilities.

### Phase 1: Foundation (2-3 weeks, ~40-60 hours)
- Extend `generate_spec_files.py` for SWIG interface generation (~15-20 hours)
- Create SWIG templates for common NWB patterns (~10-15 hours)
- Implement macro preprocessing pipeline (~10-15 hours)
- Initial testing and validation (~5-10 hours)

### Phase 2: Core Bindings (3-4 weeks, ~60-80 hours)
- Generate bindings for core NWB classes (TimeSeries, Container, etc.) (~20-25 hours)
- Implement CMake integration (~15-20 hours)
- Create basic test suite for one language (Python) (~15-20 hours)
- Debug and refine the generation process (~10-15 hours)

### Phase 3: Complete Coverage (2-3 weeks, ~40-60 hours)
- Generate bindings for all NWB classes (~15-20 hours)
- Add support for all three target languages (C#, Java) (~15-20 hours)
- Comprehensive testing and documentation (~10-20 hours)

### Phase 4: Optimization (1-2 weeks, ~20-40 hours)
- Performance optimization (~5-10 hours)
- Advanced features (callbacks, custom typemaps) (~10-15 hours)
- CI/CD integration (~5-15 hours)

**Total Estimated Effort**: 8-12 weeks (160-240 hours) for a complete implementation.

**Factors that could affect timeline**:
- Complexity of macro system edge cases
- SWIG-specific challenges with C++17 features
- Testing thoroughness requirements
- Documentation and packaging requirements

---

## 🔮 Future Enhancements

- **Package Distribution**: Automatic generation of language-specific packages (NuGet, PyPI, Maven)
- **Documentation Generation**: Automatic API documentation for each language
- **Advanced Features**: Support for callbacks, custom converters, and language-specific optimizations
- **IDE Integration**: IntelliSense/autocomplete support for generated bindings

This refined approach ensures that SWIG bindings for aqnwb are automatically maintained, preserving the convenience of the macro system while providing robust multi-language support that evolves with the core library.
