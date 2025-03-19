#pragma once

#include "nwb/hdmf/base/Container.hpp"
#include "ndx_labmetadata_example.hpp"
 
class LabMetaDataExtensionExample : public AQNWB::NWB::Container
{
public:
    LabMetaDataExtensionExample(
        const std::string& path, 
        std::shared_ptr<AQNWB::IO::BaseIO> io);

    Status initialize(const std::string& tissuePreparation);
 
    DEFINE_FIELD(
        readTissuePreparation, 
        AQNWB::NWB::DatasetField, 
        std::string, 
        "tissue_preparation", 
        Lab-specific description of the preparation of the tissue)
 
    // Using the custom macro with the namespace variable
    REGISTER_SUBCLASS(LabMetaDataExtensionExample, AQNWB::SPEC::NDX_LABMETADATA_EXAMPLE::namespaceName)

private:
    /// LabMetaData is stored in the NWB file at this location
    const std::string m_nwbBasePath = "/general";
};
