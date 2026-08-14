#include "nwb/hdmf/table/VectorIndex.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(VectorIndex)

// Constructor
VectorIndex::VectorIndex(const std::string& path,
                         std::shared_ptr<AQNWB::IO::BaseIO> io)
    : VectorData(path, io)
{
}

// Initialize the object
Status VectorIndex::initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                               const std::string& description,
                               const VectorData& target)
{
  Status initStatus = Status::Success;

  // Get the io pointer.
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "VectorIndex:initialize IO object has been deleted."
              << std::endl;
    return Status::Failure;
  }
  if (!ioPtr->canModifyObjects()) {
    std::cerr << "VectorIndex::initialize IO object cannot modify objects."
              << std::endl;
    return Status::Failure;
  }

  // Make sure that dataConfig uses a uint data type as required
  // Extract shape, chunking, and data type from the config
  SizeArray shape, chunking;
  BaseDataType dataType;
  Status propStatus =
      dataConfig.getProperties(ioPtr.get(), shape, chunking, dataType);
  if (!propStatus) {
    std::cerr << "VectorIndex::initialize could not retrieve dataset properties"
              << std::endl;
    return Status::Failure;
  }

  if (dataType != IO::BaseDataType::Type::T_U8
      && dataType != IO::BaseDataType::Type::T_U16
      && dataType != IO::BaseDataType::Type::T_U32
      && dataType != IO::BaseDataType::Type::T_U64)
  {
    throw std::invalid_argument(
        "VectorIndex::initalize invalid dataConfig requires a uint type");
  }
  if (shape.size() != 1) {
    throw std::invalid_argument(
        "VectorIndex::initialize invalid dataConfig requires 1D shape");
  }

  // Call parent initialize method.
  Status parentInitStatus = VectorData::initialize(dataConfig, description);
  initStatus = initStatus && parentInitStatus;
  ;

  // Initialize the target attribute
  ioPtr->createReferenceAttribute(target.getPath(), getPath(), "target");

  return initStatus;
}
