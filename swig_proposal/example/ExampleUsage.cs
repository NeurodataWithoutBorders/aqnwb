/*
 * Example C# usage of the generated SWIG bindings for TimeSeries.
 * 
 * This demonstrates how users would interact with the aqnwb library
 * through the automatically generated C# bindings, particularly useful
 * for Bonsai integration.
 */

using System;
// using AqnwbTimeseries; // This would be the actual namespace after SWIG compilation

namespace AqnwbExample
{
    class TimeSeriesExample
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Example TimeSeries Usage in C#");
            Console.WriteLine("==============================");
            
            DemonstrateTimeSeriesUsage();
            DemonstrateMacroBenefits();
        }
        
        static void DemonstrateTimeSeriesUsage()
        {
            Console.WriteLine(@"
// Example of what the API would look like after SWIG compilation:

// Create an IO object
var io = new HDF5IO(""example.nwb"");

// Create a TimeSeries object  
var ts = new TimeSeries(""/acquisition/timeseries"", io);

// APPROACH 1: Direct SWIG methods (fastest, most explicit)
var description = ts.readDescription();     // Default std::string
var comments = ts.readComments();           // Default std::string
var dataAny = ts.readData();                // Default std::any

// All BaseRecordingData types available as type-specific methods
var dataFloat = ts.readDataFloat();         // 32-bit float
var dataDouble = ts.readDataDouble();       // 64-bit double
var dataInt32 = ts.readDataInt32_T();       // 32-bit signed int
var dataUint8 = ts.readDataUint8_T();       // 8-bit unsigned int
var dataInt64 = ts.readDataInt64_T();       // 64-bit signed int
var dataString = ts.readDataStd_String();   // String data

Console.WriteLine($""Description: {description.get()}"");
Console.WriteLine($""Data (float): {dataFloat.getShape()}"");

// APPROACH 2: Generic extension methods (convenient, looks like C# generics)
using AqnwbExtensions;

// All BaseRecordingData types available via generics
var genericFloat = ts.ReadData<float>();    // -> readDataFloat()
var genericDouble = ts.ReadData<double>();  // -> readDataDouble()
var genericInt = ts.ReadData<int>();        // -> readDataInt32_T()
var genericByte = ts.ReadData<byte>();      // -> readDataUint8_T()
var genericLong = ts.ReadData<long>();      // -> readDataInt64_T()
var genericString = ts.ReadData<string>();  // -> readDataStd_String()
var genericTimestamps = ts.ReadTimestamps<double>(); // Extension method

Console.WriteLine($""Generic float data: {genericFloat.getShape()}"");
Console.WriteLine($""Generic byte data: {genericByte.getShape()}"");

// APPROACH 3: Fluent typed reader (object-oriented)
var floatReader = DataReaderFactory.CreateFloatReader(ts);
var readerData = floatReader.Data;          // Typed data access
var readerTimestamps = floatReader.Timestamps; // Typed timestamp access

Console.WriteLine($""Reader data: {readerData.getShape()}"");

// APPROACH 4: Generic attribute reading
var attrDescription = ts.ReadAttribute<string>(""description"");
var attrConversion = ts.ReadAttribute<float>(""conversion"");

Console.WriteLine($""Attribute description: {attrDescription.get()}"");

// Read timestamps (default double)
var timestamps = ts.readTimestamps();
Console.WriteLine($""Number of timestamps: {timestamps.getSize()}"");

// Read conversion factor (default float)
var conversion = ts.readDataConversion();
Console.WriteLine($""Conversion factor: {conversion.get()}"");

// Read unit information (default string)
var unit = ts.readDataUnit();
Console.WriteLine($""Data unit: {unit.get()}"");

// Write data (using the write accessor)
var writeData = ts.recordData();
writeData.writeData(newDataArray);

// Write timestamps
var writeTimestamps = ts.recordTimestamps();
writeTimestamps.writeData(newTimestampsArray);
");
        }
        
        static void DemonstrateMacroBenefits()
        {
            Console.WriteLine("\nBenefits for Bonsai Integration:");
            Console.WriteLine("--------------------------------");
            
            Console.WriteLine(@"
The SWIG bindings provide excellent integration with Bonsai:

1. .NET Native Integration:
   - Direct C# classes and methods
   - No P/Invoke or marshaling overhead
   - Full IntelliSense support in Visual Studio

2. Type Safety:
   - Compile-time type checking
   - Automatic memory management
   - Exception handling

3. Bonsai Workflow Integration:
   - TimeSeries objects can be used directly in Bonsai nodes
   - Streaming data support through recordData() methods
   - Real-time data acquisition and storage

4. Performance:
   - Direct access to C++ performance
   - Minimal overhead for data operations
   - Efficient memory usage

Example Bonsai Node Usage:
```csharp
public class TimeSeriesNode : Sink<double[]>
{
    public string FilePath { get; set; }
    public string TimeSeriesPath { get; set; }
    
    private TimeSeries timeSeries;
    private BaseRecordingData recordData;
    
    public override void Process(double[] input)
    {
        // Write data directly to NWB file
        recordData.writeData(input);
    }
}
```
");
        }
    }
}
