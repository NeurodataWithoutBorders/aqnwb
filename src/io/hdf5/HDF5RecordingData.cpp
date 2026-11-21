#include <algorithm>
#include <codecvt>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include "io/hdf5/HDF5RecordingData.hpp"

#include <H5Cpp.h>
#include <H5Fpublic.h>

#include "Utils.hpp"

using namespace H5;
using namespace AQNWB::IO::HDF5;

namespace
{
/// The HDF5 native type of a numeric BaseDataType with typeSize 1, or
/// H5I_INVALID_HID for other types (strings, array types)
hid_t nativeTypeId(const AQNWB::IO::BaseDataType& type)
{
  using AQNWB::IO::BaseDataType;
  if (type.typeSize != 1) {
    return H5I_INVALID_HID;  // array types: use the general path
  }
  switch (type.type) {
    case BaseDataType::Type::T_I8:
      return H5T_NATIVE_INT8;
    case BaseDataType::Type::T_I16:
      return H5T_NATIVE_INT16;
    case BaseDataType::Type::T_I32:
      return H5T_NATIVE_INT32;
    case BaseDataType::Type::T_I64:
      return H5T_NATIVE_INT64;
    case BaseDataType::Type::T_U8:
      return H5T_NATIVE_UINT8;
    case BaseDataType::Type::T_U16:
      return H5T_NATIVE_UINT16;
    case BaseDataType::Type::T_U32:
      return H5T_NATIVE_UINT32;
    case BaseDataType::Type::T_U64:
      return H5T_NATIVE_UINT64;
    case BaseDataType::Type::T_F32:
      return H5T_NATIVE_FLOAT;
    case BaseDataType::Type::T_F64:
      return H5T_NATIVE_DOUBLE;
    default:
      return H5I_INVALID_HID;
  }
}
}  // namespace

HDF5RecordingData::HDF5RecordingData(std::unique_ptr<H5::DataSet> data)
{
  DataSpace dSpace = data->getSpace();

  SizeType numDimensions = static_cast<SizeType>(dSpace.getSimpleExtentNdims());
  std::vector<hsize_t> dims(numDimensions);

  numDimensions =
      static_cast<SizeType>(dSpace.getSimpleExtentDims(dims.data()));

  m_shape = SizeArray(numDimensions);
  for (SizeType i = 0; i < numDimensions; ++i) {
    m_shape[i] = static_cast<SizeType>(dims[i]);
  }
  m_position = SizeArray(numDimensions,
                         0);  // Initialize position with 0 for each dimension
  m_dataset = std::make_unique<H5::DataSet>(*data);
}

HDF5RecordingData::~HDF5RecordingData()
{
  // Safety
  m_dataset->flush(H5F_SCOPE_GLOBAL);
}

Status HDF5RecordingData::writeDataBlock(const SizeArray& dataShape,
                                         const SizeArray& positionOffset,
                                         const BaseDataType& type,
                                         const void* data)
{
  try {
    // check type. Strings should use the other variant of this function
    if (type.type == BaseDataType::Type::V_STR
        || type.type == BaseDataType::Type::T_STR)
    {
      std::cerr << "HDF5RecordingData::writeDataBlock called for string data, "
                   "use HDF5RecordingData::writeDataBlock with a string array "
                   "data input instead of void* data."
                << std::endl;
      return Status::Failure;
    }

    // Fast path for numeric data in datasets with at least one dimension
    const hid_t memType = nativeTypeId(type);
    if (memType != H5I_INVALID_HID && this->getNumDimensions() > 0) {
      if (writeNumericBlock(dataShape, positionOffset, memType, data)
          != Status::Success)
      {
        return Status::Failure;
      }
    } else {
      // validate and allocate space
      DataSpace mSpace;
      DataSpace fSpace;
      Status setupStatus = writeDataBlockHelper(dataShape,
                                                positionOffset,
                                                mSpace,  // output
                                                fSpace  // output
      );
      if (setupStatus == Status::Failure) {
        return Status::Failure;
      }

      // Write the data
      DataType nativeType = HDF5IO::getNativeType(type);
      m_dataset->write(data, nativeType, mSpace, fSpace);
    }

    // Update position for simple extension
    for (SizeType i = 0; i < dataShape.size(); ++i) {
      m_position[i] += dataShape[i];
    }
  } catch (DataSetIException& error) {
    error.printErrorStack();
    return Status::Failure;
  } catch (DataSpaceIException& error) {
    error.printErrorStack();
    return Status::Failure;
  } catch (FileIException& error) {
    error.printErrorStack();
    return Status::Failure;
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return Status::Failure;
  }
  return Status::Success;
}

Status HDF5RecordingData::writeDataBlock(const SizeArray& dataShape,
                                         const SizeArray& positionOffset,
                                         const AQNWB::IO::BaseDataType& type,
                                         const std::vector<std::string>& data)
{
  if (getNumDimensions() == 0 && data.size() != 1) {
    std::cerr << "HDF5RecordingData::writeDataBlock: a scalar dataset takes "
                 "exactly one string"
              << std::endl;
    return Status::Failure;
  }
  try {
    // validate and allocate space
    DataSpace mSpace;
    DataSpace fSpace;
    Status setupStatus = writeDataBlockHelper(dataShape,
                                              positionOffset,
                                              mSpace,  // output
                                              fSpace  // output
    );
    if (setupStatus == Status::Failure) {
      return Status::Failure;
    }
    // Write the data
    // Write the data
    DataType nativeType = HDF5IO::getNativeType(type);
    if (type.type == BaseDataType::Type::V_STR) {
      // Handle variable length strings
      std::vector<const char*> cstrBuffer(data.size());
      for (size_t i = 0; i < data.size(); ++i) {
        cstrBuffer[i] = data[i].c_str();
      }
      // Write the data
      m_dataset->write(cstrBuffer.data(), nativeType, mSpace, fSpace);
    } else if (type.type == BaseDataType::Type::T_STR) {
      // Handle fixed-length strings
      std::vector<char> buffer(data.size() * type.typeSize, '\0');
      size_t bufferIndex = 0;
      for (const auto& str : data) {
        if (str.size() > type.typeSize) {
          return Status::Failure;  // String length exceeds the fixed length
        }
        std::memcpy(&buffer[bufferIndex], str.c_str(), str.size());
        bufferIndex += type.typeSize;
      }
      m_dataset->write(buffer.data(), nativeType, mSpace, fSpace);
    } else {
      std::cerr
          << "HDF5RecordingData::writeDataBlock non-string type for string data"
          << std::endl;
      return Status::Failure;
    }

    // Update position for simple extension
    for (SizeType i = 0; i < dataShape.size(); ++i) {
      m_position[i] += dataShape[i];
    }
  } catch (DataSetIException& error) {
    error.printErrorStack();
    return Status::Failure;
  } catch (DataSpaceIException& error) {
    error.printErrorStack();
    return Status::Failure;
  } catch (FileIException& error) {
    error.printErrorStack();
    return Status::Failure;
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return Status::Failure;
  }
  return Status::Success;
}


static_assert(sizeof(hid_t) == sizeof(int64_t), "hid_t is passed as int64_t");

Status HDF5RecordingData::writeNumericBlock(const SizeArray& dataShape,
                                            const SizeArray& positionOffset,
                                            int64_t memType,
                                            const void* data)
{
  // Same semantics as writeDataBlockHelper + DataSet::write, but with the HDF5
  // C API: the C++ API copies the dataspaces and the data type (H5Scopy,
  // H5Tcopy) on assignment, which dominated the cost of small writes (e.g.,
  // one row of a DynamicTable or one channel of a block).
  const SizeType numDimensions = this->getNumDimensions();
  if (dataShape.size() != numDimensions
      || positionOffset.size() != numDimensions)
  {
    return Status::Failure;
  }

  std::vector<hsize_t> dims(numDimensions), offset(numDimensions),
      count(numDimensions);
  bool needsExtend = false;
  for (SizeType i = 0; i < numDimensions; ++i) {
    offset[i] = static_cast<hsize_t>(positionOffset[i]);
    count[i] = dataShape[i] == 0 ? 1 : static_cast<hsize_t>(dataShape[i]);
    const hsize_t end = static_cast<hsize_t>(dataShape[i]) + offset[i];
    dims[i] = std::max(static_cast<hsize_t>(m_shape[i]), end);
    needsExtend = needsExtend || end > m_shape[i];
  }

  const hid_t dataset = m_dataset->getId();
  if (needsExtend) {
    if (H5Dset_extent(dataset, dims.data()) < 0) {
      return Status::Failure;
    }
    for (SizeType i = 0; i < numDimensions; ++i) {
      m_shape[i] = static_cast<SizeType>(dims[i]);
    }
  }

  const hid_t fileSpace = H5Dget_space(dataset);
  if (fileSpace < 0) {
    return Status::Failure;
  }
  herr_t status = H5Sselect_hyperslab(
      fileSpace, H5S_SELECT_SET, offset.data(), nullptr, count.data(), nullptr);
  const hid_t memSpace =
      H5Screate_simple(static_cast<int>(numDimensions), count.data(), nullptr);
  if (status >= 0 && memSpace >= 0) {
    status = H5Dwrite(dataset, memType, memSpace, fileSpace, H5P_DEFAULT, data);
  }
  if (memSpace >= 0) {
    H5Sclose(memSpace);
  }
  H5Sclose(fileSpace);
  return (status >= 0 && memSpace >= 0) ? Status::Success : Status::Failure;
}

Status HDF5RecordingData::writeDataBlockHelper(const SizeArray& dataShape,
                                               const SizeArray& positionOffset,
                                               DataSpace& mSpace,
                                               DataSpace& fSpace)
{
  // Check that the dataShape and positionOffset inputs match the dimensions
  // of the dataset
  SizeType numDimensions = this->getNumDimensions();
  if (dataShape.size() != numDimensions
      || positionOffset.size() != numDimensions)
  {
    return Status::Failure;
  }

  // Scalar dataset: the write replaces the value of the dataset
  if (numDimensions == 0) {
    mSpace = DataSpace(H5S_SCALAR);
    fSpace = m_dataset->getSpace();
    return Status::Success;
  }

  // Ensure that we have enough space to accommodate new data
  std::vector<hsize_t> dSetDims(numDimensions), offset(numDimensions);
  bool needsExtend = false;
  for (SizeType i = 0; i < numDimensions; ++i) {
    offset[i] = static_cast<hsize_t>(positionOffset[i]);

    if (dataShape[i] + offset[i] > m_shape[i]) {
      dSetDims[i] = dataShape[i] + offset[i];
      needsExtend = true;
    } else {
      dSetDims[i] = m_shape[i];
    }
  }

  // Adjust dataset dimensions if necessary (only possible for chunked
  // datasets, so datasets with a fixed size can be overwritten as well)
  if (needsExtend) {
    m_dataset->extend(dSetDims.data());
  }

  // Set the size to the new size based on the updated dimensionality
  fSpace = m_dataset->getSpace();
  fSpace.getSimpleExtentDims(dSetDims.data());
  for (SizeType i = 0; i < numDimensions; ++i) {
    m_shape[i] = dSetDims[i];
  }

  // Create memory space with the shape of the data
  std::vector<hsize_t> dataDims(numDimensions);
  for (SizeType i = 0; i < numDimensions; ++i) {
    dataDims[i] = dataShape[i] == 0 ? 1 : static_cast<hsize_t>(dataShape[i]);
  }
  mSpace = DataSpace(static_cast<int>(numDimensions), dataDims.data());

  // Select hyperslab in the file space
  fSpace.selectHyperslab(H5S_SELECT_SET, dataDims.data(), offset.data());

  return Status::Success;
}
