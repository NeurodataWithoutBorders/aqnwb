#include <codecvt>
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

  this->nDimensions = static_cast<SizeType>(dSpace.getSimpleExtentNdims());
  std::vector<hsize_t> dims(this->nDimensions), chunk(this->nDimensions);

  this->nDimensions =
      static_cast<SizeType>(dSpace.getSimpleExtentDims(dims.data()));
  prop.getChunk(static_cast<int>(this->nDimensions), chunk.data());

  this->size = std::vector<SizeType>(this->nDimensions);
  for (SizeType i = 0; i < this->nDimensions; ++i) {
    this->size[i] = static_cast<SizeType>(dims[i]);
  }
  this->position = std::vector<SizeType>(
      this->nDimensions, 0);  // Initialize position with 0 for each dimension
  m_dataset = std::make_unique<H5::DataSet>(*data);
}

HDF5RecordingData::~HDF5RecordingData()
{
  // Safety
  m_dataset->flush(H5F_SCOPE_GLOBAL);
}

Status HDF5RecordingData::writeDataBlock(
    const std::vector<SizeType>& dataShape,
    const std::vector<SizeType>& positionOffset,
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
      position[i] += dataShape[i];
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
    const std::vector<SizeType>& dataShape,
    const std::vector<SizeType>& positionOffset,
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
      position[i] += dataShape[i];
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
    const std::vector<SizeType>& dataShape,
    const std::vector<SizeType>& positionOffset,
    DataSpace& mSpace,
    DataSpace& fSpace)
{
  // Check that the dataShape and positionOffset inputs match the dimensions
  // of the dataset
  if (dataShape.size() != nDimensions || positionOffset.size() != nDimensions) {
    return Status::Failure;
  }

  // Ensure that we have enough space to accommodate new data
  std::vector<hsize_t> dSetDims(nDimensions), offset(nDimensions);
  for (SizeType i = 0; i < nDimensions; ++i) {
    offset[i] = static_cast<hsize_t>(positionOffset[i]);

    if (dataShape[i] + offset[i] > size[i]) {
      dSetDims[i] = dataShape[i] + offset[i];
    } else {
      dSetDims[i] = size[i];
    }
  }

  // Adjust dataset dimensions if necessary
  m_dataset->extend(dSetDims.data());

  // Set the size to the new size based on the updated dimensionality
  fSpace = m_dataset->getSpace();
  fSpace.getSimpleExtentDims(dSetDims.data());
  for (SizeType i = 0; i < nDimensions; ++i) {
    size[i] = dSetDims[i];
  }

  // Create memory space with the shape of the data
  std::vector<hsize_t> dataDims(nDimensions);
  for (SizeType i = 0; i < nDimensions; ++i) {
    dataDims[i] = dataShape[i] == 0 ? 1 : static_cast<hsize_t>(dataShape[i]);
  }
  mSpace = DataSpace(static_cast<int>(nDimensions), dataDims.data());

  // Select hyperslab in the file space
  fSpace.selectHyperslab(H5S_SELECT_SET, dataDims.data(), offset.data());

  return Status::Success;
}
