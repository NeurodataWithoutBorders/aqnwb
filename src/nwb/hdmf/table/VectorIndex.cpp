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

std::shared_ptr<VectorData> VectorIndex::getTargetColumn()
{
  if (!m_targetColumn) {
    m_targetColumn = readTarget();
  }
  return m_targetColumn;
}

std::vector<AQNWB::Types::CellValue> VectorIndex::readIndexedCellValues(
    SizeType start, SizeType count, SizeType stride, SizeType block)
{
  // First, read the indices from this VectorIndex
  SizeArray startArray = {start};
  SizeArray countArray = count > 0 ? SizeArray {count} : SizeArray {};
  SizeArray strideArray = {stride};
  SizeArray blockArray = {block};

  std::vector<AQNWB::Types::CellValue> indices = VectorData::readCellValues(
      startArray, countArray, strideArray, blockArray);

  if (indices.empty()) {
    return {};
  }

  // Get the target column
  auto target = getTargetColumn();
  if (!target) {
    std::cerr
        << "VectorIndex::readCellValues: target VectorData is not available."
        << std::endl;
    return {};
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
      // Extract the value from the CellValue variant
      std::visit(
          [&](auto&& arg)
          {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, AQNWB::Types::ScalarDataVariant>) {
              std::visit(
                  [&](auto&& scalarArg)
                  {
                    using ScalarT = std::decay_t<decltype(scalarArg)>;
                    if constexpr (std::is_integral_v<ScalarT>) {
                      prevIndex = static_cast<uint64_t>(scalarArg);
                    }
                  },
                  arg);
            }
          },
          prevIndices[0].value);
    }
  }

  std::vector<AQNWB::Types::CellValue> result;
  result.reserve(indices.size());

  // For each index, read the corresponding vector from the target
  for (const auto& indexCell : indices) {
    uint64_t currIndex = 0;

    // Extract the value from the CellValue variant
    std::visit(
        [&](auto&& arg)
        {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, AQNWB::Types::ScalarDataVariant>) {
            std::visit(
                [&](auto&& scalarArg)
                {
                  using ScalarT = std::decay_t<decltype(scalarArg)>;
                  if constexpr (std::is_integral_v<ScalarT>) {
                    currIndex = static_cast<uint64_t>(scalarArg);
                  }
                },
                arg);
          }
        },
        indexCell.value);

    // Calculate the number of elements to read
    uint64_t numElements = currIndex - prevIndex;

    if (numElements == 0) {
      // Empty vector
      result.emplace_back(std::vector<uint8_t> {});  // Use a dummy type, it
                                                     // will be empty anyway
    } else {
      // Read the vector from the target
      SizeArray targetStart = {static_cast<SizeType>(prevIndex)};
      SizeArray targetCount = {static_cast<SizeType>(numElements)};

      std::vector<AQNWB::Types::CellValue> targetCells =
          target->readCellValues(targetStart, targetCount);

      // We need to combine the individual cells into a single vector CellValue
      // This is a bit tricky because we need to know the type
      if (!targetCells.empty()) {
        std::visit(
            [&](auto&& arg)
            {
              using T = std::decay_t<decltype(arg)>;
              if constexpr (std::is_same_v<T,
                                           AQNWB::Types::ScalarDataVariant>) {
                std::visit(
                    [&](auto&& scalarArg)
                    {
                      using ScalarT = std::decay_t<decltype(scalarArg)>;
                      if constexpr (!std::is_same_v<ScalarT, std::monostate>) {
                        std::vector<ScalarT> vec;
                        vec.reserve(targetCells.size());

                        for (const auto& cell : targetCells) {
                          std::visit(
                              [&](auto&& cellArg)
                              {
                                using CellT = std::decay_t<decltype(cellArg)>;
                                if constexpr (std::is_same_v<
                                                  CellT,
                                                  AQNWB::Types::
                                                      ScalarDataVariant>)
                                {
                                  std::visit(
                                      [&](auto&& cellScalarArg)
                                      {
                                        using CellScalarT = std::decay_t<
                                            decltype(cellScalarArg)>;
                                        if constexpr (std::is_same_v<
                                                          CellScalarT,
                                                          ScalarT>) {
                                          vec.push_back(cellScalarArg);
                                        }
                                      },
                                      cellArg);
                                }
                              },
                              cell.value);
                        }

                        result.emplace_back(vec);
                      }
                    },
                    arg);
              }
            },
            targetCells[0].value);
      } else {
        result.emplace_back(std::vector<uint8_t> {});
      }
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
