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

Status MeaningsTable::validateDataSpecs(
    const std::vector<DataSpecPtr>& dataSpecs) const
{
  return checkRequiredColumnNames({"id", "value", "meaning"}, dataSpecs);
}

std::vector<DynamicTable::DataSpecPtr> MeaningsTable::createDefaultDataSpecs(
    const AQNWB::IO::BaseDataType& valueDataType, const SizeType rowChunkSize)
{
  std::vector<DataSpecPtr> specs =
      DynamicTable::createDefaultDataSpecs(rowChunkSize);

  IO::ArrayDataSetConfig valueConfig(
      valueDataType, SizeArray {0}, SizeArray {rowChunkSize});
  specs.push_back(std::make_shared<VectorData::DataSpec>(
      "value",
      valueConfig,
      "The value of a row in the linked VectorData object."));

  IO::ArrayDataSetConfig meaningConfig(
      IO::BaseDataType::V_STR, SizeArray {0}, SizeArray {rowChunkSize});
  specs.push_back(std::make_shared<VectorData::DataSpec>(
      "meaning",
      meaningConfig,
      "The meaning of the value in the linked VectorData object."));

  return specs;
}

// Initialize the object
Status MeaningsTable::initialize(const VectorData& targetVectorData,
                                 const AQNWB::IO::BaseDataType& valueDataType,
                                 const std::string& description,
                                 const std::vector<DataSpecPtr>& columnSpecs)
{
  Status initStatus = Status::Success;

  // Retrieve the IO object
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr << "IO object has been deleted. Can't initialize MeaningsTable: "
              << m_path << std::endl;
    return Status::Failure;
  }

  std::vector<DataSpecPtr> specsToUse = columnSpecs;
  if (specsToUse.empty()) {
    specsToUse = createDefaultDataSpecs(valueDataType);
  }

  // Call parent initialize method. This initializes the DynamicTable and
  // creates the description attribute. Column names are written by
  // DynamicTable::finalize(). ElementIdentifiers ids are set in the
  // constructor.
  Status parentInitStatus = DynamicTable::initialize(description, specsToUse);
  initStatus = initStatus && parentInitStatus;

  // Set the target VectorData reference as an attribute
  Status targetStatus = ioPtr->createReferenceAttribute(
      targetVectorData.getPath(), m_path, "target");

  // Update the overall status and return the final status
  initStatus = initStatus && targetStatus;
  return initStatus;
}
