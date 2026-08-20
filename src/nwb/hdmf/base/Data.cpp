#include <numeric>

#include "nwb/hdmf/base/Data.hpp"

#include "io/RecordingObjects.hpp"

using namespace AQNWB::NWB;

// Register the base Data class
REGISTER_SUBCLASS_IMPL(Data)

/** Constructor */
Data::Data(const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
    : RegisteredType(path, io)
{
}

Status Data::initialize(const IO::BaseArrayDataSetConfig& dataConfig)
{
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr << "IO object has been deleted. Can't initialize Data: " << m_path
              << std::endl;
    return Status::Failure;
  }

  // Create the dataset or link
  try {
    auto dataset = ioPtr->createArrayDataSet(dataConfig, this->m_path);
    // Note: dataset may be nullptr for links; this is not an error.
  } catch (const std::runtime_error& e) {
    std::cerr << "Data::initialize: Failed to create dataset: " << e.what()
              << std::endl;
    return Status::Failure;
  }

  if (dataConfig.isLink()) {
    // For links, don't set attributes since we don't own the dataset.
    // Validate that the link target has the common NWB attributes.
    const auto* linkConfig =
        dynamic_cast<const IO::LinkArrayDataSetConfig*>(&dataConfig);
    if (linkConfig) {
      return linkConfig->validateTarget(
          *ioPtr, {}, {}, {"namespace", "object_id", "neurodata_type"});
    }
  } else {
    // setup common attributes
    Status commonAttrsStatus = ioPtr->createCommonNWBAttributes(
        m_path, this->getNamespace(), this->getTypeName());
    return commonAttrsStatus;
  }
  return Status::Success;
}

std::vector<AQNWB::Types::CellValue> Data::readCellValues(
    const SizeArray& start,
    const SizeArray& count,
    const SizeArray& stride,
    const SizeArray& block)
{
  std::vector<AQNWB::Types::CellValue> result;

  // Read the data block
  auto dataBlock = readData()->valuesGeneric(start, count, stride, block);

  // Get the number of elements read
  size_t numElements = std::accumulate(dataBlock.shape.begin(),
                                       dataBlock.shape.end(),
                                       1ULL,
                                       std::multiplies<size_t>());

  if (numElements == 0) {
    return result;
  }

  result.reserve(numElements);

  // Convert the data block to CellValues based on the data type
  switch (dataBlock.baseDataType.type) {
    case IO::BaseDataType::T_U8: {
      auto data = IO::DataBlock<uint8_t>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_U16: {
      auto data = IO::DataBlock<uint16_t>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_U32: {
      auto data = IO::DataBlock<uint32_t>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_U64: {
      auto data = IO::DataBlock<uint64_t>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_I8: {
      auto data = IO::DataBlock<int8_t>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_I16: {
      auto data = IO::DataBlock<int16_t>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_I32: {
      auto data = IO::DataBlock<int32_t>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_I64: {
      auto data = IO::DataBlock<int64_t>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_F32: {
      auto data = IO::DataBlock<float>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_F64: {
      auto data = IO::DataBlock<double>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    case IO::BaseDataType::T_STR:
    case IO::BaseDataType::V_STR: {
      auto data = IO::DataBlock<std::string>::fromGeneric(dataBlock);
      for (size_t i = 0; i < numElements; ++i) {
        result.emplace_back(data.data[i]);
      }
      break;
    }
    default:
      throw std::runtime_error("Unsupported data type in Data::readCellValues");
  }

  return result;
}

namespace AQNWB::NWB
{

template<typename DTYPE>
std::shared_ptr<DataTyped<DTYPE>> DataTyped<DTYPE>::create(
    const std::string& path, std::shared_ptr<AQNWB::IO::BaseIO> io)
{
  if (io) {
    auto recordingObjects = io->getRecordingObjects();
    if (recordingObjects) {
      auto existingObj = recordingObjects->getRecordingObject(path);
      if (existingObj) {
        auto typedObj =
            std::dynamic_pointer_cast<DataTyped<DTYPE>>(existingObj);
        if (typedObj) {
          return typedObj;
        }
        // If an object with the same path exists but is not of this exact type
        // (e.g., it's a base Data), we create a new DataTyped
        // instance but do NOT add it to the recording objects cache to avoid
        // overwriting the existing Data object or causing conflicts.
        return std::shared_ptr<DataTyped<DTYPE>>(
            new DataTyped<DTYPE>(path, io));
      }
    }
  }
  return RegisteredType::create<DataTyped<DTYPE>>(path, io);
}

// Explicitly instantiate the DataTyped template for all common data types.
// This ensures that these specializations are generated by the compiler,
// reducing compile times and ensuring availability throughout the program.
template class DataTyped<std::any>;
template class DataTyped<uint8_t>;
template class DataTyped<uint16_t>;
template class DataTyped<uint32_t>;
template class DataTyped<uint64_t>;
template class DataTyped<int8_t>;
template class DataTyped<int16_t>;
template class DataTyped<int32_t>;
template class DataTyped<int64_t>;
template class DataTyped<float>;
template class DataTyped<double>;
template class DataTyped<std::string>;
}  // namespace AQNWB::NWB
