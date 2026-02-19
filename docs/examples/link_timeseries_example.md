# Using LinkArrayDataSetConfig for TimeSeries Data

This example demonstrates how to create multiple TimeSeries objects that share the same data via soft-links, which is useful for time-alignment scenarios.

## Use Case

When you have multiple TimeSeries that need to reference the same data array but with different timestamps or metadata, you can use `LinkArrayDataSetConfig` to create soft-links instead of duplicating the data.

## Example Code

```cpp
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/base/TimeSeries.hpp"

using namespace AQNWB;

int main() {
    // Create an NWB file
    std::string filepath = "linked_timeseries_example.nwb";
    auto io = std::make_shared<IO::HDF5::HDF5IO>(filepath);
    io->open();
    
    auto nwbfile = NWB::NWBFile::create(io);
    nwbfile->initialize();
    
    // Create the original TimeSeries with actual data
    auto originalSeries = NWB::TimeSeries::create("/acquisition/original_series", io);
    
    SizeType numSamples = 1000;
    IO::BaseDataType dataType = IO::BaseDataType::F32;
    IO::ArrayDataSetConfig dataConfig(dataType, SizeArray{0}, SizeArray{100});
    
    originalSeries->initialize(
        dataConfig,
        "volts",
        "Original electrical recording",
        "Recorded from electrode array",
        1.0f,   // conversion
        -1.0f,  // resolution
        0.0f    // offset
    );
    
    // Write data to the original series
    std::vector<float> data(numSamples);
    std::vector<double> timestamps(numSamples);
    // ... fill data and timestamps ...
    
    originalSeries->writeData(
        {numSamples}, {0}, 
        data.data(), 
        timestamps.data()
    );
    
    // Create a linked TimeSeries with different timestamps
    auto linkedSeries = NWB::TimeSeries::create("/acquisition/resampled_series", io);
    
    // Create link configuration pointing to the original data
    std::string linkTarget = "/acquisition/original_series/data";
    IO::LinkArrayDataSetConfig linkConfig(linkTarget, SizeArray{0}, SizeArray{100});
    
    linkedSeries->initialize(
        linkConfig,  // Use link instead of creating new data
        "volts",
        "Resampled version with same data but different timestamps",
        "Time-aligned to stimulus onset",
        1.0f,
        -1.0f,
        0.0f
    );
    
    // Write only the new timestamps (data is linked)
    std::vector<double> newTimestamps(numSamples);
    // ... fill with resampled timestamps ...
    
    auto timestampRecorder = linkedSeries->recordTimestamps();
    timestampRecorder->writeDataBlock(
        {numSamples}, {0},
        IO::BaseDataType::F64,
        newTimestamps.data()
    );
    
    io->stopRecording();
    io->close();
    
    return 0;
}
```

## Benefits

1. **Storage Efficiency**: Data is stored only once, saving disk space
2. **Consistency**: Changes to the original data are automatically reflected in all linked series
3. **Flexibility**: Each linked series can have its own metadata, timestamps, and attributes
4. **NWB Compliance**: Soft-links are a standard HDF5 feature fully supported by NWB

## Verification

You can verify the links were created correctly using h5ls:

```bash
h5ls -r linked_timeseries_example.nwb
```

Output:
```
/acquisition/original_series/data     Dataset {1000/Inf}
/acquisition/resampled_series/data    Soft Link {/acquisition/original_series/data}
```

## Notes

- The `LinkArrayDataSetConfig` constructor requires the target path and optionally shape/chunking
- Shape and chunking are used to configure related datasets (e.g., timestamps)
- `createArrayDataSet()` returns `nullptr` for links since you cannot write directly to a link
- Links can only point to datasets within the same HDF5 file
