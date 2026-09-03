#include <algorithm>

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
  if (propStatus != Status::Success) {
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
  Status targetStatus =
      ioPtr->createReferenceAttribute(targetPath, getPath(), "target");
  initStatus = initStatus && targetStatus;

  // Save the data type and current index for use in appendRow
  m_dataType = dataType;
  m_dataTypeInitialized = true;
  m_currentIndex = 0;
  m_currentIndexInitialized = true;

  return initStatus;
}

std::shared_ptr<VectorData> VectorIndex::getTargetColumn()
{
  if (!m_targetColumn) {
    m_targetColumn = readTarget();
  }
  return m_targetColumn;
}

uint64_t VectorIndex::extractIndexValue(
    const AQNWB::Types::CellValue& cell) const
{
  uint64_t index = 0;
  // A CellValue contains a variant (cell.value) which can be a scalar or a
  // vector. We expect an index to be a scalar value.
  std::visit(
      [&](auto&& arg)
      {
        using T = std::decay_t<decltype(arg)>;
        // Check if the variant holds a scalar value
        if constexpr (std::is_same_v<T, AQNWB::Types::ScalarDataVariant>) {
          // The scalar value itself is another variant holding the actual type
          // (e.g., uint32_t, uint64_t)
          std::visit(
              [&](auto&& scalarArg)
              {
                using ScalarT = std::decay_t<decltype(scalarArg)>;
                // Ensure the scalar is an integral type before casting to
                // uint64_t
                if constexpr (std::is_integral_v<ScalarT>) {
                  index = static_cast<uint64_t>(scalarArg);
                }
              },
              arg);
        }
      },
      cell.value);
  return index;
}

AQNWB::Types::CellValue VectorIndex::combineCellsToVector(
    const std::vector<AQNWB::Types::CellValue>& cells,
    size_t offset,
    size_t count) const
{
  // Handle edge cases where the requested slice is empty or out of bounds
  if (offset >= cells.size() || count == 0) {
    return AQNWB::Types::CellValue(std::vector<uint8_t> {});
  }

  AQNWB::Types::CellValue result(std::vector<uint8_t> {});

  // We need to determine the underlying data type of the cells to create a
  // vector of the same type. We inspect the first cell in the requested slice
  // to find its type.
  std::visit(
      [&](auto&& arg)
      {
        using T = std::decay_t<decltype(arg)>;
        // We expect the individual cells to be scalar values
        if constexpr (std::is_same_v<T, AQNWB::Types::ScalarDataVariant>) {
          std::visit(
              [&](auto&& scalarArg)
              {
                using ScalarT = std::decay_t<decltype(scalarArg)>;
                // Ensure the scalar is not empty (monostate)
                if constexpr (!std::is_same_v<ScalarT, std::monostate>) {
                  // Create a vector of the discovered type to hold the combined
                  // elements
                  std::vector<ScalarT> vec;
                  vec.reserve(count);

                  // Iterate over the requested slice of cells
                  for (size_t i = 0; i < count; ++i) {
                    if (offset + i >= cells.size()) {
                      break;
                    }

                    // Extract the value from each cell and add it to our vector
                    std::visit(
                        [&](auto&& cellArg)
                        {
                          using CellT = std::decay_t<decltype(cellArg)>;
                          if constexpr (std::is_same_v<
                                            CellT,
                                            AQNWB::Types::ScalarDataVariant>)
                          {
                            std::visit(
                                [&](auto&& cellScalarArg)
                                {
                                  using CellScalarT =
                                      std::decay_t<decltype(cellScalarArg)>;
                                  // Only add the value if its type matches the
                                  // type of the first cell
                                  if constexpr (std::is_same_v<CellScalarT,
                                                               ScalarT>) {
                                    vec.push_back(cellScalarArg);
                                  }
                                },
                                cellArg);
                          }
                        },
                        cells[offset + i].value);
                  }
                  // Wrap the populated vector in a CellValue
                  result = AQNWB::Types::CellValue(std::move(vec));
                }
              },
              arg);
        }
      },
      cells[offset].value);

  return result;
}

std::vector<AQNWB::Types::CellValue> VectorIndex::readIndexedCellValues(
    SizeType start, SizeType count)
{
  if (count == 0) {
    return {};
  }

  auto ioPtr = getIO();
  if (!ioPtr || !ioPtr->isOpen()) {
    throw std::runtime_error(
        "VectorIndex::readIndexedCellValues: IO object has been deleted or is "
        "closed.");
  }

  auto dataset = readData();
  if (!dataset || !dataset->exists()) {
    throw std::runtime_error(
        "VectorIndex::readIndexedCellValues: dataset is not initialized or "
        "does not exist.");
  }

  // First, read the indices from this VectorIndex
  SizeArray startArray = {start};
  SizeArray countArray =
      count != AQNWB::Types::SizeTypeNotSet ? SizeArray {count} : SizeArray {};

  std::vector<AQNWB::Types::CellValue> indices =
      VectorData::readCellValues(startArray, countArray);

  if (indices.empty()) {
    return {};
  }

  // Get the target column
  auto target = getTargetColumn();
  if (!target) {
    throw std::runtime_error(
        "VectorIndex::readIndexedCellValues: target VectorData is not "
        "available.");
  }

  // We need to read the previous index to know where the first vector starts
  // If start == 0, the previous index is 0
  uint64_t prevIndex = 0;
  if (start > 0) {
    SizeArray prevStart = {start - 1};
    SizeArray prevCount = {1};
    std::vector<AQNWB::Types::CellValue> prevIndices =
        VectorData::readCellValues(prevStart, prevCount);
    if (!prevIndices.empty()) {
      prevIndex = extractIndexValue(prevIndices[0]);
    }
  }

  // Extract all current indices
  std::vector<uint64_t> currIndices;
  currIndices.reserve(indices.size());
  std::transform(indices.begin(),
                 indices.end(),
                 std::back_inserter(currIndices),
                 [this](const auto& indexCell)
                 { return extractIndexValue(indexCell); });

  // Validate indices and find total range to read
  uint64_t totalElementsToRead = 0;
  uint64_t lastIndex = prevIndex;
  for (uint64_t currIndex : currIndices) {
    if (currIndex < lastIndex) {
      throw std::invalid_argument(
          "VectorIndex::readIndexedCellValues: Invalid index data, "
          "currIndex ("
          + std::to_string(currIndex) + ") < prevIndex ("
          + std::to_string(lastIndex) + ")");
    }
    lastIndex = currIndex;
  }

  if (!currIndices.empty()) {
    totalElementsToRead = currIndices.back() - prevIndex;
  }

  // Validate that the indices do not exceed the target dataset size
  if (totalElementsToRead > 0) {
    SizeArray targetShape;
    bool targetExists = ioPtr->objectExists(target->getPath());
    if (!targetExists) {
      throw std::runtime_error(
          "VectorIndex::readIndexedCellValues: Target dataset does not exist.");
    }

    try {
      targetShape = ioPtr->getStorageObjectShape(target->getPath());
    } catch (const std::exception& e) {
      throw std::runtime_error(
          "VectorIndex::readIndexedCellValues: Failed to get target dataset "
          "shape: "
          + std::string(e.what()));
    }

    if (!targetShape.empty() && currIndices.back() > targetShape[0]) {
      throw std::out_of_range(
          "VectorIndex::readIndexedCellValues: Index out of bounds. "
          "Max index ("
          + std::to_string(currIndices.back())
          + ") exceeds target dataset size (" + std::to_string(targetShape[0])
          + ").");
    } else if (targetShape.empty() && currIndices.back() > 0) {
      throw std::out_of_range(
          "VectorIndex::readIndexedCellValues: Index out of bounds. "
          "Max index ("
          + std::to_string(currIndices.back())
          + ") exceeds target dataset size (0).");
    }
  }

  // Empty rows have no values from which to infer their element type, so use
  // the target dataset metadata to create correctly typed empty vectors.
  auto targetData = target->readData();
  if (!targetData) {
    throw std::runtime_error(
        "VectorIndex::readIndexedCellValues: target data is unavailable.");
  }
  const auto emptyTargetValues =
      IO::BaseDataType::createEmptyVectorVariant(targetData->getDataType());
  if (std::holds_alternative<std::monostate>(emptyTargetValues)) {
    throw std::runtime_error(
        "VectorIndex::readIndexedCellValues: target data type is unsupported.");
  }

  // Read nonempty target values in one operation before slicing them into rows.
  std::vector<AQNWB::Types::CellValue> allTargetCells;
  if (totalElementsToRead > 0) {
    SizeArray targetStart = {static_cast<SizeType>(prevIndex)};
    SizeArray targetCount = {static_cast<SizeType>(totalElementsToRead)};
    try {
      allTargetCells = target->readCellValues(targetStart, targetCount);
    } catch (const std::out_of_range&) {
      throw;  // Re-throw out_of_range
    } catch (const std::exception& e) {
      throw std::runtime_error(
          "VectorIndex::readIndexedCellValues: Failed to read target cells: "
          + std::string(e.what()));
    }
  }

  std::vector<AQNWB::Types::CellValue> result;
  result.reserve(indices.size());

  // Slice the bulk-read target cells into individual vectors
  size_t targetCellOffset = 0;
  for (uint64_t currIndex : currIndices) {
    uint64_t numElements = currIndex - prevIndex;

    if (numElements == 0) {
      result.emplace_back(emptyTargetValues);
    } else {
      result.push_back(
          combineCellsToVector(allTargetCells, targetCellOffset, numElements));
      targetCellOffset += numElements;
    }

    prevIndex = currIndex;
  }

  return result;
}

Status VectorIndex::initializeAppendState()
{
  if (m_currentIndexInitialized && m_dataTypeInitialized) {
    return Status::Success;
  }

  auto dataset = readData();
  if (!dataset || !dataset->exists()) {
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
    try {
      m_dataType = ioPtr->getStorageObjectDataType(dataset->getPath());
      m_dataTypeInitialized = true;
    } catch (const std::exception& e) {
      std::cerr
          << "VectorIndex::initializeAppendState: error getting data type: "
          << e.what() << std::endl;
      return Status::Failure;
    }
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

Status VectorIndex::appendData(const AQNWB::Types::CellValue& targetValues,
                               size_t& elementsAppended)
{
  if (!m_currentIndexInitialized || !m_dataTypeInitialized) {
    Status initStatus = initializeAppendState();
    if (initStatus != Status::Success) {
      return initStatus;
    }
  }

  auto target = getTargetColumn();
  if (!target) {
    std::cerr << "VectorIndex::appendData: target VectorData is not available."
              << std::endl;
    return Status::Failure;
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
