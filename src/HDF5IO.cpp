#include <codecvt>
#include <memory>
#include <filesystem>

#include <H5Cpp.h>
#include "HDF5IO.hpp"

using namespace H5;
using namespace AQNWBIO;

// HDF5IO

HDF5IO::HDF5IO() {}

HDF5IO::~HDF5IO() {
    close();
}

HDF5IO::HDF5IO(std::string fileName)
    : filename(fileName)
{
}

std::string HDF5IO::getFileName()
{
  return filename;
}

int HDF5IO::open()
{
  if (!readyToOpen)
    return -1;

  if (std::filesystem::exists(getFileName()))
    newfile = false;
  else
    newfile = true;

  if (opened)
    return -1;

  return open(newfile);
}

int HDF5IO::open(bool newfile)
{
  int accFlags, ret = 0;

  if (opened)
    return -1;

  FileAccPropList props = FileAccPropList::DEFAULT;

  if (newfile)
    accFlags = H5F_ACC_TRUNC;
  else
    accFlags = H5F_ACC_RDWR;
  file = std::make_unique<H5::H5File>(getFileName(), accFlags, FileCreatPropList::DEFAULT, props);
  opened = true;

  if (newfile) {
    ret = createFileStructure();
  }

  if (ret) {
    file = nullptr;
    opened = false;
    std::cerr << "Error creating file structure" << std::endl;
  }

  return ret;
}

void HDF5IO::close()
{
  file = nullptr;
  opened = false;
}

int HDF5IO::createFileStructure()  // TODO - move this to NWB class
{
    return 0;
}


int HDF5IO::setAttribute(BaseDataType type,
                         const void* data,
                         std::string path,
                         std::string name,
                         int size)
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
    dloc = file->openDataSet(path);  // do I need to convert strings to UTF8?
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

int HDF5IO::createGroup(std::string path)
{
  if (!opened)
    return -1;
    try
    {
      file->createGroup(path);
    }
    catch (FileIException error)
    {
        showError(error.getCDetailMsg());
    }
    catch (GroupIException error)
    {
        showError(error.getCDetailMsg());
    } 
  return 0;
}

int HDF5IO::createGroupIfDoesNotExist(std::string path)
{
  if (!opened)
    return -1;
  try {
    // std::u16string u16_conv = std::wstring_convert<
    //     std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path);
    // const char * c = path.c_str();
    file->childObjType(path);


  } catch (FileIException) {
    return createGroup(path);
  }
  return 0;
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
    // std::cout << "DataSetIException" << std::endl;
    error.printErrorStack();
    return nullptr;
  } catch (FileIException error) {
    // std::cout << "FileIException" << std::endl;
    error.printErrorStack();
    return nullptr;
  } catch (DataSpaceIException error) {
    // std::cout << "DataSpaceIException" << std::endl;
    error.printErrorStack();
    return nullptr;
  }
}

HDF5RecordingData* HDF5IO::createDataSet(BaseDataType type,
                                         int sizeX,
                                         int chunkX,
                                         std::string path)
{
  int chunks[3] = {chunkX, 0, 0};
  return createDataSet(type, 1, &sizeX, chunks, path);
}

HDF5RecordingData* HDF5IO::createDataSet(BaseDataType type, int sizeX, int sizeY, int chunkX, std::string path)
{
    int size[2];
    int chunks[3] = {chunkX, 0, 0};
    size[0] = sizeX;
    size[1] = sizeY;
    return createDataSet(type,2,size,chunks,path);
}

HDF5RecordingData* HDF5IO::createDataSet(BaseDataType type, int sizeX, int sizeY, int sizeZ, int chunkX, std::string path)
{
    int size[3];
    int chunks[3] = {chunkX, 0, 0};
    size[0] = sizeX;
    size[1] = sizeY;
    size[2] = sizeZ;
    return createDataSet(type,3,size,chunks,path);
}

HDF5RecordingData* HDF5IO::createDataSet(BaseDataType type, int sizeX, int sizeY, int sizeZ, int chunkX, int chunkY, std::string path)
{
    int size[3];
    int chunks[3] = {chunkX, chunkY, 0};
    size[0] = sizeX;
    size[1] = sizeY;
    size[2] = sizeZ;
    return createDataSet(type,3,size,chunks,path);
}

HDF5RecordingData* HDF5IO::createDataSet(BaseDataType type,
                                         int dimension,
                                         int* size,
                                         int* chunking,
                                         std::string path)
{
  std::unique_ptr<DataSet> data;
  DSetCreatPropList prop;
  if (!opened)
    return nullptr;

  // Right now this classes don't support datasets with rank > 3.
  // If it's needed in the future we can extend them to be of generic rank
  if ((dimension > 3) || (dimension < 1))
    return nullptr;

  DataType H5type = getH5Type(type);

  hsize_t dims[3], chunk_dims[3], max_dims[3];

  for (int i = 0; i < dimension; i++) {
    dims[i] = static_cast<hsize_t>(size[i]);
    if (chunking[i] > 0) {
      chunk_dims[i] = static_cast<hsize_t>(chunking[i]);
      max_dims[i] = H5S_UNLIMITED;
    } else {
      chunk_dims[i] = static_cast<hsize_t>(size[i]);
      max_dims[i] = static_cast<hsize_t>(size[i]);
    }
  }

  DataSpace dSpace(dimension, dims, max_dims);
  prop.setChunk(dimension, chunk_dims);

  data = std::make_unique<H5::DataSet>(file->createDataSet(path, H5type, dSpace, prop));
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
//   std::unique_ptr<H5::DataSet> dataSet = std::make_unique<H5::DataSet>(*data);
  // ScopedPointer<DataSet> dataSet = data;
  hsize_t dims[3], chunk[3];

  dSpace = data->getSpace();
  prop = data->getCreatePlist();

  dimension = dSpace.getSimpleExtentDims(dims);
  prop.getChunk(dimension, chunk);

  this->size[0] = static_cast<int>(dims[0]);
  if (dimension > 1)
    this->size[1] = static_cast<int>(dims[1]);
  else
    this->size[1] = 1;
  if (dimension > 1)
    this->size[2] = static_cast<int>(dims[2]);
  else
    this->size[2] = 1;

  this->xChunkSize = static_cast<int>(chunk[0]);
  this->xPos = 0;
  this->dSet = std::make_unique<H5::DataSet>(*data);;
  this->rowXPos.clear();
  this->rowXPos.insert(this->rowXPos.end(), static_cast<size_t>(this->size[1]), 0);
}

// HDF5RecordingData

HDF5RecordingData::~HDF5RecordingData()
{
  // Safety
  dSet->flush(H5F_SCOPE_GLOBAL);
}

int HDF5RecordingData::writeDataBlock(int xDataSize, int yDataSize, BaseDataType type, const void* data)
{
    hsize_t dim[3],offset[3];
    DataSpace fSpace;
    DataType nativeType;

    dim[2] = static_cast<hsize_t>(size[2]);
    //only modify y size if new required size is larger than what we had.
    if (yDataSize > size[1])
        dim[1] = static_cast<hsize_t>(yDataSize);
    else
        dim[1] = static_cast<hsize_t>(size[1]);
    dim[0] = static_cast<hsize_t>(xPos + xDataSize);
    
    //First be sure that we have enough space
    dSet->extend(dim);

    fSpace = dSet->getSpace();
    fSpace.getSimpleExtentDims(dim);
    size[0] = static_cast<int>(dim[0]);
    if (dimension > 1)
        size[1] = static_cast<int>(dim[1]);

    //Create memory space
    dim[0] = static_cast<hsize_t>(xDataSize);
    dim[1] = static_cast<hsize_t>(yDataSize);
    dim[2] = static_cast<hsize_t>(size[2]);

    DataSpace mSpace(dimension,dim);
    //select where to write
    offset[0] = static_cast<hsize_t>(xPos);
    offset[1] = 0;
    offset[2] = 0;

    fSpace.selectHyperslab(H5S_SELECT_SET, dim, offset);

    nativeType = HDF5IO::getNativeType(type);

    dSet->write(data,nativeType,mSpace,fSpace);
    xPos += xDataSize;

    return 0;
}