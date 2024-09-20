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

  int nDimensions = dSpace.getSimpleExtentNdims();
  std::vector<hsize_t> dims(nDimensions), chunk(nDimensions);

  nDimensions = dSpace.getSimpleExtentDims(
      dims.data());  // TODO -redefine here or use original?
  prop.getChunk(static_cast<int>(nDimensions), chunk.data());

  this->size = std::vector<SizeType>(nDimensions);
  for (int i = 0; i < nDimensions; ++i) {
    this->size[i] = dims[i];
  }
  this->nDimensions = nDimensions;
  this->position = std::vector<SizeType>(
      nDimensions, 0);  // Initialize position with 0 for each dimension
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
    // check dataShape and positionOffset inputs match the dimensions of the
    // dataset
    if (dataShape.size() != nDimensions || positionOffset.size() != nDimensions)
    {
      return Status::Failure;
    }

    // Ensure that we have enough space to accommodate new data
    std::vector<hsize_t> dSetDims(nDimensions), offset(nDimensions);
    for (int i = 0; i < nDimensions; ++i) {
      offset[i] = static_cast<hsize_t>(positionOffset[i]);

      if (dataShape[i] + offset[i] > size[i])  // TODO - do I need offset here
        dSetDims[i] = dataShape[i] + offset[i];
      else
        dSetDims[i] = size[i];
    }

    // Adjust dataset dimensions if necessary
    m_dataset->extend(dSetDims.data());

    // Set size to new size based on updated dimensionality
    DataSpace fSpace = m_dataset->getSpace();
    fSpace.getSimpleExtentDims(dSetDims.data());
    for (int i = 0; i < nDimensions; ++i) {
      size[i] = dSetDims[i];
    }

    // Create memory space with the shape of the data
    // DataSpace mSpace(dimension, dSetDim.data());
    std::vector<hsize_t> dataDims(nDimensions);
    for (int i = 0; i < nDimensions; ++i) {
      if (dataShape[i] == 0) {
        dataDims[i] = 1;
      } else {
        dataDims[i] = static_cast<hsize_t>(dataShape[i]);
      }
    }
    DataSpace mSpace(static_cast<int>(nDimensions), dataDims.data());

    // Select hyperslab in the file space
    fSpace.selectHyperslab(H5S_SELECT_SET, dataDims.data(), offset.data());

    // Write the data
    DataType nativeType = HDF5IO::getNativeType(type);
    m_dataset->write(data, nativeType, mSpace, fSpace);

    // Update position for simple extension
    for (int i = 0; i < dataShape.size(); ++i) {
      position[i] += dataShape[i];
    }
  } catch (DataSetIException error) {
    error.printErrorStack();
  } catch (DataSpaceIException error) {
    error.printErrorStack();
  } catch (FileIException error) {
    error.printErrorStack();
  }
  return Status::Success;
}
