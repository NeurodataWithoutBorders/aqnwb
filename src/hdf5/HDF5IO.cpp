#include <codecvt>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include "hdf5/HDF5IO.hpp"

#include <H5Cpp.h>
#include <H5Fpublic.h>

#include "Utils.hpp"

using namespace H5;
using namespace AQNWB::HDF5;

// HDF5IO
HDF5IO::HDF5IO(const std::string& filename, const bool disableSWMRMode)
    : BaseIO(filename)
    , m_disableSWMRMode(disableSWMRMode)
{
}

HDF5IO::~HDF5IO()
{
  close();
}

std::string HDF5IO::getFileName()
{
  return this->m_filename;
}

Status HDF5IO::open()
{
  if (std::filesystem::exists(getFileName())) {
    return open(false);
  } else {
    return open(true);
  }
}

Status HDF5IO::open(bool newfile)
{
  int accFlags = 0;

  if (this->m_opened)
    return Status::Failure;

  FileAccPropList fapl = FileAccPropList::DEFAULT;
  H5Pset_libver_bounds(fapl.getId(), H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);

  if (newfile)
    accFlags = H5F_ACC_TRUNC;
  else
    accFlags = H5F_ACC_RDWR;

  this->m_file = std::make_unique<H5::H5File>(
      getFileName(), accFlags, FileCreatPropList::DEFAULT, fapl);
  this->m_opened = true;

  return Status::Success;
}

Status HDF5IO::close()
{
  if (this->m_file != nullptr && this->m_opened) {
    this->m_file->close();
    this->m_file = nullptr;
    this->m_opened = false;
  }

  return Status::Success;
}

Status checkStatus(int status)
{
  if (status < 0)
    return Status::Failure;
  else
    return Status::Success;
}

Status HDF5IO::flush()
{
  int status = H5Fflush(this->m_file->getId(), H5F_SCOPE_GLOBAL);
  return checkStatus(status);
}

Status HDF5IO::createAttribute(const BaseDataType& type,
                               const void* data,
                               const std::string& path,
                               const std::string& name,
                               const SizeType& size)
{
  H5Object* loc;
  Group gloc;
  DataSet dloc;
  Attribute attr;
  DataType H5type;
  DataType origType;

  if (!this->m_opened)
    return Status::Failure;

  // open the group or dataset
  H5O_type_t objectType = getObjectType(path);
  switch (objectType) {
    case H5O_TYPE_GROUP:
      gloc = this->m_file->openGroup(path);
      loc = &gloc;
      break;
    case H5O_TYPE_DATASET:
      dloc = this->m_file->openDataSet(path);
      loc = &dloc;
      break;
    default:
      return Status::Failure;  // not a valid dataset or group type
  }

  H5type = getH5Type(type);
  origType = getNativeType(type);

  if (size > 1) {
    hsize_t dims = static_cast<hsize_t>(size);
    H5type = ArrayType(H5type, 1, &dims);
    origType = ArrayType(origType, 1, &dims);
  }

  if (loc->attrExists(name)) {
    attr = loc->openAttribute(name);
  } else {
    DataSpace attr_dataspace(H5S_SCALAR);
    attr = loc->createAttribute(name, H5type, attr_dataspace);
  }

  attr.write(origType, data);

  return Status::Success;
}

Status HDF5IO::createAttribute(const std::string& data,
                               const std::string& path,
                               const std::string& name)
{
  std::vector<const char*> dataPtrs;
  dataPtrs.push_back(data.c_str());

  return createAttribute(dataPtrs, path, name, data.length());
}

Status HDF5IO::createAttribute(const std::vector<std::string>& data,
                               const std::string& path,
                               const std::string& name)
{
  std::vector<const char*> dataPtrs;
  SizeType maxLength = 0;
  for (const std::string& str : data) {
    SizeType length = str.length();
    maxLength = std::max(maxLength, length);
    dataPtrs.push_back(str.c_str());
  }

  return createAttribute(dataPtrs, path, name, maxLength + 1);
}

Status HDF5IO::createAttribute(const std::vector<const char*>& data,
                               const std::string& path,
                               const std::string& name,
                               const SizeType& maxSize)
{
  H5Object* loc;
  Group gloc;
  DataSet dloc;
  Attribute attr;
  hsize_t dims[1];

  if (!this->m_opened)
    return Status::Failure;

  StrType H5type(PredType::C_S1, maxSize);
  H5type.setSize(H5T_VARIABLE);

  // open the group or dataset
  H5O_type_t objectType = getObjectType(path);
  switch (objectType) {
    case H5O_TYPE_GROUP:
      gloc = this->m_file->openGroup(path);
      loc = &gloc;
      break;
    case H5O_TYPE_DATASET:
      dloc = this->m_file->openDataSet(path);
      loc = &dloc;
      break;
    default:
      return Status::Failure;  // not a valid dataset or group type
  }

  try {
    if (loc->attrExists(name)) {
      return Status::Failure;  // don't allow overwriting because string
                               // attributes cannot change size easily
    } else {
      DataSpace attr_dataspace;
      SizeType nStrings = data.size();
      if (nStrings > 1) {
        dims[0] = nStrings;
        attr_dataspace = DataSpace(1, dims);
      } else
        attr_dataspace = DataSpace(H5S_SCALAR);
      attr = loc->createAttribute(name, H5type, attr_dataspace);
    }
    attr.write(H5type, data.data());
  } catch (GroupIException error) {
    error.printErrorStack();
  } catch (AttributeIException error) {
    error.printErrorStack();
  } catch (FileIException error) {
    error.printErrorStack();
  } catch (DataSetIException error) {
    error.printErrorStack();
  }
  return Status::Success;
}

Status HDF5IO::createReferenceAttribute(const std::string& referencePath,
                                        const std::string& path,
                                        const std::string& name)
{
  H5Object* loc;
  Group gloc;
  DataSet dloc;
  Attribute attr;

  if (!this->m_opened)
    return Status::Failure;

  // open the group or dataset
  H5O_type_t objectType = getObjectType(path);
  switch (objectType) {
    case H5O_TYPE_GROUP:
      gloc = this->m_file->openGroup(path);
      loc = &gloc;
      break;
    case H5O_TYPE_DATASET:
      dloc = this->m_file->openDataSet(path);
      loc = &dloc;
      break;
    default:
      return Status::Failure;  // not a valid dataset or group type
  }

  try {
    if (loc->attrExists(name)) {
      attr = loc->openAttribute(name);
    } else {
      DataSpace attr_space(H5S_SCALAR);
      attr = loc->createAttribute(name, H5::PredType::STD_REF_OBJ, attr_space);
    }

    hobj_ref_t* rdata = new hobj_ref_t[sizeof(hobj_ref_t)];

    this->m_file->reference(rdata, referencePath.c_str());

    attr.write(H5::PredType::STD_REF_OBJ, rdata);
    delete[] rdata;

  } catch (GroupIException error) {
    error.printErrorStack();
  } catch (AttributeIException error) {
    error.printErrorStack();
  } catch (FileIException error) {
    error.printErrorStack();
  } catch (DataSetIException error) {
    error.printErrorStack();
  }

  return Status::Success;
}

Status HDF5IO::createGroup(const std::string& path)
{
  if (!this->m_opened)
    return Status::Failure;
  try {
    this->m_file->createGroup(path);
  } catch (FileIException error) {
    error.printErrorStack();
  } catch (GroupIException error) {
    error.printErrorStack();
  }
  return Status::Success;
}

Status HDF5IO::createGroupIfDoesNotExist(const std::string& path)
{
  if (!this->m_opened)
    return Status::Failure;
  try {
    this->m_file->childObjType(path);
  } catch (FileIException) {
    return createGroup(path);
  }
  return Status::Success;
}

/** Creates a link to another location in the file */
Status HDF5IO::createLink(const std::string& path, const std::string& reference)
{
  if (!this->m_opened)
    return Status::Failure;

  herr_t error = H5Lcreate_soft(reference.c_str(),
                                this->m_file->getLocId(),
                                path.c_str(),
                                H5P_DEFAULT,
                                H5P_DEFAULT);

  return checkStatus(error);
}

Status HDF5IO::createReferenceDataSet(
    const std::string& path, const std::vector<std::string>& references)
{
  if (!this->m_opened)
    return Status::Failure;

  const hsize_t size = references.size();

  hobj_ref_t* rdata = new hobj_ref_t[size * sizeof(hobj_ref_t)];

  for (SizeType i = 0; i < size; i++) {
    this->m_file->reference(&rdata[i], references[i].c_str());
  }

  hid_t space = H5Screate_simple(1, &size, NULL);

  hid_t dset = H5Dcreate(this->m_file->getLocId(),
                         path.c_str(),
                         H5T_STD_REF_OBJ,
                         space,
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);

  herr_t writeStatus =
      H5Dwrite(dset, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);

  delete[] rdata;

  herr_t dsetStatus = H5Dclose(dset);
  herr_t spaceStatus = H5Sclose(space);

  return checkStatus(writeStatus);
}

Status HDF5IO::createStringDataSet(const std::string& path,
                                   const std::string& value)
{
  if (!this->m_opened)
    return Status::Failure;

  std::unique_ptr<H5::DataSet> dataset;
  DataType H5type = getH5Type(BaseDataType::STR(value.length()));
  DataSpace dSpace(H5S_SCALAR);

  dataset = std::make_unique<H5::DataSet>(
      this->m_file->createDataSet(path, H5type, dSpace));
  dataset->write(value.c_str(), H5type);

  return Status::Success;
}

Status HDF5IO::createStringDataSet(const std::string& path,
                                   const std::vector<std::string>& values)
{
  if (!this->m_opened)
    return Status::Failure;

  std::vector<const char*> cStrs;
  cStrs.reserve(values.size());
  for (const auto& str : values) {
    cStrs.push_back(str.c_str());
  }

  std::unique_ptr<BaseRecordingData> dataset;
  dataset = std::unique_ptr<BaseRecordingData>(createArrayDataSet(
      BaseDataType::V_STR, SizeArray {values.size()}, SizeArray {1}, path));
  dataset->writeDataBlock(
      std::vector<SizeType>(1, 1), BaseDataType::V_STR, cStrs.data());

  return Status::Success;
}

Status HDF5IO::startRecording()
{
  if (!this->m_opened)
    return Status::Failure;

  if (!this->m_disableSWMRMode) {
    herr_t status = H5Fstart_swmr_write(this->m_file->getId());
    return checkStatus(status);
  }
  return Status::Success;
}

Status HDF5IO::stopRecording()
{
  // if SWMR mode is disabled, stopping the recording will leave the file open
  if (!this->m_disableSWMRMode) {
    close();  // SWMR mode cannot be disabled so close the file
  } else {
    this->flush();
  }
  return Status::Success;
}

bool HDF5IO::canModifyObjects()
{
  if (!this->m_opened)
    return false;

  // Check if we are in SWMR mode
  bool inSWMRMode = false;
  unsigned int intent;
  herr_t status = H5Fget_intent(this->m_file->getId(), &intent);
  bool statusOK = (status >= 0);
  if (statusOK) {
    inSWMRMode = (intent & (H5F_ACC_SWMR_READ | H5F_ACC_SWMR_WRITE));
  }

  // if the file is opened and we are not in swmr mode then we can modify
  // objects
  return statusOK && !inSWMRMode;
}

bool HDF5IO::objectExists(const std::string& path)
{
  htri_t exists = H5Lexists(this->m_file->getId(), path.c_str(), H5P_DEFAULT);
  if (exists > 0) {
    return true;
  } else {
    return false;
  }
}

std::unique_ptr<AQNWB::BaseRecordingData> HDF5IO::getDataSet(
    const std::string& path)
{
  std::unique_ptr<DataSet> data;

  if (!this->m_opened)
    return nullptr;

  try {
    data = std::make_unique<H5::DataSet>(this->m_file->openDataSet(path));
    return std::make_unique<HDF5RecordingData>(std::move(data));
  } catch (DataSetIException error) {
    error.printErrorStack();
    return nullptr;
  } catch (FileIException error) {
    error.printErrorStack();
    return nullptr;
  } catch (DataSpaceIException error) {
    error.printErrorStack();
    return nullptr;
  }
}

std::unique_ptr<AQNWB::BaseRecordingData> HDF5IO::createArrayDataSet(
    const BaseDataType& type,
    const SizeArray& size,
    const SizeArray& chunking,
    const std::string& path)
{
  std::unique_ptr<DataSet> data;
  DSetCreatPropList prop;
  DataType H5type = getH5Type(type);

  if (!this->m_opened)
    return nullptr;

  SizeType dimension = size.size();
  if (dimension < 1)  // Check for at least one dimension
    return nullptr;

  // Ensure chunking is properly allocated and has at least 'dimension' elements
  assert(chunking.size() >= dimension);

  // Use vectors to support an arbitrary number of dimensions
  std::vector<hsize_t> dims(dimension), chunk_dims(dimension),
      max_dims(dimension);

  for (SizeType i = 0; i < dimension; i++) {
    dims[i] = static_cast<hsize_t>(size[i]);
    if (chunking[i] > 0) {
      chunk_dims[i] = static_cast<hsize_t>(chunking[i]);
      max_dims[i] = H5S_UNLIMITED;
    } else {
      chunk_dims[i] = static_cast<hsize_t>(size[i]);
      max_dims[i] = static_cast<hsize_t>(size[i]);
    }
  }

  DataSpace dSpace(static_cast<int>(dimension), dims.data(), max_dims.data());
  prop.setChunk(static_cast<int>(dimension), chunk_dims.data());

  data = std::make_unique<H5::DataSet>(
      this->m_file->createDataSet(path, H5type, dSpace, prop));

  return std::make_unique<HDF5RecordingData>(std::move(data));
}

H5O_type_t HDF5IO::getObjectType(const std::string& path)
{
#if H5_VERSION_GE(1, 12, 0)
  // get whether path is a dataset or group
  H5O_info_t objInfo;  // Structure to hold information about the object
  H5Oget_info_by_name(this->m_file->getId(),
                      path.c_str(),
                      &objInfo,
                      H5O_INFO_BASIC,
                      H5P_DEFAULT);
#else
  // get whether path is a dataset or group
  H5O_info_t objInfo;  // Structure to hold information about the object
  H5Oget_info_by_name(
      this - m_ > file->getId(), path.c_str(), &objInfo, H5P_DEFAULT);
#endif
  H5O_type_t objectType = objInfo.type;

  return objectType;
}

H5::DataType HDF5IO::getNativeType(BaseDataType type)
{
  H5::DataType baseType;

  switch (type.type) {
    case BaseDataType::Type::T_I8:
      baseType = PredType::NATIVE_INT8;
      break;
    case BaseDataType::Type::T_I16:
      baseType = PredType::NATIVE_INT16;
      break;
    case BaseDataType::Type::T_I32:
      baseType = PredType::NATIVE_INT32;
      break;
    case BaseDataType::Type::T_I64:
      baseType = PredType::NATIVE_INT64;
      break;
    case BaseDataType::Type::T_U8:
      baseType = PredType::NATIVE_UINT8;
      break;
    case BaseDataType::Type::T_U16:
      baseType = PredType::NATIVE_UINT16;
      break;
    case BaseDataType::Type::T_U32:
      baseType = PredType::NATIVE_UINT32;
      break;
    case BaseDataType::Type::T_U64:
      baseType = PredType::NATIVE_UINT64;
      break;
    case BaseDataType::Type::T_F32:
      baseType = PredType::NATIVE_FLOAT;
      break;
    case BaseDataType::Type::T_F64:
      baseType = PredType::NATIVE_DOUBLE;
      break;
    case BaseDataType::Type::T_STR:
      return StrType(PredType::C_S1, type.typeSize);
      break;
    case BaseDataType::Type::V_STR:
      return StrType(PredType::C_S1, H5T_VARIABLE);
      break;
    default:
      baseType = PredType::NATIVE_INT32;
  }
  if (type.typeSize > 1) {
    hsize_t size = type.typeSize;
    return ArrayType(baseType, 1, &size);
  } else
    return baseType;
}

H5::DataType HDF5IO::getH5Type(BaseDataType type)
{
  H5::DataType baseType;

  switch (type.type) {
    case BaseDataType::Type::T_I8:
      baseType = PredType::STD_I8LE;
      break;
    case BaseDataType::Type::T_I16:
      baseType = PredType::STD_I16LE;
      break;
    case BaseDataType::Type::T_I32:
      baseType = PredType::STD_I32LE;
      break;
    case BaseDataType::Type::T_I64:
      baseType = PredType::STD_I64LE;
      break;
    case BaseDataType::Type::T_U8:
      baseType = PredType::STD_U8LE;
      break;
    case BaseDataType::Type::T_U16:
      baseType = PredType::STD_U16LE;
      break;
    case BaseDataType::Type::T_U32:
      baseType = PredType::STD_U32LE;
      break;
    case BaseDataType::Type::T_U64:
      baseType = PredType::STD_U64LE;
      break;
    case BaseDataType::Type::T_F32:
      return PredType::IEEE_F32LE;
      break;
    case BaseDataType::Type::T_F64:
      baseType = PredType::IEEE_F64LE;
      break;
    case BaseDataType::Type::T_STR:
      return StrType(PredType::C_S1, type.typeSize);
      break;
    case BaseDataType::Type::V_STR:
      return StrType(PredType::C_S1, H5T_VARIABLE);
      break;
    default:
      return PredType::STD_I32LE;
  }
  if (type.typeSize > 1) {
    hsize_t size = type.typeSize;
    return ArrayType(baseType, 1, &size);
  } else
    return baseType;
}

// HDF5RecordingData
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
  this->m_dataset = std::make_unique<H5::DataSet>(*data);
}

// HDF5RecordingData

HDF5RecordingData::~HDF5RecordingData()
{
  // Safety
  this->m_dataset->flush(H5F_SCOPE_GLOBAL);
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
    this->m_dataset->extend(dSetDims.data());

    // Set size to new size based on updated dimensionality
    DataSpace fSpace = this->m_dataset->getSpace();
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
    this->m_dataset->write(data, nativeType, mSpace, fSpace);

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

const H5::DataSet* HDF5RecordingData::getDataSet()
{
  return this->m_dataset.get();
};
