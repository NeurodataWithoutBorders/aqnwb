#include "nwb/hdmf/table/DynamicTableRegion.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;

// DynamicTableRegion
// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(DynamicTableRegion)

/** Constructor */
DynamicTableRegion::DynamicTableRegion(const std::string& path,
                                       std::shared_ptr<IO::BaseIO> io)
    : VectorData(path, io)
{
}

/** Destructor */
DynamicTableRegion::~DynamicTableRegion() {}

Status DynamicTableRegion::DataSpec::initialize(Data& data) const
{
  auto* region = dynamic_cast<DynamicTableRegion*>(&data);
  if (!region) {
    std::cerr << "DynamicTableRegion::DataSpec::initialize received an "
                 "incompatible object"
              << std::endl;
    return Status::Failure;
  }
  return region->initialize(getDataConfig(),
                            description,
                            tablePath);
}

std::shared_ptr<DynamicTableRegion::DataSpec>
DynamicTableRegion::createDataSpec(const std::string& name,
                                   const IO::ArrayDataSetConfig& dataConfig,
                                   const std::string& description,
                                   const std::string& tablePath)
{
  return std::make_shared<DataSpec>(name, dataConfig, description, tablePath);
}

Status DynamicTableRegion::initialize(
    const IO::BaseArrayDataSetConfig& dataConfig,
    const std::string& description,
    const std::string& tablePath)
{
  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr << "DynamicTableRegion::initialize: IO object is not valid."
              << std::endl;
    return Status::Failure;
  }

  // The indices must be integers and the dataset must be 1D
  SizeArray shape, chunking;
  IO::BaseDataType dataType;
  if (dataConfig.getProperties(ioPtr.get(), shape, chunking, dataType)
      != Status::Success)
  {
    std::cerr << "DynamicTableRegion::initialize: Could not retrieve the "
                 "dataset properties."
              << std::endl;
    return Status::Failure;
  }
  const bool isInteger = dataType.type == IO::BaseDataType::Type::T_I8
      || dataType.type == IO::BaseDataType::Type::T_I16
      || dataType.type == IO::BaseDataType::Type::T_I32
      || dataType.type == IO::BaseDataType::Type::T_I64
      || dataType.type == IO::BaseDataType::Type::T_U8
      || dataType.type == IO::BaseDataType::Type::T_U16
      || dataType.type == IO::BaseDataType::Type::T_U32
      || dataType.type == IO::BaseDataType::Type::T_U64;
  if (!isInteger || shape.size() != 1) {
    std::cerr << "DynamicTableRegion::initialize: The data must be a 1D array "
                 "of integers."
              << std::endl;
    return Status::Failure;
  }
  if (!ioPtr->objectExists(tablePath)) {
    std::cerr << "DynamicTableRegion::initialize: The table " << tablePath
              << " does not exist." << std::endl;
    return Status::Failure;
  }

  Status status = VectorData::initialize(dataConfig, description);
  status =
      status && ioPtr->createReferenceAttribute(tablePath, m_path, "table");
  return status;
}
