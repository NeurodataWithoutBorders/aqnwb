#pragma once

#include "nwb/hdmf/base/Container.hpp"
#include "ndx_labmetadata_example.hpp"
 
class LabMetaDataExtensionExample : public AQNWB::NWB::Container
{
    // Register the class with the type registry
    REGISTER_SUBCLASS(
        LabMetaDataExtensionExample, 
        Container,
        AQNWB::SPEC::NDX_LABMETADATA_EXAMPLE::namespaceName)

protected:
    // Constructor with path and io inputs required by RegisteredType
    // The constructor is protected to enforce use of the create() 
    // factory method for creating instances of this class.
    LabMetaDataExtensionExample(
        const std::string& path, 
        std::shared_ptr<AQNWB::IO::BaseIO> io);

public:
    // Method for initializing and writing the data in the NWB file
    Status initialize(const std::string& tissuePreparation);
 
    // Define methods for reading custom extension fields
    DEFINE_DATASET_FIELD(
        readTissuePreparation, 
        recordTissuePreparation,
        std::string, 
        "tissue_preparation", 
        Lab-specific description of the preparation of the tissue)

private:
    // LabMetaData objects are stored in the NWB file at this location
    const std::string m_nwbBasePath = "/general";
};
