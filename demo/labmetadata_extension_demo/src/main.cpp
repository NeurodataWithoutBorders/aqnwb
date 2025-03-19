#include "LabMetaDataExtensionExample.hpp"
#include "nwb/NWBFile.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include <iostream>

// The main routine implementing the main workflow of this demo application
int main(int argc, char* argv[])
{
    // Create the HDF5 I/O object for write
    std::string filePath = "testLabMetaDataExtensionExample.nwb";
    
    // Print progress status
    std::cout << std::endl << "Opening NWB file: " << filePath << std::endl;
 
    // Create an IO object for writing the NWB file
    std::shared_ptr<AQNWB::IO::BaseIO> io = std::make_shared<AQNWB::IO::HDF5::HDF5IO>(filePath);
    io->open(AQNWB::IO::FileMode::Overwrite);

    // Create the NWBFile
    std::shared_ptr<AQNWB::NWB::NWBFile> nwbfile = std::make_shared<AQNWB::NWB::NWBFile>(io);
    nwbfile->initialize("test_identifier", "Test NWB File", "Data collection info");

    // Create the LabMetaDataExtensionExample object
    std::string labMetaDataPath = AQNWB::mergePaths("/general", "custom_lab_metadata");
    std::shared_ptr<LabMetaDataExtensionExample> labMetaData = std::make_shared<LabMetaDataExtensionExample>(labMetaDataPath, io);
    std::cout << "Writing "<< labMetaData->getPath() << " extension data" << std::endl;
    labMetaData->initialize("Tissue preparation details");

    // Close the file
    io->close();
    std::cout << "Finished data write. Starting read." <<std::endl;

    // Create a new HDF5 I/O object for read
    std::shared_ptr<AQNWB::IO::BaseIO> readIO = std::make_shared<AQNWB::IO::HDF5::HDF5IO>(filePath);
    readIO->open(AQNWB::IO::FileMode::ReadOnly);
    
    // Read the LabMetaDataExtensionExample object from the file
    std::shared_ptr<LabMetaDataExtensionExample> readLabMetaData = std::make_shared<LabMetaDataExtensionExample>(labMetaDataPath, readIO);

    // Read the LabMetaDataExtensionExample.readTissuePreparation field
    auto tissuePreparationWrapper = readLabMetaData->readTissuePreparation();
    std::string tissuePreparation = tissuePreparationWrapper->values().data[0];
    std::cout << "Read Tissue Preparation: " << tissuePreparation << std::endl;

    // Close the read file
    readIO->close();
    
    return 0;
}
