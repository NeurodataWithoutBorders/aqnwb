#include "nwb/hdmf/table/VectorData.hpp"

using namespace AQNWB::NWB;

// Register the base VectorData class
REGISTER_SUBCLASS_IMPL(VectorData)

VectorData::VectorData(const std::string& path, std::shared_ptr<IO::BaseIO> io)
    : Data(path, io)
{
}

VectorData::DataSpec::DataSpec(const std::string& datasetName,
                               const IO::ArrayDataSetConfig& dataConfig,
                               const std::string& columnDescription)
    : Data::DataSpec<VectorData>(datasetName, dataConfig)
    , description(columnDescription)
{
}

Status VectorData::DataSpec::initialize(Data& data) const
{
  auto* vectorData = dynamic_cast<VectorData*>(&data);
  if (!vectorData) {
    std::cerr << "VectorData::DataSpec::initialize received incompatible Data "
                 "object"
              << std::endl;
    return Status::Failure;
  }
  return vectorData->initialize(
      static_cast<const IO::ArrayDataSetConfig&>(*this), description);
}

std::shared_ptr<VectorData> VectorData::createReferenceVectorData(
    const std::string& path,
    std::shared_ptr<IO::BaseIO> io,
    const std::string& description,
    const std::vector<std::string>& references)
{
  Status dataStatus = io->createReferenceDataSet(path, references);
  if (dataStatus != Status::Success) {
    return nullptr;
  }

  auto vectorData = VectorData::create(path, io);
  Status commonAttrsStatus = io->createCommonNWBAttributes(
      path, vectorData->getNamespace(), vectorData->getTypeName());
  Status attrStatus = io->createAttribute(description, path, "description");
  if ((attrStatus && commonAttrsStatus) != Status::Success) {
    return nullptr;
  }

  return vectorData;
}

std::shared_ptr<VectorData::DataSpec> VectorData::createDataSpec(
    const std::string& name,
    const IO::ArrayDataSetConfig& dataConfig,
    const std::string& description)
{
  return std::make_shared<DataSpec>(name, dataConfig, description);
}

Status VectorData::initialize(const IO::BaseArrayDataSetConfig& dataConfig,
                              const std::string& description)
{
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr << "IO object has been deleted. Can't initialize VectorData: "
              << m_path << std::endl;
    return Status::Failure;
  }

  Status dataStatus = Data::initialize(dataConfig);
  if (dataConfig.isLink()) {
    const auto* linkConfig =
        dynamic_cast<const IO::LinkArrayDataSetConfig*>(&dataConfig);
    if (linkConfig) {
      return dataStatus
          && linkConfig->validateTarget(*ioPtr, {}, {}, {"description"});
    }
    return dataStatus;
  }

  Status attrStatus =
      ioPtr->createAttribute(description, m_path, "description");
  return dataStatus && attrStatus;
}

Status VectorData::appendData(const AQNWB::Types::CellValue& cellValue,
                              size_t& elementsAppended)
{
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr
        << "IO object has been deleted. Can't append value to VectorData: "
        << m_path << std::endl;
    return Status::Failure;
  }

  auto dataset = recordData();
  if (!dataset) {
    std::cerr << "VectorData::appendValue: dataset is not initialized."
              << std::endl;
    return Status::Failure;
  }

  auto dataType = ioPtr->getStorageObjectDataType(this->getPath());
  SizeArray positionOffset = {0};
  auto currentShape = dataset->getShape();
  if (!currentShape.empty()) {
    positionOffset[0] = currentShape[0];
  }

  // The cellValue contains a variant that can hold either a scalar or a vector.
  // We use std::visit to handle both cases.
  return std::visit(
      [&](auto&& arg) -> Status
      {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          return Status::Failure;
        } else {
          // Check if the value is a scalar
          if (auto* scalarVariant =
                  std::get_if<AQNWB::Types::ScalarDataVariant>(
                      &cellValue.value))
          {
            elementsAppended = 1;
            // Visit the inner scalar variant to get the actual value
            return std::visit(
                [&](auto&& scalarArg) -> Status
                {
                  using ScalarT = std::decay_t<decltype(scalarArg)>;
                  if constexpr (std::is_same_v<ScalarT, std::monostate>) {
                    return Status::Failure;
                  } else {
                    // Strings need to be passed as a vector to writeDataBlock
                    if constexpr (std::is_same_v<ScalarT, std::string>) {
                      std::vector<std::string> strVec = {scalarArg};
                      return dataset->writeDataBlock(
                          SizeArray {1}, positionOffset, dataType, strVec);
                    } else {
                      // Other scalar types can be passed by pointer
                      return dataset->writeDataBlock(
                          SizeArray {1}, positionOffset, dataType, &scalarArg);
                    }
                  }
                },
                *scalarVariant);
          }
          // Check if the value is a vector (for ragged arrays)
          else if (auto* vectorVariant =
                       std::get_if<AQNWB::Types::VectorDataVariant>(
                           &cellValue.value))
          {
            // Visit the inner vector variant to get the actual vector
            return std::visit(
                [&](auto&& vectorArg) -> Status
                {
                  using VectorT = std::decay_t<decltype(vectorArg)>;
                  if constexpr (std::is_same_v<VectorT, std::monostate>) {
                    return Status::Failure;
                  } else {
                    elementsAppended = vectorArg.size();
                    if (elementsAppended == 0) {
                      return Status::Success;  // Nothing to append
                    }
                    // Strings are passed directly as a vector
                    if constexpr (std::is_same_v<VectorT,
                                                 std::vector<std::string>>) {
                      return dataset->writeDataBlock(
                          SizeArray {static_cast<SizeType>(elementsAppended)},
                          positionOffset,
                          dataType,
                          vectorArg);
                    } else {
                      // Other vector types are passed by pointer to their
                      // underlying data
                      return dataset->writeDataBlock(
                          SizeArray {static_cast<SizeType>(elementsAppended)},
                          positionOffset,
                          dataType,
                          vectorArg.data());
                    }
                  }
                },
                *vectorVariant);
          }
          return Status::Failure;
        }
      },
      cellValue.value);
}

Status VectorData::appendBuffer(const IO::BaseDataType::BaseDataVectorVariant& buffer)
{
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr
        << "IO object has been deleted. Can't append buffer to VectorData: "
        << m_path << std::endl;
    return Status::Failure;
  }

  auto dataset = recordData();
  if (!dataset) {
    std::cerr << "VectorData::appendBuffer: dataset is not initialized."
              << std::endl;
    return Status::Failure;
  }

  auto dataType = ioPtr->getStorageObjectDataType(this->getPath());
  SizeArray positionOffset = {0};
  auto currentShape = dataset->getShape();
  if (!currentShape.empty()) {
    positionOffset[0] = currentShape[0];
  }

  return std::visit(
      [&](const auto& vec) -> Status
      {
        using VecType = std::decay_t<decltype(vec)>;
        if constexpr (std::is_same_v<VecType, std::monostate>) {
          return Status::Failure;
        } else {
          if (vec.empty()) {
            return Status::Success;
          }
          SizeArray dataShape = {static_cast<SizeType>(vec.size())};
          if constexpr (std::is_same_v<VecType, std::vector<std::string>>) {
            return dataset->writeDataBlock(
                dataShape, positionOffset, dataType, vec);
          } else {
            return dataset->writeDataBlock(
                dataShape, positionOffset, dataType, vec.data());
          }
        }
      },
      buffer);
}

namespace AQNWB::NWB
{
// Explicitly instantiate the VectorDataTyped template for all common data
// types. This ensures that these specializations are generated by the compiler,
// reducing compile times and ensuring availability throughout the program.
template class VectorDataTyped<std::any>;
template class VectorDataTyped<uint8_t>;
template class VectorDataTyped<uint16_t>;
template class VectorDataTyped<uint32_t>;
template class VectorDataTyped<uint64_t>;
template class VectorDataTyped<int8_t>;
template class VectorDataTyped<int16_t>;
template class VectorDataTyped<int32_t>;
template class VectorDataTyped<int64_t>;
template class VectorDataTyped<float>;
template class VectorDataTyped<double>;
template class VectorDataTyped<std::string>;
}  // namespace AQNWB::NWB
