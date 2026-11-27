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
    const hid_t memType = HDF5IO::getNativeNumericTypeId(type);
    if (memType != H5I_INVALID_HID && this->getNumDimensions() > 0) {
      if (writeNumericBlock(dataShape, positionOffset, memType, data)
          != Status::Success)
      {
        return Status::Failure;
      }
    } else {
      // Regular path with the C++ API for array types (typeSize > 1), which
      // have no predefined HDF5 type, and for scalar datasets
      if (writeBlock(dataShape, positionOffset, getMemType(type), data)
          != Status::Success)
      {
        return Status::Failure;
      }
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
    const DataType& nativeType = getMemType(type);
    Status writeStatus = Status::Failure;
    if (type.type == BaseDataType::Type::V_STR) {
      // Handle variable length strings
      std::vector<const char*> cstrBuffer(data.size());
      for (size_t i = 0; i < data.size(); ++i) {
        cstrBuffer[i] = data[i].c_str();
      }
      writeStatus =
          writeBlock(dataShape, positionOffset, nativeType, cstrBuffer.data());
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
      writeStatus =
          writeBlock(dataShape, positionOffset, nativeType, buffer.data());
    } else {
      std::cerr
          << "HDF5RecordingData::writeDataBlock non-string type for string data"
          << std::endl;
      return Status::Failure;
    }
    if (writeStatus != Status::Success) {
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

Status HDF5RecordingData::writeNumericBlock(const SizeArray& dataShape,
                                            const SizeArray& positionOffset,
                                            hid_t memType,
                                            const void* data)
{
  // Same as writeBlock, but with the HDF5 C API, which avoids the overhead
  // of the C++ API (e.g., reference counting of the IDs of H5::DataType and
  // H5::DataSpace objects) for small writes, such as one channel of a block
  std::vector<hsize_t> offset, count;
  if (prepareWrite(dataShape, positionOffset, offset, count) != Status::Success)
  {
    return Status::Failure;
  }
  const int numDimensions = static_cast<int>(count.size());
  const hid_t dataset = m_dataset->getId();
  const hid_t fileSpace = H5Dget_space(dataset);
  if (fileSpace < 0) {
    return Status::Failure;
  }
  herr_t status = H5Sselect_hyperslab(
      fileSpace, H5S_SELECT_SET, offset.data(), nullptr, count.data(), nullptr);
  const hid_t memSpace = H5Screate_simple(numDimensions, count.data(), nullptr);
  if (status >= 0 && memSpace >= 0) {
    status = H5Dwrite(dataset, memType, memSpace, fileSpace, H5P_DEFAULT, data);
  }
  if (memSpace >= 0) {
    H5Sclose(memSpace);
  }
  H5Sclose(fileSpace);
  return (status >= 0 && memSpace >= 0) ? Status::Success : Status::Failure;
}

Status HDF5RecordingData::prepareWrite(const SizeArray& dataShape,
                                       const SizeArray& positionOffset,
                                       std::vector<hsize_t>& offset,
                                       std::vector<hsize_t>& count)
{
  // Check that the dataShape and positionOffset inputs match the dimensions
  // of the dataset
  const SizeType numDimensions = this->getNumDimensions();
  if (dataShape.size() != numDimensions
      || positionOffset.size() != numDimensions)
  {
    return Status::Failure;
  }

  // Ensure that we have enough space to accommodate new data. A scalar
  // dataset has no dimensions: the write replaces its value.
  offset.assign(numDimensions, 0);
  count.assign(numDimensions, 0);
  std::vector<hsize_t> dims(numDimensions);
  bool needsExtend = false;
  for (SizeType i = 0; i < numDimensions; ++i) {
    offset[i] = static_cast<hsize_t>(positionOffset[i]);
    count[i] = dataShape[i] == 0 ? 1 : static_cast<hsize_t>(dataShape[i]);
    const hsize_t end = static_cast<hsize_t>(dataShape[i]) + offset[i];
    dims[i] = std::max(static_cast<hsize_t>(m_shape[i]), end);
    needsExtend = needsExtend || end > m_shape[i];
  }

  // Adjust dataset dimensions if necessary (only possible for chunked
  // datasets, so datasets with a fixed size can be overwritten as well)
  if (needsExtend) {
    if (H5Dset_extent(m_dataset->getId(), dims.data()) < 0) {
      return Status::Failure;
    }
    for (SizeType i = 0; i < numDimensions; ++i) {
      m_shape[i] = static_cast<SizeType>(dims[i]);
    }
  }
  return Status::Success;
}

Status HDF5RecordingData::writeReferences(
    const SizeArray& dataShape,
    const SizeArray& positionOffset,
    const std::vector<std::string>& references)
{
  const hid_t dataset = m_dataset->getId();
  const hid_t fileType = H5Dget_type(dataset);
  const bool isReference =
      fileType >= 0 && H5Tget_class(fileType) == H5T_REFERENCE;
  if (fileType >= 0) {
    H5Tclose(fileType);
  }
  if (!isReference) {
    std::cerr << "HDF5RecordingData::writeReferences: The dataset is not a "
                 "dataset of references."
              << std::endl;
    return Status::Failure;
  }
  // Create the references in the file of the dataset
  std::vector<hobj_ref_t> refs(references.size());
  Status status = Status::Success;
  for (size_t i = 0; i < references.size() && status == Status::Success; ++i) {
    if (H5Rcreate(&refs[i], dataset, references[i].c_str(), H5R_OBJECT, -1) < 0)
    {
      std::cerr << "HDF5RecordingData::writeReferences: Failed to create a "
                   "reference to "
                << references[i] << std::endl;
      status = Status::Failure;
    }
  }
  if (status != Status::Success) {
    return Status::Failure;
  }
  try {
    return writeBlock(
        dataShape, positionOffset, H5::PredType::STD_REF_OBJ, refs.data());
  } catch (const H5::Exception& error) {
    std::cerr << "HDF5RecordingData::writeReferences: " << error.getDetailMsg()
              << std::endl;
    return Status::Failure;
  }
}

const DataType& HDF5RecordingData::getMemType(const BaseDataType& type)
{
  if (!m_memType || !m_memBaseType.has_value()
      || !(m_memBaseType.value() == type))
  {
    m_memType = std::make_unique<DataType>(HDF5IO::getNativeType(type));
    m_memBaseType = type;
  }
  return *m_memType;
}

Status HDF5RecordingData::writeBlock(const SizeArray& dataShape,
                                     const SizeArray& positionOffset,
                                     const DataType& memType,
                                     const void* data)
{
  std::vector<hsize_t> offset, count;
  if (prepareWrite(dataShape, positionOffset, offset, count) != Status::Success)
  {
    return Status::Failure;
  }

  // Initialize the dataspaces directly: assigning an H5::DataSpace to an
  // existing one copies the dataspace (H5Scopy)
  DataSpace fSpace = m_dataset->getSpace();
  DataSpace mSpace = count.empty()
      ? DataSpace(H5S_SCALAR)
      : DataSpace(static_cast<int>(count.size()), count.data());
  if (!count.empty()) {
    fSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());
  }
  m_dataset->write(data, memType, mSpace, fSpace);
  return Status::Success;
}
