#include <codecvt>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include "HDF5IO.hpp"
#include "Utils.hpp"

#include <H5Cpp.h>

using namespace H5;
using namespace AQNWBIO;

// HDF5IO

HDF5IO::HDF5IO() {}

HDF5IO::HDF5IO(std::string fileName)
    : filename(fileName)
{
}

HDF5IO::~HDF5IO()
{
  close();
}

std::string HDF5IO::getFileName()
{
  return filename;
}

int HDF5IO::open()
{
  if (std::filesystem::exists(getFileName())) {
    return open(false);
  } else {
    return open(true);
  }
}

int HDF5IO::open(bool newfile)
{
  int accFlags = 0;

  if (opened)
    return -1;

  FileAccPropList props = FileAccPropList::DEFAULT;

  if (newfile)
    accFlags = H5F_ACC_TRUNC;
  else
    accFlags = H5F_ACC_RDWR;

  file = std::make_unique<H5::H5File>(
      getFileName(), accFlags, FileCreatPropList::DEFAULT, props);
  opened = true;

  return 0;
}

void HDF5IO::close()
{
  file = nullptr;
  opened = false;
}

int HDF5IO::setAttribute(BaseDataType type,
                         const void* data,
                         std::string path,
                         std::string name,
                         size_t size)
{
  H5Object* loc;
  Group gloc;
  DataSet dloc;
  Attribute attr;
  DataType H5type;
  DataType origType;

  if (!opened)
    return -1;

  try {
    gloc = file->openGroup(path);
    loc = &gloc;
  } catch (FileIException
               error)  // If there is no group with that path, try a dataset
  {
    dloc = file->openDataSet(path);
    loc = &dloc;
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

  return 0;
}

int HDF5IO::setAttribute(const std::string& data,
                         std::string path,
                         std::string name)
{
  std::vector<const char*> dataPtrs;
  dataPtrs.push_back(data.c_str());

  return setAttribute(dataPtrs, path, name, data.length());
}

int HDF5IO::setAttribute(const std::vector<std::string>& data,
                         std::string path,
                         std::string name)
{
  std::vector<const char*> dataPtrs;
  size_t maxLength = 0;
  for (const std::string& str : data) {
    size_t length = str.length();
    maxLength = std::max(maxLength, length);
    dataPtrs.push_back(str.c_str());
  }

  return setAttribute(dataPtrs, path, name, maxLength + 1);
}

int HDF5IO::setAttribute(const std::vector<const char*>& data,
                         std::string path,
                         std::string name,
                         size_t maxSize)
{
  H5Object* loc;
  Group gloc;
  DataSet dloc;
  Attribute attr;
  hsize_t dims[1];

  if (!opened)
    return -1;

  StrType H5type(PredType::C_S1, maxSize);
  H5type.setSize(H5T_VARIABLE);

  try {
    gloc = file->openGroup(path);
    loc = &gloc;
  } catch (FileIException
               error)  // If there is no group with that path, try a dataset
  {
    dloc = file->openDataSet(path);
    loc = &dloc;
  }

  try {
    if (loc->attrExists(name)) {
      return -1;  // don't allow overwriting because string attributes cannot
                  // change size easily
    } else {
      DataSpace attr_dataspace;
      size_t nStrings = data.size();
      if (nStrings > 1) {
        dims[0] = nStrings;
        attr_dataspace = DataSpace(1, dims);
      } else
        attr_dataspace = DataSpace(H5S_SCALAR);
      attr = loc->createAttribute(name, H5type, attr_dataspace);
    }
    attr.write(H5type, data.data());
  } catch (GroupIException error) {
    showError(error.getCDetailMsg());
  } catch (AttributeIException error) {
    showError(error.getCDetailMsg());
  } catch (FileIException error) {
    showError(error.getCDetailMsg());
  } catch (DataSetIException error) {
    showError(error.getCDetailMsg());
  }
  return 0;
}

int HDF5IO::setAttributeRef(std::string referencePath,
                            std::string path,
                            std::string name)
{
  H5Object* loc;
  Group gloc;
  DataSet dloc;
  Attribute attr;

  if (!opened)
    return -1;

  try {
    gloc = file->openGroup(path);
    loc = &gloc;
  } catch (FileIException
               error)  // If there is no group with that path, try a dataset
  {
    dloc = file->openDataSet(path);
    loc = &dloc;
  }

  try {
    if (loc->attrExists(name)) {
      attr = loc->openAttribute(name);
    } else {
      DataType data_type(H5T_STD_REF_OBJ);
      DataSpace attr_space(H5S_SCALAR);
      attr = loc->createAttribute(name, data_type, attr_space);
    }

    hobj_ref_t* rdata = new hobj_ref_t[sizeof(hobj_ref_t)];

    file->reference(rdata, referencePath.c_str());

    attr.write(H5T_STD_REF_OBJ, rdata);
    delete[] rdata;

  } catch (GroupIException error) {
    showError(error.getCDetailMsg());
  } catch (AttributeIException error) {
    showError(error.getCDetailMsg());
  } catch (FileIException error) {
    showError(error.getCDetailMsg());
  } catch (DataSetIException error) {
    showError(error.getCDetailMsg());
  }

  return 0;
}

int HDF5IO::createGroup(std::string path)
{
  if (!opened)
    return -1;
  try {
    file->createGroup(path);
  } catch (FileIException error) {
    showError(error.getCDetailMsg());
  } catch (GroupIException error) {
    showError(error.getCDetailMsg());
  }
  return 0;
}

int HDF5IO::createGroupIfDoesNotExist(std::string path)
{
  if (!opened)
    return -1;
  try {
    file->childObjType(path);
  } catch (FileIException) {
    return createGroup(path);
  }
  return 0;
}

/** Creates a link to another location in the file */
void HDF5IO::createLink(std::string path, std::string reference)
{
  H5Lcreate_soft(reference.c_str(),
                 file->getLocId(),
                 path.c_str(),
                 H5P_DEFAULT,
                 H5P_DEFAULT);
}

void HDF5IO::createReferenceDataSet(std::string path,
                                    std::vector<std::string> references)
{
  const hsize_t size = references.size();

  hobj_ref_t* rdata = new hobj_ref_t[size * sizeof(hobj_ref_t)];

  for (size_t i = 0; i < size; i++) {
    file->reference(&rdata[i], references[i].c_str());
  }

  hid_t space = H5Screate_simple(1, &size, NULL);

  hid_t dset = H5Dcreate(file->getLocId(),
                         path.c_str(),
                         H5T_STD_REF_OBJ,
                         space,
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);

  herr_t status =
      H5Dwrite(dset, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);

  delete[] rdata;

  status = H5Dclose(dset);
  status = H5Dclose(space);
}

HDF5RecordingData* HDF5IO::getDataSet(std::string path)
{
  std::unique_ptr<DataSet> data;

  if (!opened)
    return nullptr;

  try {
    data = std::make_unique<H5::DataSet>(file->openDataSet(path));
    return new HDF5RecordingData(data.release());
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

void HDF5IO::createStringDataSet(std::string path, std::string value)
{
  std::unique_ptr<H5::DataSet> dataset;
  DataType H5type = getH5Type(BaseDataType::STR(value.length()));
  DataSpace dSpace(H5S_SCALAR);

  dataset =
      std::make_unique<H5::DataSet>(file->createDataSet(path, H5type, dSpace));
  dataset->write(value.c_str(), H5type);
}

BaseRecordingData* HDF5IO::createDataSet(BaseDataType type,
                                         const std::vector<size_t>& size,
                                         const std::vector<size_t>& chunking,
                                         std::string const path)
{
  std::unique_ptr<DataSet> data;
  DSetCreatPropList prop;
  if (!opened)
    return nullptr;

  // Right now this classes don't support datasets with rank > 3.
  // If it's needed in the future we can extend them to be of generic rank
  size_t dimension = size.size();
  if ((dimension > 3) || (dimension < 1))
    return nullptr;

  DataType H5type = getH5Type(type);

  hsize_t dims[3], chunk_dims[3], max_dims[3];

  for (size_t i = 0; i < dimension; i++) {
    dims[i] = static_cast<hsize_t>(size[i]);
    if (chunking[i] > 0) {
      chunk_dims[i] = static_cast<hsize_t>(chunking[i]);
      max_dims[i] = H5S_UNLIMITED;
    } else {
      chunk_dims[i] = static_cast<hsize_t>(size[i]);
      max_dims[i] = static_cast<hsize_t>(size[i]);
    }
  }

  DataSpace dSpace(static_cast<int>(dimension), dims, max_dims);
  prop.setChunk(static_cast<int>(dimension), chunk_dims);

  data = std::make_unique<H5::DataSet>(
      file->createDataSet(path, H5type, dSpace, prop));
  return new HDF5RecordingData(data.release());
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
HDF5RecordingData::HDF5RecordingData(H5::DataSet* data)
{
  DataSpace dSpace;
  DSetCreatPropList prop;
  //   std::unique_ptr<H5::DataSet> dataSet =
  //   std::make_unique<H5::DataSet>(*data);
  // ScopedPointer<DataSet> dataSet = data;
  hsize_t dims[3], chunk[3];

  dSpace = data->getSpace();
  prop = data->getCreatePlist();

  dimension = dSpace.getSimpleExtentDims(dims);
  prop.getChunk(static_cast<int>(dimension), chunk);

  this->size[0] = dims[0];
  if (dimension > 1)
    this->size[1] = dims[1];
  else
    this->size[1] = 1;
  if (dimension > 1)
    this->size[2] = dims[2];
  else
    this->size[2] = 1;

  this->xChunkSize = chunk[0];
  this->xPos = 0;
  this->dSet = std::make_unique<H5::DataSet>(*data);
  ;
  this->rowXPos.clear();
  this->rowXPos.insert(
      this->rowXPos.end(), static_cast<size_t>(this->size[1]), 0);
}

// HDF5RecordingData

HDF5RecordingData::~HDF5RecordingData()
{
  // Safety
  dSet->flush(H5F_SCOPE_GLOBAL);
}

int HDF5RecordingData::writeDataBlock(size_t xDataSize,
                                      size_t yDataSize,
                                      BaseDataType type,
                                      const void* data)
{
  hsize_t dim[3], offset[3];
  DataSpace fSpace;
  DataType nativeType;

  dim[2] = static_cast<hsize_t>(size[2]);
  // only modify y size if new required size is larger than what we had.
  if (yDataSize > size[1])
    dim[1] = static_cast<hsize_t>(yDataSize);
  else
    dim[1] = static_cast<hsize_t>(size[1]);
  dim[0] = static_cast<hsize_t>(xPos) + xDataSize;

  // First be sure that we have enough space
  dSet->extend(dim);

  fSpace = dSet->getSpace();
  fSpace.getSimpleExtentDims(dim);
  size[0] = dim[0];
  if (dimension > 1)
    size[1] = dim[1];

  // Create memory space
  dim[0] = static_cast<hsize_t>(xDataSize);
  dim[1] = static_cast<hsize_t>(yDataSize);
  dim[2] = static_cast<hsize_t>(size[2]);

  DataSpace mSpace(static_cast<int>(dimension), dim);
  // select where to write
  offset[0] = static_cast<hsize_t>(xPos);
  offset[1] = 0;
  offset[2] = 0;

  fSpace.selectHyperslab(H5S_SELECT_SET, dim, offset);

  nativeType = HDF5IO::getNativeType(type);

  dSet->write(data, nativeType, mSpace, fSpace);
  xPos += xDataSize;

  return 0;
}
