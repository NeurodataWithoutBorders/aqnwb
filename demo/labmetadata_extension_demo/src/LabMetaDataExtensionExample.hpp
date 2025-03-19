#pragma once

#include "nwb/hdmf/base/Container.hpp"
#include "ndx_labmetadata_example.hpp"
 
class LabMetaDataExtensionExample : public AQNWB::NWB::Container
{
public:
    /// Constructure with path and io inputs required by RegisteredType
    LabMetaDataExtensionExample(
        const std::string& path, 
        std::shared_ptr<AQNWB::IO::BaseIO> io);

    /// Metho for initalizing and writing the data in the NWB file 
    Status initialize(const std::string& tissuePreparation);
 
    /// Define methods for reading custom extensions files
    DEFINE_FIELD(
        readTissuePreparation, 
        AQNWB::NWB::DatasetField, 
        std::string, 
        "tissue_preparation", 
        Lab-specific description of the preparation of the tissue)
 
    /// Register the class with the type registry
    REGISTER_SUBCLASS(LabMetaDataExtensionExample, AQNWB::SPEC::NDX_LABMETADATA_EXAMPLE::namespaceName)

private:
    /// LabMetaData objects are stored in the NWB file at this location
    const std::string m_nwbBasePath = "/general";
};
