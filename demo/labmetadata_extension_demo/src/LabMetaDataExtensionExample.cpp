#include "LabMetaDataExtensionExample.hpp"
#include "Utils.hpp"
#include <iostream>

using namespace AQNWB::NWB;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(LabMetaDataExtensionExample)

/** Constructor */ 
LabMetaDataExtensionExample::LabMetaDataExtensionExample(
    const std::string& path,
    std::shared_ptr<AQNWB::IO::BaseIO> io)
    : AQNWB::NWB::Container(path, io)
{
    // Check that our path points to expected location in the NWB file
    if (path.find(m_nwbBasePath) != 0) {
        std::cerr << "LabMetaData path expected to appear in "
                  << m_nwbBasePath <<  " in the NWB file" << std::endl;
    }
}

/** Initialize */
Status LabMetaDataExtensionExample::initialize(const std::string& tissuePreparation)
{
  Status containerStatus = Container::initialize();
  auto tissuePrepPath = AQNWB::mergePaths(m_path, "tissue_preparation");
  Status tissueDataStatus = m_io->createStringDataSet(tissuePrepPath, tissuePreparation);
  return containerStatus && tissueDataStatus;
}
