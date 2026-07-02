#include "MeaningsTable.hpp"

#include "Utils.hpp"
// Includes for referenced types
#include "nwb/hdmf/table/VectorData.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(MeaningsTable)

// Constructor
MeaningsTable::MeaningsTable(const std::string& path,
                             std::shared_ptr<AQNWB::IO::BaseIO> io)
    : DynamicTable(path, io)
{
}

// Initialize the object
Status MeaningsTable::initialize(const VectorData& targetVectorData,
                                 const AQNWB::IO::BaseDataType& valueDataType,
                                 const std::string& description,
                                 const SizeType rowChunkSize)
{
  Status initStatus = Status::Success;

  // Retrieve the IO object
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr << "IO object has been deleted. Can't initialize MeaningsTable: "
              << m_path << std::endl;
    return Status::Failure;
  }

  // Call parent initialize method. This initializes the DynamicTable and
  // creates the description attribute. Column names are written by
  // DynamicTable::finalize(). ElementIdentifiers ids are set in the
  // constructor.
  Status parentInitStatus = DynamicTable::initialize(description);
  initStatus = initStatus && parentInitStatus;

  // Initialize value dataset
  auto valuePath = AQNWB::mergePaths(m_path, "value");
  IO::ArrayDataSetConfig valueConfig(
      valueDataType, SizeArray {0}, SizeArray {rowChunkSize});
  auto valuesColumn = VectorData::create(valuePath, ioPtr);
  Status valueStatus = valuesColumn->initialize(
      valueConfig, "The value of a row in the linked VectorData object.");
  Status addValueColumnStatus = addColumn(valuesColumn);

  // Initialize meaning dataset
  auto meaningPath = AQNWB::mergePaths(m_path, "meaning");
  IO::ArrayDataSetConfig meaningConfig(
      IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {rowChunkSize});
  auto meaningColumn = VectorDataTyped<std::string>::create(meaningPath, ioPtr);
  Status meaningStatus = meaningColumn->initialize(
      meaningConfig,
      "The meaning of the value in the linked VectorData object.");
  Status addMeaningColumnStatus = addColumn(meaningColumn);

  // Set the target VectorData reference
  auto targetPath = AQNWB::mergePaths(m_path, "target");
  Status targetStatus =
      ioPtr->createLink(targetPath, targetVectorData.getPath());

  // Update the overall status and return the final status
  initStatus = initStatus && valueStatus && addValueColumnStatus
      && meaningStatus && addMeaningColumnStatus && targetStatus;
  return initStatus;
}
