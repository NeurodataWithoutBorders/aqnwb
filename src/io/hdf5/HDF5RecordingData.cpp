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
  DSetCreatPropList prop = data->getCreatePlist();

  SizeType numDimensions = static_cast<SizeType>(dSpace.getSimpleExtentNdims());
  std::vector<hsize_t> dims(numDimensions), chunk(numDimensions);

  numDimensions =
      static_cast<SizeType>(dSpace.getSimpleExtentDims(dims.data()));

  // Check if the dataset is chunked before trying to get chunk information
  H5D_layout_t layout = prop.getLayout();
  if (layout == H5D_CHUNKED) {
    // Only get chunk information for chunked datasets
    prop.getChunk(static_cast<int>(numDimensions), chunk.data());
  } else {
    // For non-chunked datasets, use the dataset dimensions as the chunk size
    for (SizeType i = 0; i < numDimensions; ++i) {
      chunk[i] = dims[i];
    }
  }

  m_shape = SizeArray(numDimensions);
  for (SizeType i = 0; i < numDimensions; ++i) {
    m_shape[i] = static_cast<SizeType>(dims[i]);
  }
  m_position = SizeArray(
      numDimensions, 0);  // Initialize position with 0 for each dimension
  m_dataset = std::make_unique<H5::DataSet>(*data);
}

HDF5RecordingData::~HDF5RecordingData()
{
  // Safety
  m_dataset->flush(H5F_SCOPE_GLOBAL);
}

Status HDF5RecordingData::writeDataBlock(
    const SizeArray& dataShape,
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

Status HDF5RecordingData::writeDataBlock(
    const SizeArray& dataShape,
    const SizeArray& positionOffset,
    const AQNWB::IO::BaseDataType& type,
    const std::vector<std::string>& data)
{
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
    if (type.type == BaseDataType::Type::V_STR) {
      // Handle variable length strings
      DataType nativeType = StrType(0, H5T_VARIABLE);
      std::vector<const char*> cstrBuffer(data.size());
      for (size_t i = 0; i < data.size(); ++i) {
        cstrBuffer[i] = data[i].c_str();
      }
      // Write the data
      m_dataset->write(cstrBuffer.data(), nativeType, mSpace, fSpace);
    } else if (type.type == BaseDataType::Type::T_STR) {
      // Handle fixed-length strings
      DataType nativeType = HDF5IO::getNativeType(type);
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

Status HDF5RecordingData::writeDataBlockHelper(
    const SizeArray& dataShape,
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

  // Ensure that we have enough space to accommodate new data
  std::vector<hsize_t> dSetDims(numDimensions), offset(numDimensions);
  for (SizeType i = 0; i < numDimensions; ++i) {
    offset[i] = static_cast<hsize_t>(positionOffset[i]);

    if (dataShape[i] + offset[i] > m_shape[i]) {
      dSetDims[i] = dataShape[i] + offset[i];
    } else {
      dSetDims[i] = m_shape[i];
    }
  }

  // Adjust dataset dimensions if necessary
  m_dataset->extend(dSetDims.data());

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
