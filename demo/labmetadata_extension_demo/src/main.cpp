#include "LabMetaDataExtensionExample.hpp"
#include "nwb/NWBFile.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "Utils.hpp"
#include <iostream>


// The main routine implementing the main workflow of this demo application
int main(int argc, char* argv[])
{
    // Create the HDF5 I/O object for write
    std::string filePath = "testLabMetaDataExtensionExample.nwb";
    
    // Print progress status
    std::cout << std::endl << "Opening NWB file: " << filePath << std::endl;
 
    // Create an IO object for writing the NWB file
    auto io = AQNWB::createIO("HDF5", filePath);
    io->open(AQNWB::IO::FileMode::Overwrite);

    // Create the NWBFile
    auto nwbfile = AQNWB::NWB::NWBFile::create(io);
    nwbfile->initialize("test_identifier", "Test NWB File", "Data collection info");

    // Create the LabMetaDataExtensionExample object
    std::string labMetaDataPath = AQNWB::mergePaths("/general", "custom_lab_metadata");
    auto labMetaData = LabMetaDataExtensionExample::create(labMetaDataPath, io);
    std::cout << "Writing "<< labMetaData->getPath() << " extension data" << std::endl;
    labMetaData->initialize("Tissue preparation details");

    // Close the file
    io->getRecordingObjects()->finalize();
    io->close();
    std::cout << "Finished data write. Starting read." <<std::endl;

    // Create a new HDF5 I/O object for read
    auto readIO = AQNWB::createIO("HDF5", filePath);
    readIO->open(AQNWB::IO::FileMode::ReadOnly);
    
    // Read the LabMetaDataExtensionExample object from the file
    auto readLabMetaData = LabMetaDataExtensionExample::create(labMetaDataPath, readIO);

    // Read the LabMetaDataExtensionExample.readTissuePreparation field
    auto tissuePreparationWrapper = readLabMetaData->readTissuePreparation();
    std::string tissuePreparation = tissuePreparationWrapper->values().data[0];
    std::cout << "Read Tissue Preparation: " << tissuePreparation << std::endl;

    // Close the read file
    readIO->close();
    
    return 0;
}
