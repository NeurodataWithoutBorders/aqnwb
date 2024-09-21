#include <codecvt>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include "io/hdf5/HDF5IO.hpp"

#include <H5Cpp.h>
#include <H5Fpublic.h>

#include "Utils.hpp"
#include "io/hdf5/HDF5RecordingData.hpp"

using namespace H5;
using namespace AQNWB::IO::HDF5;

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

  if (m_opened)
    return Status::Failure;

  FileAccPropList fapl = FileAccPropList::DEFAULT;
  H5Pset_libver_bounds(fapl.getId(), H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);

  if (newfile)
    accFlags = H5F_ACC_TRUNC;
  else
    accFlags = H5F_ACC_RDWR;

  m_file = std::make_unique<H5::H5File>(
      getFileName(), accFlags, FileCreatPropList::DEFAULT, fapl);
  m_opened = true;

  return Status::Success;
}

Status HDF5IO::close()
{
  if (m_file != nullptr && m_opened) {
    m_file->close();
    m_file = nullptr;
    m_opened = false;
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
  int status = H5Fflush(m_file->getId(), H5F_SCOPE_GLOBAL);
  return checkStatus(status);
}

std::unique_ptr<H5::Attribute> HDF5IO::getAttribute(const std::string& path)
{
  // Split the path to get the parent object and the attribute name
  size_t pos = path.find_last_of('/');
  if (pos == std::string::npos) {
    return nullptr;
  }

  std::string parentPath = path.substr(0, pos);
  std::string attrName = path.substr(pos + 1);

  try {
    // Try to open the parent object as a group
    H5::Group parentGroup = m_file->openGroup(parentPath);
    return std::make_unique<H5::Attribute>(parentGroup.openAttribute(attrName));
  } catch (const H5::Exception& e) {
    // If it's not a group, try to open it as a dataset
    try {
      H5::DataSet parentDataset = m_file->openDataSet(parentPath);
      return std::make_unique<H5::Attribute>(
          parentDataset.openAttribute(attrName));
    } catch (const H5::Exception& e) {
      // Handle HDF5 exceptions
      return nullptr;
    }
  }
}

StorageObjectType HDF5IO::getStorageObjectType(std::string path)
{
  try {
    H5O_type_t objectType = getH5ObjectType(path);

    switch (objectType) {
      case H5O_TYPE_GROUP:
        return StorageObjectType::Group;
      case H5O_TYPE_DATASET:
        return StorageObjectType::Dataset;
      case H5O_TYPE_NAMED_DATATYPE:
        // Handle named datatype if needed
        return StorageObjectType::Undefined;
      default:
        // Check if the object is an attribute
        if (auto attr = getAttribute(path)) {
          return StorageObjectType::Attribute;
        }
        return StorageObjectType::Undefined;
    }
  } catch (const H5::Exception& e) {
    // Handle HDF5 exceptions
    std::cerr << "HDF5 error: " << e.getDetailMsg() << std::endl;
    return StorageObjectType::Undefined;
  }
}

template<typename T, typename HDF5TYPE>
std::vector<T> HDF5IO::readDataHelper(const HDF5TYPE& dataSource,
                                      size_t numElements,
                                      const H5::DataSpace& memspace,
                                      const H5::DataSpace& dataspace)
{
  std::vector<T> data(numElements);
  if (const H5::DataSet* dataset =
          dynamic_cast<const H5::DataSet*>(&dataSource))
  {
    H5::DataType type = dataset->getDataType();
    dataset->read(data.data(), type, memspace, dataspace);
  } else if (const H5::Attribute* attribute =
                 dynamic_cast<const H5::Attribute*>(&dataSource))
  {
    H5::DataType type = attribute->getDataType();
    attribute->read(type, data.data());
  } else {
    throw std::runtime_error("Unsupported data source type");
  }

  return data;
}

template<typename T, typename HDF5TYPE>
std::vector<T> HDF5IO::readDataHelper(const HDF5TYPE& dataSource,
                                      size_t numElements)
{
  return this->readDataHelper<T>(
      dataSource, numElements, H5::DataSpace(), H5::DataSpace());
}

template<typename HDF5TYPE>
std::vector<std::string> HDF5IO::readStringDataHelper(
    const HDF5TYPE& dataSource,
    size_t numElements,
    const H5::DataSpace& memspace,
    const H5::DataSpace& dataspace)
{
  std::vector<std::string> data(numElements);
  if (const H5::DataSet* dataset =
          dynamic_cast<const H5::DataSet*>(&dataSource))
  {
    H5::StrType strType(H5::PredType::C_S1);
    dataset->read(data.data(), strType, memspace, dataspace);
  } else if (const H5::Attribute* attribute =
                 dynamic_cast<const H5::Attribute*>(&dataSource))
  {
    H5::StrType strType(H5::PredType::C_S1);
    attribute->read(strType, data.data());
  } else {
    throw std::runtime_error("Unsupported data source type");
  }
  return data;
}

template<typename HDF5TYPE>
std::vector<std::string> HDF5IO::readStringDataHelper(
    const HDF5TYPE& dataSource, size_t numElements)
{
  return this->readStringDataHelper(
      dataSource, numElements, H5::DataSpace(), H5::DataSpace());
}

AQNWB::IO::DataBlockGeneric HDF5IO::readAttribute(const std::string& dataPath)
{
  // Create the return value to fill
  AQNWB::IO::DataBlockGeneric result;
  // Read the attribute
  auto attributePtr = this->getAttribute(dataPath);
  // Make sure dataPath points to an attribute
  assert(attributePtr != nullptr);
  H5::Attribute& attribute = *attributePtr;
  H5::DataType dataType = attribute.getDataType();

  // Determine the shape of the attribute
  H5::DataSpace dataspace = attribute.getSpace();
  int rank = dataspace.getSimpleExtentNdims();
  if (rank == 0) {
    // Scalar attribute
    result.shape.clear();
  } else {
    std::vector<hsize_t> tempShape(rank);
    dataspace.getSimpleExtentDims(tempShape.data(), nullptr);
    result.shape.clear();
    result.shape.reserve(rank);
    for (const auto& v : tempShape) {
      result.shape.push_back(v);
    }
  }

  // Determine the size of the attribute from the shape
  size_t numElements = 1;  // Scalar (default)
  if (result.shape.size() > 0) {  // N-dimensional array
    for (const auto v : result.shape) {
      numElements *= v;
    }
  }

  // Read the attribute into a vector of the appropriate type
  if (dataType.getClass() == H5T_STRING) {
    if (dataType.isVariableStr()) {
      // Handle variable-length strings
      std::vector<std::string> stringData;
      try {
        // Allocate a buffer to hold the variable-length string data
        char* buffer;
        attribute.read(dataType, &buffer);

        // Convert the buffer to std::string
        stringData.emplace_back(buffer);

        // Free the memory allocated by HDF5
        H5Dvlen_reclaim(
            dataType.getId(), dataspace.getId(), H5P_DEFAULT, &buffer);
      } catch (const H5::Exception& e) {
        throw std::runtime_error(
            "Failed to read variable-length string attribute: "
            + std::string(e.getDetailMsg()));
      }
      result.data = stringData;
      result.typeIndex = typeid(std::string);
    } else {
      result.data = readStringDataHelper(attribute, numElements);
      result.typeIndex = typeid(std::string);
    }
  } else if (dataType == H5::PredType::NATIVE_DOUBLE) {
    result.data = readDataHelper<double>(attribute, numElements);
    result.typeIndex = typeid(double);
  } else if (dataType == H5::PredType::NATIVE_FLOAT) {
    result.data = readDataHelper<float>(attribute, numElements);
    result.typeIndex = typeid(float);
  } else if (dataType == H5::PredType::NATIVE_INT) {
    result.data = readDataHelper<int>(attribute, numElements);
    result.typeIndex = typeid(int);
  } else if (dataType == H5::PredType::NATIVE_UINT) {
    result.data = readDataHelper<unsigned int>(attribute, numElements);
    result.typeIndex = typeid(unsigned int);
  } else if (dataType == H5::PredType::NATIVE_LONG) {
    result.data = readDataHelper<long>(attribute, numElements);
    result.typeIndex = typeid(long);
  } else if (dataType == H5::PredType::NATIVE_ULONG) {
    result.data = readDataHelper<unsigned long>(attribute, numElements);
    result.typeIndex = typeid(unsigned long);
  } else if (dataType == H5::PredType::NATIVE_LLONG) {
    result.data = readDataHelper<long long>(attribute, numElements);
    result.typeIndex = typeid(long long);
  } else if (dataType == H5::PredType::NATIVE_ULLONG) {
    result.data = readDataHelper<unsigned long long>(attribute, numElements);
    result.typeIndex = typeid(unsigned long long);
  } else {
    throw std::runtime_error("Unsupported data type");
  }

  return result;
}

AQNWB::IO::DataBlockGeneric HDF5IO::readDataset(
    const std::string& dataPath,
    const std::vector<SizeType>& start,
    const std::vector<SizeType>& count,
    const std::vector<SizeType>& stride,
    const std::vector<SizeType>& block)
{
  // Check that the dataset exists
  assert(H5Lexists(m_file->getId(), dataPath.c_str(), H5P_DEFAULT) > 0);
  // create the return value to fill
  IO::DataBlockGeneric result;

  // Create new vectors of type hsize_t for stride and block because
  // HDF5 needs hsize_t and we use SizeType in AqNWB instead
  std::vector<hsize_t> stride_hsize(stride.size());
  std::vector<hsize_t> block_hsize(block.size());
  // Copy elements from the original vectors to the new vectors
  std::copy(stride.begin(), stride.end(), stride_hsize.begin());
  std::copy(block.begin(), block.end(), block_hsize.begin());

  // Read the dataset
  H5::DataSet dataset = m_file->openDataSet(dataPath);

  // Get the dataspace of the dataset
  H5::DataSpace dataspace = dataset.getSpace();

  // Get the number of dimensions and their sizes
  int rank = dataspace.getSimpleExtentNdims();
  hsize_t dims[rank];
  dataspace.getSimpleExtentDims(dims, nullptr);

  // Store the shape information
  result.shape.assign(dims, dims + rank);

  // Calculate the total number of elements
  size_t numElements = 1;
  for (int i = 0; i < rank; ++i) {
    numElements *= dims[i];
  }

  // Create a memory dataspace for the slice
  H5::DataSpace memspace;
  if (!start.empty() && !count.empty()) {
    hsize_t offset[rank];
    hsize_t block_count[rank];
    for (int i = 0; i < rank; ++i) {
      offset[i] = start[i];
      block_count[i] = count[i];
    }
    dataspace.selectHyperslab(
        H5S_SELECT_SET,
        block_count,
        offset,
        stride_hsize.empty() ? nullptr : stride_hsize.data(),
        block_hsize.empty() ? nullptr : block_hsize.data());
    memspace = H5::DataSpace(rank, block_count);
  } else {
    memspace = H5::DataSpace(dataspace);
  }

  // Read the dataset into a vector of the appropriate type
  H5::DataType dataType = dataset.getDataType();
  if (dataType == H5::PredType::C_S1) {
    result.data =
        readStringDataHelper(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(std::string);
  } else if (dataType == H5::PredType::NATIVE_DOUBLE) {
    result.data =
        readDataHelper<double>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(double);
  } else if (dataType == H5::PredType::NATIVE_FLOAT) {
    result.data =
        readDataHelper<float>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(float);
  } else if (dataType == H5::PredType::NATIVE_INT) {
    result.data =
        readDataHelper<int>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(int);
  } else if (dataType == H5::PredType::NATIVE_UINT) {
    result.data =
        readDataHelper<unsigned int>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(unsigned int);
  } else if (dataType == H5::PredType::NATIVE_LONG) {
    result.data =
        readDataHelper<long>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(long);
  } else if (dataType == H5::PredType::NATIVE_ULONG) {
    result.data = readDataHelper<unsigned long>(
        dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(unsigned long);
  } else if (dataType == H5::PredType::NATIVE_LLONG) {
    result.data =
        readDataHelper<long long>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(long long);
  } else if (dataType == H5::PredType::NATIVE_ULLONG) {
    result.data = readDataHelper<unsigned long long>(
        dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(unsigned long long);
  } else {
    throw std::runtime_error("Unsupported data type");
  }
  // return the result
  return result;
}

Status HDF5IO::createAttribute(const IO::BaseDataType& type,
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

  if (!m_opened)
    return Status::Failure;

  // open the group or dataset
  H5O_type_t objectType = getH5ObjectType(path);
  switch (objectType) {
    case H5O_TYPE_GROUP:
      gloc = m_file->openGroup(path);
      loc = &gloc;
      break;
    case H5O_TYPE_DATASET:
      dloc = m_file->openDataSet(path);
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

  if (!m_opened)
    return Status::Failure;

  StrType H5type(PredType::C_S1, maxSize);
  H5type.setSize(H5T_VARIABLE);

  // open the group or dataset
  H5O_type_t objectType = getH5ObjectType(path);
  switch (objectType) {
    case H5O_TYPE_GROUP:
      gloc = m_file->openGroup(path);
      loc = &gloc;
      break;
    case H5O_TYPE_DATASET:
      dloc = m_file->openDataSet(path);
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

  if (!m_opened)
    return Status::Failure;

  // open the group or dataset
  H5O_type_t objectType = getH5ObjectType(path);
  switch (objectType) {
    case H5O_TYPE_GROUP:
      gloc = m_file->openGroup(path);
      loc = &gloc;
      break;
    case H5O_TYPE_DATASET:
      dloc = m_file->openDataSet(path);
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

    m_file->reference(rdata, referencePath.c_str());

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
  if (!m_opened)
    return Status::Failure;
  try {
    m_file->createGroup(path);
  } catch (FileIException error) {
    error.printErrorStack();
  } catch (GroupIException error) {
    error.printErrorStack();
  }
  return Status::Success;
}

Status HDF5IO::createGroupIfDoesNotExist(const std::string& path)
{
  if (!m_opened)
    return Status::Failure;
  try {
    m_file->childObjType(path);
  } catch (FileIException) {
    return createGroup(path);
  }
  return Status::Success;
}

/** Creates a link to another location in the file */
Status HDF5IO::createLink(const std::string& path, const std::string& reference)
{
  if (!m_opened)
    return Status::Failure;

  herr_t error = H5Lcreate_soft(reference.c_str(),
                                m_file->getLocId(),
                                path.c_str(),
                                H5P_DEFAULT,
                                H5P_DEFAULT);

  return checkStatus(error);
}

Status HDF5IO::createReferenceDataSet(
    const std::string& path, const std::vector<std::string>& references)
{
  if (!m_opened)
    return Status::Failure;

  const hsize_t size = references.size();

  hobj_ref_t* rdata = new hobj_ref_t[size * sizeof(hobj_ref_t)];

  for (SizeType i = 0; i < size; i++) {
    m_file->reference(&rdata[i], references[i].c_str());
  }

  hid_t space = H5Screate_simple(1, &size, NULL);

  hid_t dset = H5Dcreate(m_file->getLocId(),
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
  if (!m_opened)
    return Status::Failure;

  std::unique_ptr<H5::DataSet> dataset;
  DataType H5type = getH5Type(IO::BaseDataType::STR(value.length()));
  DataSpace dSpace(H5S_SCALAR);

  dataset = std::make_unique<H5::DataSet>(
      m_file->createDataSet(path, H5type, dSpace));
  dataset->write(value.c_str(), H5type);

  return Status::Success;
}

Status HDF5IO::createStringDataSet(const std::string& path,
                                   const std::vector<std::string>& values)
{
  if (!m_opened)
    return Status::Failure;

  std::vector<const char*> cStrs;
  cStrs.reserve(values.size());
  for (const auto& str : values) {
    cStrs.push_back(str.c_str());
  }

  std::unique_ptr<IO::BaseRecordingData> dataset;
  dataset = std::unique_ptr<IO::BaseRecordingData>(createArrayDataSet(
      IO::BaseDataType::V_STR, SizeArray {values.size()}, SizeArray {1}, path));
  dataset->writeDataBlock(
      std::vector<SizeType>(1, 1), IO::BaseDataType::V_STR, cStrs.data());

  return Status::Success;
}

Status HDF5IO::startRecording()
{
  if (!m_opened)
    return Status::Failure;

  if (!m_disableSWMRMode) {
    herr_t status = H5Fstart_swmr_write(m_file->getId());
    return checkStatus(status);
  }
  return Status::Success;
}

Status HDF5IO::stopRecording()
{
  // if SWMR mode is disabled, stopping the recording will leave the file open
  if (!m_disableSWMRMode) {
    close();  // SWMR mode cannot be disabled so close the file
  } else {
    this->flush();
  }
  return Status::Success;
}

bool HDF5IO::canModifyObjects()
{
  if (!m_opened)
    return false;

  // Check if we are in SWMR mode
  bool inSWMRMode = false;
  unsigned int intent;
  herr_t status = H5Fget_intent(m_file->getId(), &intent);
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
  htri_t exists = H5Lexists(m_file->getId(), path.c_str(), H5P_DEFAULT);
  if (exists > 0) {
    return true;
  } else {
    return false;
  }
}

std::unique_ptr<AQNWB::IO::BaseRecordingData> HDF5IO::getDataSet(
    const std::string& path)
{
  std::unique_ptr<DataSet> data;

  if (!m_opened)
    return nullptr;

  try {
    data = std::make_unique<H5::DataSet>(m_file->openDataSet(path));
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

std::unique_ptr<AQNWB::IO::BaseRecordingData> HDF5IO::createArrayDataSet(
    const IO::BaseDataType& type,
    const SizeArray& size,
    const SizeArray& chunking,
    const std::string& path)
{
  std::unique_ptr<DataSet> data;
  DSetCreatPropList prop;
  DataType H5type = getH5Type(type);

  if (!m_opened)
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
      m_file->createDataSet(path, H5type, dSpace, prop));

  return std::make_unique<HDF5RecordingData>(std::move(data));
}

H5O_type_t HDF5IO::getH5ObjectType(const std::string& path)
{
#if H5_VERSION_GE(1, 12, 0)
  // get whether path is a dataset or group
  H5O_info_t objInfo;  // Structure to hold information about the object
  H5Oget_info_by_name(
      m_file->getId(), path.c_str(), &objInfo, H5O_INFO_BASIC, H5P_DEFAULT);
#else
  // get whether path is a dataset or group
  H5O_info_t objInfo;  // Structure to hold information about the object
  H5Oget_info_by_name(m_file->getId(), path.c_str(), &objInfo, H5P_DEFAULT);
#endif
  H5O_type_t objectType = objInfo.type;

  return objectType;
}

H5::DataType HDF5IO::getNativeType(IO::BaseDataType type)
{
  H5::DataType baseType;

  switch (type.type) {
    case IO::BaseDataType::Type::T_I8:
      baseType = PredType::NATIVE_INT8;
      break;
    case IO::BaseDataType::Type::T_I16:
      baseType = PredType::NATIVE_INT16;
      break;
    case IO::BaseDataType::Type::T_I32:
      baseType = PredType::NATIVE_INT32;
      break;
    case IO::BaseDataType::Type::T_I64:
      baseType = PredType::NATIVE_INT64;
      break;
    case IO::BaseDataType::Type::T_U8:
      baseType = PredType::NATIVE_UINT8;
      break;
    case IO::BaseDataType::Type::T_U16:
      baseType = PredType::NATIVE_UINT16;
      break;
    case IO::BaseDataType::Type::T_U32:
      baseType = PredType::NATIVE_UINT32;
      break;
    case IO::BaseDataType::Type::T_U64:
      baseType = PredType::NATIVE_UINT64;
      break;
    case IO::BaseDataType::Type::T_F32:
      baseType = PredType::NATIVE_FLOAT;
      break;
    case IO::BaseDataType::Type::T_F64:
      baseType = PredType::NATIVE_DOUBLE;
      break;
    case IO::BaseDataType::Type::T_STR:
      return StrType(PredType::C_S1, type.typeSize);
      break;
    case IO::BaseDataType::Type::V_STR:
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

H5::DataType HDF5IO::getH5Type(IO::BaseDataType type)
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