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
                               const std::string& targetPath)
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

  // Make sure that dataConfig uses a unsigned int data type as required
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

  if (dataType.type != IO::BaseDataType::Type::T_U8
      && dataType.type != IO::BaseDataType::Type::T_U16
      && dataType.type != IO::BaseDataType::Type::T_U32
      && dataType.type != IO::BaseDataType::Type::T_U64)
  {
    throw std::invalid_argument(
        "VectorIndex::initialize invalid dataConfig requires a unsigned int "
        "type");
  }
  if (shape.size() != 1) {
    throw std::invalid_argument(
        "VectorIndex::initialize invalid dataConfig requires 1D shape");
  }

  // Call parent initialize method.
  Status parentInitStatus = VectorData::initialize(dataConfig, description);
  initStatus = initStatus && parentInitStatus;

  // Initialize the target attribute
  ioPtr->createReferenceAttribute(targetPath, getPath(), "target");

  // Save the data type and current index for use in appendRow
  m_dataType = dataType;
  m_dataTypeInitialized = true;
  m_currentIndex = 0;
  m_currentIndexInitialized = true;

  return initStatus;
}

Status VectorIndex::initializeAppendState()
{
  if (m_currentIndexInitialized && m_dataTypeInitialized) {
    return Status::Success;
  }

  auto dataset = readData();
  if (!dataset) {
    std::cerr
        << "VectorIndex::initializeAppendState: dataset is not initialized."
        << std::endl;
    return Status::Failure;
  }

  auto ioPtr = getIO();
  if (!ioPtr) {
    std::cerr
        << "VectorIndex::initializeAppendState: IO object has been deleted."
        << std::endl;
    return Status::Failure;
  }

  if (!m_dataTypeInitialized) {
    m_dataType = ioPtr->getStorageObjectDataType(dataset->getPath());
    m_dataTypeInitialized = true;
  }

  if (m_currentIndexInitialized) {
    return Status::Success;
  }

  auto shape = dataset->getShape();
  if (shape.empty() || shape[0] == 0) {
    m_currentIndex = 0;
    m_currentIndexInitialized = true;
    return Status::Success;
  }

  // Read the last value
  SizeArray positionOffset = {shape[0] - 1};
  SizeArray readShape = {1};
  uint64_t lastValue = 0;

  Status readStatus = Status::Failure;

  try {
    auto dataBlock = dataset->valuesGeneric(positionOffset, readShape);
    auto variantData = dataBlock.as_variant();

    if (m_dataType.type == IO::BaseDataType::Type::T_U8) {
      if (auto* vec = std::get_if<std::vector<uint8_t>>(&variantData)) {
        if (!vec->empty()) {
          lastValue = vec->front();
          readStatus = Status::Success;
        }
      }
    } else if (m_dataType.type == IO::BaseDataType::Type::T_U16) {
      if (auto* vec = std::get_if<std::vector<uint16_t>>(&variantData)) {
        if (!vec->empty()) {
          lastValue = vec->front();
          readStatus = Status::Success;
        }
      }
    } else if (m_dataType.type == IO::BaseDataType::Type::T_U32) {
      if (auto* vec = std::get_if<std::vector<uint32_t>>(&variantData)) {
        if (!vec->empty()) {
          lastValue = vec->front();
          readStatus = Status::Success;
        }
      }
    } else if (m_dataType.type == IO::BaseDataType::Type::T_U64) {
      if (auto* vec = std::get_if<std::vector<uint64_t>>(&variantData)) {
        if (!vec->empty()) {
          lastValue = vec->front();
          readStatus = Status::Success;
        }
      }
    } else {
      std::cerr << "VectorIndex::initializeAppendState: unsupported data type."
                << std::endl;
      return Status::Failure;
    }
  } catch (const std::exception& e) {
    std::cerr << "VectorIndex::initializeAppendState: error reading data: "
              << e.what() << std::endl;
    return Status::Failure;
  }

  if (readStatus == Status::Success) {
    m_currentIndex = lastValue;
    m_currentIndexInitialized = true;
  }

  return readStatus;
}

Status VectorIndex::appendData(const CellValue& targetValues,
                               size_t& elementsAppended)
{
  if (!m_currentIndexInitialized || !m_dataTypeInitialized) {
    Status initStatus = initializeAppendState();
    if (initStatus != Status::Success) {
      return initStatus;
    }
  }

  auto target = m_targetColumn;
  if (!target) {
    target = readTarget();
    if (target) {
      setTargetColumn(target);
    } else {
      std::cerr
          << "VectorIndex::appendData: target VectorData is not available."
          << std::endl;
      return Status::Failure;
    }
  }

  elementsAppended = 0;
  Status appendStatus = target->appendData(targetValues, elementsAppended);
  if (appendStatus != Status::Success) {
    return appendStatus;
  }

  m_currentIndex += elementsAppended;

  // Now append the new index to this VectorIndex
  // We need to append the value based on the actual data type of the dataset
  auto ioPtr = getIO();
  if (!ioPtr) {
    return Status::Failure;
  }
  size_t indexElementsAppended = 0;

  if (m_dataType.type == IO::BaseDataType::Type::T_U8) {
    CellValue indexValue(static_cast<uint8_t>(m_currentIndex));
    return VectorData::appendData(indexValue, indexElementsAppended);
  } else if (m_dataType.type == IO::BaseDataType::Type::T_U16) {
    CellValue indexValue(static_cast<uint16_t>(m_currentIndex));
    return VectorData::appendData(indexValue, indexElementsAppended);
  } else if (m_dataType.type == IO::BaseDataType::Type::T_U32) {
    CellValue indexValue(static_cast<uint32_t>(m_currentIndex));
    return VectorData::appendData(indexValue, indexElementsAppended);
  } else if (m_dataType.type == IO::BaseDataType::Type::T_U64) {
    CellValue indexValue(m_currentIndex);
    return VectorData::appendData(indexValue, indexElementsAppended);
  } else {
    std::cerr << "VectorIndex::appendData: unsupported data type." << std::endl;
    return Status::Failure;
  }
}
