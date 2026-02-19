#include <cassert>
#include <codecvt>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "io/hdf5/HDF5IO.hpp"

#include <H5Cpp.h>
#include <H5Fpublic.h>

#include "Utils.hpp"
#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"
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
    return open(FileMode::ReadWrite);
  } else {
    return open(FileMode::Overwrite);
  }
}

Status HDF5IO::open(FileMode mode)
{
  unsigned int accFlags = 0;

  if (m_opened) {
    return Status::Failure;
  }

  if (!std::filesystem::exists(getFileName())) {
    if (mode == FileMode::ReadWrite || mode == FileMode::ReadOnly) {
      return Status::Failure;
    }
  }

  FileAccPropList fapl = FileAccPropList::DEFAULT;
  H5Pset_libver_bounds(fapl.getId(), H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);

  switch (mode) {
    case FileMode::Overwrite:
      accFlags = H5F_ACC_TRUNC;
      break;
    case FileMode::ReadWrite:
      accFlags = H5F_ACC_RDWR;
      break;
    case FileMode::ReadOnly:
      accFlags = H5F_ACC_RDONLY | H5F_ACC_SWMR_READ;
      break;
    default:
      throw std::invalid_argument("Invalid file mode");
  }

  m_file = std::make_unique<H5::H5File>(
      getFileName(), accFlags, FileCreatPropList::DEFAULT, fapl);
  m_opened = true;

  return Status::Success;
}

Status HDF5IO::close()
{
  auto baseCloseStatus = BaseIO::close();  // clear the recording containers
  // Close the file if it is open
  if (m_file != nullptr && m_opened) {
    m_file->close();
    m_file = nullptr;
    m_opened = false;
  }
  return baseCloseStatus;
}

Status HDF5IO::flush()
{
  int status = H5Fflush(m_file->getId(), H5F_SCOPE_GLOBAL);
  return intToStatus(status);
}

std::unique_ptr<H5::Attribute> HDF5IO::getAttribute(
    const std::string& path) const
{
  // Split the path to get the parent object and the attribute name
  size_t pos = path.find_last_of('/');
  if (pos == std::string::npos) {
    std::cerr << "Invalid path: " << path << std::endl;
    return nullptr;
  }

  std::string parentPath = path.substr(0, pos);
  std::string attrName = path.substr(pos + 1);
  // If we are at the root, set parentPath to "/"
  if (parentPath.empty()) {
    parentPath = "/";
  }

  // open the group or dataset
  H5O_type_t objectType = getH5ObjectType(parentPath);
  if (objectType == H5O_TYPE_GROUP) {
    Group group = m_file->openGroup(parentPath);
    if (group.attrExists(attrName)) {
      return std::make_unique<H5::Attribute>(group.openAttribute(attrName));
    }
  } else if (objectType == H5O_TYPE_DATASET) {
    DataSet dataset = m_file->openDataSet(parentPath);
    if (dataset.attrExists(attrName)) {
      return std::make_unique<H5::Attribute>(dataset.openAttribute(attrName));
    }
  }
  return nullptr;
}

StorageObjectType HDF5IO::getStorageObjectType(std::string path) const
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
                                      const H5::DataSpace& dataspace) const
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
                                      size_t numElements) const
{
  return this->readDataHelper<T>(
      dataSource, numElements, H5::DataSpace(), H5::DataSpace());
}

template<typename HDF5TYPE>
std::vector<std::string> HDF5IO::readStringDataHelper(
    const HDF5TYPE& dataSource,
    size_t numElements,
    const H5::DataSpace& memspace,
    const H5::DataSpace& dataspace) const
{
  std::vector<std::string> data(numElements);

  try {
    // Check if the data source is a DataSet
    if (const H5::DataSet* dataset =
            dynamic_cast<const H5::DataSet*>(&dataSource))
    {
      // Get the string type of the dataset
      H5::StrType strType = dataset->getStrType();

      // Check if the dataset is empty
      if (numElements == 0) {
        return data;
      }

      if (strType.isVariableStr()) {
        // Handle variable-length strings
        std::vector<char*> buffer(numElements, nullptr);
        dataset->read(buffer.data(), strType, memspace, dataspace);

        // Convert char* to std::string and free allocated memory
        for (size_t i = 0; i < numElements; ++i) {
          if (buffer[i] == nullptr) {
            // Handle empty strings gracefully
            data[i] = "";
          } else {
            data[i] = std::string(buffer[i]);
            free(buffer[i]);  // Free the memory allocated by HDF5
          }
        }
      } else {
        // Handle fixed-length strings
        size_t strSize =
            strType.getSize();  // Get the size of the fixed-length string
        std::vector<char> buffer(numElements * strSize);
        dataset->read(buffer.data(), strType, memspace, dataspace);
        for (size_t i = 0; i < numElements; ++i) {
          data[i] = std::string(buffer.data() + i * strSize, strSize);
        }
      }
    }
    // Check if the data source is an Attribute
    else if (const H5::Attribute* attribute =
                 dynamic_cast<const H5::Attribute*>(&dataSource))
    {
      // Get the string type of the attribute
      H5::StrType strType = attribute->getStrType();

      // Check if the attribute is empty
      if (numElements == 0) {
        return data;
      }

      if (strType.isVariableStr()) {
        // Handle variable-length strings
        std::vector<char*> buffer(numElements, nullptr);
        attribute->read(strType, buffer.data());

        // Convert char* to std::string and free allocated memory
        for (size_t i = 0; i < numElements; ++i) {
          if (buffer[i] == nullptr) {
            // Handle empty strings gracefully
            data[i] = "";
          } else {
            data[i] = std::string(buffer[i]);
            free(buffer[i]);  // Free the memory allocated by HDF5
          }
        }
      } else {
        // Handle fixed-length strings
        size_t strSize =
            strType.getSize();  // Get the size of the fixed-length string
        std::vector<char> buffer(numElements * strSize);
        attribute->read(strType, buffer.data());
        for (size_t i = 0; i < numElements; ++i) {
          data[i] = std::string(buffer.data() + i * strSize, strSize);
        }
      }
    } else {
      // Throw an error if the data source type is unsupported
      throw std::runtime_error("Unsupported data source type");
    }
  } catch (const H5::Exception& e) {
    // Catch and rethrow HDF5 exceptions with a detailed message
    throw std::runtime_error("Failed to read string data: "
                             + std::string(e.getDetailMsg()));
  }

  return data;
}

template<typename HDF5TYPE>
std::vector<std::string> HDF5IO::readStringDataHelper(
    const HDF5TYPE& dataSource, size_t numElements) const
{
  // Call the main readStringDataHelper function with default DataSpaces
  return this->readStringDataHelper(
      dataSource, numElements, H5::DataSpace(), H5::DataSpace());
}

std::string HDF5IO::readReferenceAttribute(const std::string& dataPath) const
{
  // Read the attribute
  auto attributePtr = this->getAttribute(dataPath);
  if (attributePtr == nullptr) {
    throw std::invalid_argument(
        "HDF5IO::readReferenceAttribute, attribute does not exist.");
  }

  H5::Attribute& attribute = *attributePtr;
  H5::DataType dataType = attribute.getDataType();

  // Check if the attribute is a reference
  if (dataType != H5::PredType::STD_REF_OBJ) {
    throw std::invalid_argument(
        "HDF5IO::readReferenceAttribute, attribute is not a reference.");
  }

  // Read the reference
  hobj_ref_t ref;
  attribute.read(dataType, &ref);

  // Dereference the reference to get the HDF5 object ID
  // TODO: Note as of HDF5-1.12, H5Rdereference2() has been deprecated in
  //       favor of H5Ropen_attr(), H5Ropen_object() and H5Ropen_region().
  hid_t obj_id =
      H5Rdereference2(attribute.getId(), H5P_DEFAULT, H5R_OBJECT, &ref);
  if (obj_id < 0) {
    throw std::runtime_error(
        "HDF5IO::readReferenceAttribute, failed to dereference object.");
  }

  // Get the name (path) of the dereferenced object
  ssize_t buf_size = H5Iget_name(obj_id, nullptr, 0) + 1;
  // LCOV_EXCL_START
  // This is a safety check to safeguard against possible runtime issues,
  // but this should never happen.
  if (buf_size <= 0) {
    H5Oclose(obj_id);
    throw std::runtime_error(
        "HDF5IO::readReferenceAttribute, failed to get object name size.");
  }
  // LCOV_EXCL_STOP

  std::vector<char> obj_name(static_cast<size_t>(buf_size));
  // LCOV_EXCL_START
  // This is a safety check to safeguard against possible runtime issues,
  // but this should never happen.
  if (H5Iget_name(obj_id, obj_name.data(), static_cast<size_t>(buf_size)) < 0) {
    H5Oclose(obj_id);
    throw std::runtime_error(
        "HDF5IO::readReferenceAttribute, failed to get object name.");
  }
  // LCOV_EXCL_STOP

  std::string referencedPath(obj_name.data());

  // Close the dereferenced object
  H5Oclose(obj_id);

  return referencedPath;
}

AQNWB::IO::DataBlockGeneric HDF5IO::readAttribute(
    const std::string& dataPath) const
{
  // Create the return value to fill
  AQNWB::IO::DataBlockGeneric result;
  // Read the attribute
  auto attributePtr = this->getAttribute(dataPath);
  if (attributePtr == nullptr) {
    throw std::invalid_argument(
        "HDF5IO::readAttribute, attribute does not exist. " + dataPath);
  }

  H5::Attribute& attribute = *attributePtr;
  H5::DataType dataType = attribute.getDataType();

  // Determine the shape of the attribute
  H5::DataSpace dataspace = attribute.getSpace();
  SizeType rank = static_cast<SizeType>(dataspace.getSimpleExtentNdims());
  result.shape.clear();
  if (rank > 0) {
    std::vector<hsize_t> tempShape(rank);
    dataspace.getSimpleExtentDims(tempShape.data(), nullptr);
    result.shape.assign(tempShape.begin(), tempShape.end());
  }

  // Determine the size of the attribute from the shape
  size_t numElements = 1;
  for (const auto v : result.shape) {
    numElements *= v;
  }

  // Set the base data type for the attribute
  result.baseDataType = getBaseDataType(dataType);

  // Read the attribute into a vector of the appropriate type
  if (dataType.getClass() == H5T_STRING) {
    if (dataType.isVariableStr()) {
      // Handle variable-length strings
      std::vector<std::string> stringData;
      std::vector<char*> buffer(numElements);
      attribute.read(dataType, buffer.data());

      for (size_t i = 0; i < numElements; ++i) {
        stringData.emplace_back(buffer[i]);
        free(buffer[i]);  // Free the memory allocated by HDF5
      }
      result.data = stringData;
      result.typeIndex = typeid(std::string);
    } else {
      // Handle fixed-length strings
      result.data = readStringDataHelper(
          attribute, numElements, H5::DataSpace(), dataspace);
      result.typeIndex = typeid(std::string);
    }
  } else if (dataType == H5::PredType::NATIVE_DOUBLE) {
    result.data = readDataHelper<double>(attribute, numElements);
    result.typeIndex = typeid(double);
  } else if (dataType == H5::PredType::NATIVE_FLOAT) {
    result.data = readDataHelper<float>(attribute, numElements);
    result.typeIndex = typeid(float);
  } else if (dataType == H5::PredType::NATIVE_INT32) {
    result.data = readDataHelper<int32_t>(attribute, numElements);
    result.typeIndex = typeid(int32_t);
  } else if (dataType == H5::PredType::NATIVE_UINT32) {
    result.data = readDataHelper<uint32_t>(attribute, numElements);
    result.typeIndex = typeid(uint32_t);
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
  } else if (dataType.getClass() == H5T_ARRAY) {
    // Handle array attributes
    H5::ArrayType arrayType(dataType.getId());
    H5::DataType baseType = arrayType.getSuper();
    SizeType arrayRank = static_cast<SizeType>(arrayType.getArrayNDims());
    std::vector<hsize_t> arrayDims(arrayRank);
    arrayType.getArrayDims(arrayDims.data());

    // Update the shape to reflect the array dimensions
    result.shape.assign(arrayDims.begin(), arrayDims.end());

    size_t arrayNumElements = 1;
    for (const auto dim : arrayDims) {
      arrayNumElements *= dim;
    }

    if (baseType == H5::PredType::NATIVE_INT32) {
      result.data =
          readDataHelper<int32_t>(attribute, numElements * arrayNumElements);
      result.typeIndex = typeid(int32_t);
    } else if (baseType == H5::PredType::NATIVE_UINT32) {
      result.data =
          readDataHelper<uint32_t>(attribute, numElements * arrayNumElements);
      result.typeIndex = typeid(uint32_t);
    } else if (baseType == H5::PredType::NATIVE_FLOAT) {
      result.data =
          readDataHelper<float>(attribute, numElements * arrayNumElements);
      result.typeIndex = typeid(float);
    } else if (baseType == H5::PredType::NATIVE_DOUBLE) {
      result.data =
          readDataHelper<double>(attribute, numElements * arrayNumElements);
      result.typeIndex = typeid(double);
    } else {
      throw std::runtime_error("Unsupported array base data type");
    }
  } else if (dataType.getClass() == H5T_REFERENCE) {
    // Handle object references
    std::vector<hobj_ref_t> refData(numElements);
    attribute.read(dataType, refData.data());
    result.data = refData;
    result.typeIndex = typeid(hobj_ref_t);
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

  // Create the return value to fill
  IO::DataBlockGeneric result;

  // Create new vectors of type hsize_t for stride and block because
  // HDF5 needs hsize_t and we use SizeType in AqNWB instead
  std::vector<hsize_t> stride_hsize(stride.size());
  std::vector<hsize_t> block_hsize(block.size());
  // Copy elements from the original vectors to the new vectors
  std::copy(stride.begin(), stride.end(), stride_hsize.begin());
  std::copy(block.begin(), block.end(), block_hsize.begin());

  // Read the dataset
  H5::DataSet dataset;
  try {
    dataset = m_file->openDataSet(dataPath);
  } catch (const H5::Exception& e) {
    std::cerr << "Failed to open dataset: " << e.getDetailMsg() << std::endl;
    throw std::runtime_error("Failed to open dataset");
  }

  if (dataset.getId() < 0) {
    throw std::runtime_error("Dataset is not valid");
  }

  // Get the dataspace of the dataset
  H5::DataSpace dataspace = dataset.getSpace();

  // Get the number of dimensions and their sizes
  SizeType rank = static_cast<SizeType>(dataspace.getSimpleExtentNdims());
  std::vector<hsize_t> dims(rank);
  dataspace.getSimpleExtentDims(dims.data(), nullptr);

  // Store the shape information
  result.shape.assign(dims.begin(), dims.end());

  // Create a memory dataspace for the slice
  H5::DataSpace memspace;
  if (!start.empty() && !count.empty()) {
    std::vector<hsize_t> offset(rank);
    std::vector<hsize_t> block_count(rank);
    for (SizeType i = 0; i < rank; ++i) {
      offset[i] = start[i];
      block_count[i] = count[i];
      // Check that the offset and block count are within the dimensions
      if (offset[i] + block_count[i] > dims[i]) {
        throw std::runtime_error(
            "Selection + offset for dimension not within extent.");
      }
    }

    dataspace.selectHyperslab(
        H5S_SELECT_SET,
        block_count.data(),
        offset.data(),
        stride_hsize.empty() ? nullptr : stride_hsize.data(),
        block_hsize.empty() ? nullptr : block_hsize.data());

    // Calculate the memory space dimensions
    std::vector<hsize_t> mem_dims(rank);
    for (SizeType i = 0; i < rank; ++i) {
      mem_dims[i] = block_count[i];
      if (!block_hsize.empty()) {
        mem_dims[i] *= block_hsize[i];
      }
    }

    memspace = H5::DataSpace(static_cast<int>(rank), mem_dims.data());

    // Update the shape information based on the hyperslab selection
    result.shape.assign(mem_dims.begin(), mem_dims.end());
  } else {
    memspace = H5::DataSpace(dataspace);
  }

  // Calculate the total number of elements based on the hyperslab selection
  size_t numElements = 1;
  for (const auto& c : result.shape) {
    numElements *= c;
  }

  // Read the dataset into a vector of the appropriate type
  H5::DataType dataType = dataset.getDataType();
  result.baseDataType = getBaseDataType(dataType);
  if (dataType.getClass() == H5T_STRING) {
    // Use readStringDataHelper to read string data
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
  } else if (dataType == H5::PredType::NATIVE_INT8) {
    result.data =
        readDataHelper<int8_t>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(int8_t);
  } else if (dataType == H5::PredType::NATIVE_UINT8) {
    result.data =
        readDataHelper<uint8_t>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(uint8_t);
  } else if (dataType == H5::PredType::NATIVE_INT16) {
    result.data =
        readDataHelper<int16_t>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(int16_t);
  } else if (dataType == H5::PredType::NATIVE_UINT16) {
    result.data =
        readDataHelper<uint16_t>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(uint16_t);
  } else if (dataType == H5::PredType::NATIVE_INT32) {
    result.data =
        readDataHelper<int32_t>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(int32_t);
  } else if (dataType == H5::PredType::NATIVE_UINT32) {
    result.data =
        readDataHelper<uint32_t>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(uint32_t);
  } else if (dataType == H5::PredType::NATIVE_INT64) {
    result.data =
        readDataHelper<int64_t>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(int64_t);
  } else if (dataType == H5::PredType::NATIVE_UINT64) {
    result.data =
        readDataHelper<uint64_t>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(uint64_t);
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
  } else if (dataType == H5::PredType::NATIVE_UCHAR) {
    result.data = readDataHelper<unsigned char>(
        dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(unsigned char);
  } else if (dataType == H5::PredType::NATIVE_USHORT) {
    result.data = readDataHelper<unsigned short>(
        dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(unsigned short);
  } else if (dataType == H5::PredType::NATIVE_CHAR) {
    result.data =
        readDataHelper<char>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(char);
  } else if (dataType == H5::PredType::NATIVE_SHORT) {
    result.data =
        readDataHelper<short>(dataset, numElements, memspace, dataspace);
    result.typeIndex = typeid(short);
  } else {
    throw std::runtime_error("Unsupported data type");
  }
  // Return the result
  return result;
}

Status HDF5IO::createAttribute(const IO::BaseDataType& type,
                               const void* data,
                               const std::string& path,
                               const std::string& name,
                               const SizeType& size)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }

  DataType H5type = getH5Type(type);
  DataType origType = getNativeType(type);

  DataSpace attr_dataspace;
  if (size > 1) {
    hsize_t dims = static_cast<hsize_t>(size);
    attr_dataspace = DataSpace(1, &dims);  // Create 1D dataspace of size 'dims'
  } else {
    attr_dataspace = DataSpace(H5S_SCALAR);
  }

  auto manage_attribute = [&](H5Object& loc)
  {
    Attribute attr = loc.attrExists(name)
        ? loc.openAttribute(name)
        : loc.createAttribute(name, H5type, attr_dataspace);
    attr.write(origType, data);
    return Status::Success;
  };

  // open the group or dataset
  H5O_type_t objectType = getH5ObjectType(path);
  if (objectType == H5O_TYPE_GROUP) {
    Group gloc = m_file->openGroup(path);
    return manage_attribute(gloc);
  } else if (objectType == H5O_TYPE_DATASET) {
    DataSet dloc = m_file->openDataSet(path);
    return manage_attribute(dloc);
  }

  return Status::Failure;  // not a valid dataset or group type
}

Status HDF5IO::createAttribute(const std::string& data,
                               const std::string& path,
                               const std::string& name,
                               const bool overwrite)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }

  // Create variable length string type
  StrType H5type(PredType::C_S1, static_cast<size_t>(H5T_VARIABLE));

  auto manage_attribute = [&](H5Object& loc)
  {
    try {
      if (loc.attrExists(name)) {
        if (overwrite) {
          loc.removeAttr(name);
        } else {
          return Status::Failure;  // Don't allow overwriting
        }
      }

      // Create dataspace for scalar
      DataSpace attr_dataspace(H5S_SCALAR);

      // Create the attribute
      Attribute attr = loc.createAttribute(name, H5type, attr_dataspace);

      // Write the scalar string data
      const char* dataPtr = data.c_str();
      attr.write(H5type, &dataPtr);

    } catch (const GroupIException& error) {
      error.printErrorStack();
      return Status::Failure;
    } catch (const AttributeIException& error) {
      error.printErrorStack();
      return Status::Failure;
    } catch (const FileIException& error) {
      error.printErrorStack();
      return Status::Failure;
    } catch (const DataSetIException& error) {
      error.printErrorStack();
      return Status::Failure;
    }
    return Status::Success;
  };

  // Open the group or dataset
  H5O_type_t objectType = getH5ObjectType(path);
  if (objectType == H5O_TYPE_GROUP) {
    Group gloc = m_file->openGroup(path);
    return manage_attribute(gloc);
  } else if (objectType == H5O_TYPE_DATASET) {
    DataSet dloc = m_file->openDataSet(path);
    return manage_attribute(dloc);
  }

  return Status::Failure;  // Not a valid dataset or group type
}

Status HDF5IO::createAttribute(const std::vector<std::string>& data,
                               const std::string& path,
                               const std::string& name,
                               const bool overwrite)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }

  // Create variable length string type
  StrType H5type(PredType::C_S1, static_cast<size_t>(H5T_VARIABLE));

  auto manage_attribute = [&](H5Object& loc)
  {
    try {
      if (loc.attrExists(name)) {
        if (overwrite) {
          // Delete the existing attribute
          loc.removeAttr(name);
        } else {
          return Status::Failure;  // don't allow overwriting
        }
      }

      // Create dataspace based on number of strings
      hsize_t dims[1];
      dims[0] = data.size();
      DataSpace attr_dataspace(1, dims);

      // Create the attribute
      Attribute attr = loc.createAttribute(name, H5type, attr_dataspace);

      // Write the data directly from the vector of strings
      std::vector<const char*> dataPtrs;
      dataPtrs.reserve(data.size());
      for (const auto& str : data) {
        dataPtrs.push_back(str.c_str());
      }
      attr.write(H5type, dataPtrs.data());

    } catch (const GroupIException& error) {
      error.printErrorStack();
      return Status::Failure;
    } catch (const AttributeIException& error) {
      error.printErrorStack();
      return Status::Failure;
    } catch (const FileIException& error) {
      error.printErrorStack();
      return Status::Failure;
    } catch (const DataSetIException& error) {
      error.printErrorStack();
      return Status::Failure;
    }
    return Status::Success;
  };

  // open the group or dataset
  H5O_type_t objectType = getH5ObjectType(path);
  if (objectType == H5O_TYPE_GROUP) {
    Group gloc = m_file->openGroup(path);
    return manage_attribute(gloc);
  } else if (objectType == H5O_TYPE_DATASET) {
    DataSet dloc = m_file->openDataSet(path);
    return manage_attribute(dloc);
  }

  return Status::Failure;  // not a valid dataset or group type
}

Status HDF5IO::createReferenceAttribute(const std::string& referencePath,
                                        const std::string& path,
                                        const std::string& name)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }

  auto manage_attribute = [&](H5Object& loc)
  {
    try {
      Attribute attr = loc.attrExists(name)
          ? loc.openAttribute(name)
          : loc.createAttribute(
                name, H5::PredType::STD_REF_OBJ, DataSpace(H5S_SCALAR));

      hobj_ref_t rdata;
      m_file->reference(&rdata, referencePath.c_str());

      attr.write(H5::PredType::STD_REF_OBJ, &rdata);
    } catch (const H5::Exception& error) {
      error.printErrorStack();
      return Status::Failure;
    }
    return Status::Success;
  };

  // open the group or dataset
  H5O_type_t objectType = getH5ObjectType(path);
  if (objectType == H5O_TYPE_GROUP) {
    Group gloc = m_file->openGroup(path);
    return manage_attribute(gloc);
  } else if (objectType == H5O_TYPE_DATASET) {
    DataSet dloc = m_file->openDataSet(path);
    return manage_attribute(dloc);
  }

  return Status::Failure;  // not a valid dataset or group type
}

Status HDF5IO::createGroup(const std::string& path)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }

  try {
    m_file->createGroup(path);
  } catch (const FileIException& error) {
    error.printErrorStack();
    return Status::Failure;
  } catch (const GroupIException& error) {
    error.printErrorStack();
    return Status::Failure;
  }
  return Status::Success;
}

Status HDF5IO::createGroupIfDoesNotExist(const std::string& path)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }
  try {
    m_file->childObjType(path);
  } catch (const FileIException&) {
    return createGroup(path);
  }
  return Status::Success;
}

/** Creates a link to another location in the file */
Status HDF5IO::createLink(const std::string& path, const std::string& reference)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }

  herr_t error = H5Lcreate_soft(reference.c_str(),
                                m_file->getLocId(),
                                path.c_str(),
                                H5P_DEFAULT,
                                H5P_DEFAULT);

  return intToStatus(error);
}

Status HDF5IO::createReferenceDataSet(
    const std::string& path, const std::vector<std::string>& references)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }

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

  herr_t writeStatus = H5Dwrite(dset,
                                H5T_STD_REF_OBJ,
                                H5S_ALL,
                                H5S_ALL,
                                H5P_DEFAULT,
                                static_cast<const void*>(rdata));

  delete[] rdata;

  herr_t dsetStatus = H5Dclose(dset);
  if (intToStatus(dsetStatus) == Status::Failure) {
    return Status::Failure;
  }

  herr_t spaceStatus = H5Sclose(space);
  if (intToStatus(spaceStatus) == Status::Failure) {
    return Status::Failure;
  }

  return intToStatus(writeStatus);
}

Status HDF5IO::createStringDataSet(const std::string& path,
                                   const std::string& value)
{
  if (!canModifyObjects()) {
    return Status::Failure;
  }

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
  if (!canModifyObjects()) {
    return Status::Failure;
  }

  std::unique_ptr<IO::BaseRecordingData> dataset;
  IO::ArrayDataSetConfig config(
      IO::BaseDataType::V_STR, SizeArray {values.size()}, SizeArray {1});
  dataset =
      std::unique_ptr<IO::BaseRecordingData>(createArrayDataSet(config, path));

  dataset->writeDataBlock(std::vector<SizeType> {1},
                          std::vector<SizeType> {0},
                          IO::BaseDataType::V_STR,
                          values);

  return Status::Success;
}

Status HDF5IO::startRecording()
{
  if (!m_opened) {
    return Status::Failure;
  }
  // Call the base class method to pre-finalize all recording objects
  Status status = BaseIO::startRecording();
  // Start SWMR mode if it is not disabled
  if (!m_disableSWMRMode) {
    herr_t swmr_status = H5Fstart_swmr_write(m_file->getId());
    status = status && intToStatus(swmr_status);
  }
  return status;
}

Status HDF5IO::stopRecording()
{
  // Call the base class method to finalize all recording objects
  Status baseStatus = BaseIO::stopRecording();

  // if SWMR mode is disabled, stopping the recording will leave the file open
  if (!m_disableSWMRMode) {
    close();  // SWMR mode cannot be disabled so close the file
  } else {
    this->flush();
  }

  return baseStatus;
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

bool HDF5IO::objectExists(const std::string& path) const
{
  htri_t exists = H5Lexists(m_file->getId(), path.c_str(), H5P_DEFAULT);
  if (exists > 0) {
    return true;
  } else {
    return false;
  }
}

bool HDF5IO::attributeExists(const std::string& path) const
{
  auto attributePtr = this->getAttribute(path);
  return (attributePtr != nullptr);
}

std::vector<std::pair<std::string, StorageObjectType>>
HDF5IO::getStorageObjects(const std::string& path,
                          const StorageObjectType& objectType) const

{
  std::vector<std::pair<std::string, StorageObjectType>> objects;

  H5O_type_t h5Type = getH5ObjectType(path);
  if (h5Type == H5O_TYPE_GROUP) {
    H5::Group group = m_file->openGroup(path);
    hsize_t num_objs = group.getNumObjs();
    for (hsize_t i = 0; i < num_objs; ++i) {
      std::string objName = group.getObjnameByIdx(i);
      H5G_obj_t objType = group.getObjTypeByIdx(i);
      StorageObjectType storageObjectType;
      switch (objType) {
        case H5G_GROUP:
          storageObjectType = StorageObjectType::Group;
          break;
        case H5G_DATASET:
          storageObjectType = StorageObjectType::Dataset;
          break;
        default:
          storageObjectType = StorageObjectType::Undefined;
      }
      if (storageObjectType == objectType
          || objectType == StorageObjectType::Undefined)
      {
        objects.emplace_back(objName, storageObjectType);
      }
    }

    // Include attributes for groups
    if (objectType == StorageObjectType::Attribute
        || objectType == StorageObjectType::Undefined)
    {
      unsigned int numAttrs = static_cast<unsigned int>(group.getNumAttrs());
      for (unsigned int i = 0; i < numAttrs; ++i) {
        H5::Attribute attr = group.openAttribute(i);
        objects.emplace_back(attr.getName(), StorageObjectType::Attribute);
      }
    }
  } else if (h5Type == H5O_TYPE_DATASET) {
    if (objectType == StorageObjectType::Attribute
        || objectType == StorageObjectType::Undefined)
    {
      H5::DataSet dataset = m_file->openDataSet(path);
      unsigned int numAttrs = static_cast<unsigned int>(dataset.getNumAttrs());
      for (unsigned int i = 0; i < numAttrs; ++i) {
        H5::Attribute attr = dataset.openAttribute(i);
        objects.emplace_back(attr.getName(), StorageObjectType::Attribute);
      }
    }
  }

  return objects;
}

SizeArray HDF5IO::getStorageObjectShape(const std::string path) const
{
  H5::DataSpace dataspace;
  try {
    H5::DataSet dataset = m_file->openDataSet(path);
    dataspace = dataset.getSpace();
  } catch (H5::Exception&) {
    // Read the attribute
    std::unique_ptr<H5::Attribute> attributePtr = this->getAttribute(path);
    dataspace = attributePtr->getSpace();
  }
  const int rank = dataspace.getSimpleExtentNdims();
  std::vector<hsize_t> dims(static_cast<size_t>(rank));
  dataspace.getSimpleExtentDims(dims.data());

  return SizeArray(dims.begin(), dims.end());
}

SizeArray HDF5IO::getStorageObjectChunking(const std::string path) const
{
  // First check what type of object we're dealing with
  StorageObjectType objectType = getStorageObjectType(path);

  // Only datasets can have chunking - return empty for groups and attributes
  if (objectType != StorageObjectType::Dataset) {
    return SizeArray();
  }

  try {
    H5::DataSet dataset = m_file->openDataSet(path);
    H5::DSetCreatPropList plist = dataset.getCreatePlist();

    // Check if the dataset is chunked
    if (plist.getLayout() != H5D_CHUNKED) {
      return SizeArray();
    }

    // Get the chunk dimensions
    int rank = dataset.getSpace().getSimpleExtentNdims();
    std::vector<hsize_t> chunk_dims(static_cast<size_t>(rank));
    plist.getChunk(rank, chunk_dims.data());

    return SizeArray(chunk_dims.begin(), chunk_dims.end());
  } catch (H5::Exception& e) {
    std::cerr << "HDF5IO::getStorageObjectChunking: Could not get chunking for "
                 "dataset at "
              << path << ": " << e.getDetailMsg() << std::endl;
    return SizeArray();
  }
}

AQNWB::IO::BaseDataType HDF5IO::getStorageObjectDataType(
    const std::string path) const
{
  // Check if the object is a dataset
  StorageObjectType objType = getStorageObjectType(path);
  if (objType != StorageObjectType::Dataset) {
    std::string typeStr;
    switch (objType) {
      case StorageObjectType::Group:
        typeStr = "Group";
        break;
      case StorageObjectType::Attribute:
        typeStr = "Attribute";
        break;
      default:
        typeStr = "Unknown";
        break;
    }
    throw std::runtime_error(
        "HDF5IO::getStorageObjectDataType: Object at '" + path +
        "' is a " + typeStr + ", not a dataset. Cannot determine data type.");
  }

  try {
    H5::DataSet dataset = m_file->openDataSet(path);
    H5::DataType dataType = dataset.getDataType();
    return getBaseDataType(dataType);
  } catch (H5::Exception& e) {
    throw std::runtime_error(
        "HDF5IO::getStorageObjectDataType: Could not get data type for "
        "dataset at '" +
        path + "': " + e.getDetailMsg());
  }
}

std::shared_ptr<AQNWB::IO::BaseRecordingData> HDF5IO::getDataSet(
    const std::string& path)
{
  std::unique_ptr<DataSet> data;

  if (!m_opened)
    return nullptr;

  try {
    data = std::make_unique<H5::DataSet>(m_file->openDataSet(path));
    return std::make_shared<HDF5RecordingData>(std::move(data));
  } catch (const DataSetIException& error) {
    error.printErrorStack();
    return nullptr;
  } catch (const FileIException& error) {
    error.printErrorStack();
    return nullptr;
  } catch (const DataSpaceIException& error) {
    error.printErrorStack();
    return nullptr;
  }
}

std::unique_ptr<AQNWB::IO::BaseRecordingData> HDF5IO::createArrayDataSet(
    const IO::BaseArrayDataSetConfig& config, const std::string& path)
{
  if (!canModifyObjects()) {
    std::cerr << "Cannot modify objects" << std::endl;
    return nullptr;
  }

  // Check if this is a link configuration
  if (config.isLink()) {
    const IO::LinkArrayDataSetConfig* linkConfig =
        dynamic_cast<const IO::LinkArrayDataSetConfig*>(&config);
    if (linkConfig) {
      Status status = createLink(path, linkConfig->getTargetPath());
      if (status != Status::Success) {
        std::cerr << "Failed to create link from " << path << " to "
                  << linkConfig->getTargetPath() << std::endl;
      }
      // Return nullptr for links as they don't provide a recordable dataset
      return nullptr;
    }
  }

  // Regular dataset creation
  const IO::ArrayDataSetConfig* arrayConfig =
      dynamic_cast<const IO::ArrayDataSetConfig*>(&config);
  if (!arrayConfig) {
    std::cerr << "Invalid configuration type for dataset creation" << std::endl;
    return nullptr;
  }

  std::unique_ptr<DataSet> data;
  DSetCreatPropList prop;
  DataType H5type = getH5Type(arrayConfig->getType());

  const SizeArray& size = arrayConfig->getShape();
  const SizeArray& chunking = arrayConfig->getChunking();

  SizeType dimension = size.size();
  if (dimension < 1) {
    std::cerr << "Invalid dimension size" << std::endl;
    return nullptr;
  }

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

  try {
    DataSpace dSpace(static_cast<int>(dimension), dims.data(), max_dims.data());
    prop.setChunk(static_cast<int>(dimension), chunk_dims.data());

    // Apply filters if HDF5ArrayDataSetConfig is used
    const HDF5ArrayDataSetConfig* hdf5Config =
        dynamic_cast<const HDF5ArrayDataSetConfig*>(&config);
    if (hdf5Config) {
      for (const auto& filter : hdf5Config->getFilters()) {
        prop.setFilter(filter.filter_id,
                       0,
                       static_cast<size_t>(filter.cd_values.size()),
                       filter.cd_values.data());
      }
    }

    if (arrayConfig->getType().type == IO::BaseDataType::Type::T_STR) {
      H5type = StrType(PredType::C_S1, arrayConfig->getType().typeSize);
    }

    data = std::make_unique<DataSet>(
        m_file->createDataSet(path, H5type, dSpace, prop));
  } catch (const H5::Exception& e) {
    std::cerr << "HDF5 error: " << e.getDetailMsg() << std::endl;
    return nullptr;
  }

  return std::make_unique<HDF5RecordingData>(std::move(data));
}

H5O_type_t HDF5IO::getH5ObjectType(const std::string& path) const
{
  H5O_info_t objInfo;  // Structure to hold information about the object
  herr_t status;

#if H5_VERSION_GE(1, 12, 0)
  status = H5Oget_info_by_name(
      m_file->getId(), path.c_str(), &objInfo, H5O_INFO_BASIC, H5P_DEFAULT);
#else
  status =
      H5Oget_info_by_name(m_file->getId(), path.c_str(), &objInfo, H5P_DEFAULT);
#endif

  // Check if the object exists
  if (status < 0) {
    // Return a special value indicating the object does not exist
    return H5O_TYPE_UNKNOWN;
  }

  // Get the object type
  H5O_type_t objectType = objInfo.type;

  return objectType;
}

H5::DataType HDF5IO::getNativeType(IO::BaseDataType type)
{
  H5::DataType baseType;

  switch (type.type) {
    case IO::BaseDataType::Type::T_I8:
      baseType = H5::PredType::NATIVE_INT8;
      break;
    case IO::BaseDataType::Type::T_I16:
      baseType = H5::PredType::NATIVE_INT16;
      break;
    case IO::BaseDataType::Type::T_I32:
      baseType = H5::PredType::NATIVE_INT32;
      break;
    case IO::BaseDataType::Type::T_I64:
      baseType = H5::PredType::NATIVE_INT64;
      break;
    case IO::BaseDataType::Type::T_U8:
      baseType = H5::PredType::NATIVE_UINT8;
      break;
    case IO::BaseDataType::Type::T_U16:
      baseType = H5::PredType::NATIVE_UINT16;
      break;
    case IO::BaseDataType::Type::T_U32:
      baseType = H5::PredType::NATIVE_UINT32;
      break;
    case IO::BaseDataType::Type::T_U64:
      baseType = H5::PredType::NATIVE_UINT64;
      break;
    case IO::BaseDataType::Type::T_F32:
      baseType = H5::PredType::NATIVE_FLOAT;
      break;
    case IO::BaseDataType::Type::T_F64:
      baseType = H5::PredType::NATIVE_DOUBLE;
      break;
    case IO::BaseDataType::Type::T_STR:
      return H5::StrType(H5::PredType::C_S1, type.typeSize);
    case IO::BaseDataType::Type::V_STR:
      return H5::StrType(H5::PredType::C_S1, static_cast<size_t>(H5T_VARIABLE));
    default:
      baseType = H5::PredType::NATIVE_INT32;
  }

  if (type.typeSize > 1) {
    hsize_t size = type.typeSize;
    return H5::ArrayType(baseType, 1, &size);
  } else {
    return baseType;
  }
}

AQNWB::IO::BaseDataType HDF5IO::getBaseDataType(const H5::DataType& nativeType)
{
  if (nativeType == H5::PredType::NATIVE_INT8) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_I8);
  } else if (nativeType == H5::PredType::NATIVE_INT16) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_I16);
  } else if (nativeType == H5::PredType::NATIVE_INT32) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_I32);
  } else if (nativeType == H5::PredType::NATIVE_INT64) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_I64);
  } else if (nativeType == H5::PredType::NATIVE_UINT8) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_U8);
  } else if (nativeType == H5::PredType::NATIVE_UINT16) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_U16);
  } else if (nativeType == H5::PredType::NATIVE_UINT32) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_U32);
  } else if (nativeType == H5::PredType::NATIVE_UINT64) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_U64);
  } else if (nativeType == H5::PredType::NATIVE_FLOAT) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_F32);
  } else if (nativeType == H5::PredType::NATIVE_DOUBLE) {
    return IO::BaseDataType(IO::BaseDataType::Type::T_F64);
  } else if (nativeType.getClass() == H5T_STRING) {
    if (nativeType.isVariableStr()) {
      return IO::BaseDataType(IO::BaseDataType::Type::V_STR);
    } else {
      // Fixed length string: get the size
      size_t size = nativeType.getSize();
      return IO::BaseDataType(IO::BaseDataType::Type::T_STR, size);
    }
  } else if (nativeType.getClass() == H5T_ARRAY) {
    // Array type: get the base type and size
    hid_t nativeTypeId = nativeType.getId();
    H5::ArrayType arrayType(nativeTypeId);
    H5::DataType baseType = arrayType.getSuper();
    hsize_t size = static_cast<hsize_t>(arrayType.getArrayNDims());
    return IO::BaseDataType(getBaseDataType(baseType).type, size);
  } else {
    // Default case: return a 32-bit integer type
    return IO::BaseDataType(IO::BaseDataType::Type::T_I32);
  }
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
      return StrType(PredType::C_S1, static_cast<size_t>(H5T_VARIABLE));
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
