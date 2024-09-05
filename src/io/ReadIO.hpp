#pragma once

#include <any>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include <boost/multi_array.hpp>  // TODO move this and function def to the cpp file

#include "Types.hpp"

#define DEFAULT_STR_SIZE 256
#define DEFAULT_ARRAY_SIZE 1

using StorageObjectType = AQNWB::Types::StorageObjectType;
using Status = AQNWB::Types::Status;
using SizeArray = AQNWB::Types::SizeArray;
using SizeType = AQNWB::Types::SizeType;

/*!
 * \namespace AQNWB
 * \brief The main namespace for AqNWB
 */
namespace AQNWB::IO
{


/**
 * @brief Generic structure to hold type-erased data and shape
 */
class DataBlockGeneric
{
public:
  /**
   * @ brief The untyped data values.
   *
   * We know this will be a 1-dimensional std::vector of some kind,
   * so we can cast it via
   * ``std::any_cast<std::vector<DTYPE>>(genericDataBlock.data)``
   */
  std::any data;
  /**
   * \brief The 1D vector with the n-dimensional shape of the data.
   *        Set to empty in case of scalar data.
   */
  std::vector<SizeType> shape;
  /**
   * \brief Type index of the values stored in the data vector.
   *
   * I.e. if data is actually a ``std::vector<float>`` then this should be
   * set to ``typeid(float)`` and not ``typeid(std::vector<float>)``.
   * The default value is ``typeid(void)`` to indicate that the data type is
   * unknown. However, the data type should usually be determined by the
   * I/O backend when loading data so this should usually be set to the
   * correct type.
   */
  std::type_index typeIndex = typeid(void);
};

/**
 * @brief Structure to hold data and shape for a typed data vector
 *
 * @tparam DTYPE The data type of the vector
 */
template<typename DTYPE>
class DataBlock
{
public:
  /**
   * @ brief The 1D vector with the data values of type DTYPE
   */
  std::vector<DTYPE> data;
  /**
   * \brief The 1D vector with the n-dimensional shape of the data.
   *        Set to empty in case of scalar data
   */
  std::vector<SizeType> shape;
  /**
   * \brief Type index of the values stored in the data vector.
   *        Here this is fixed to ``typeid(DTYPE)``
   */
  const std::type_index typeIndex = typeid(DTYPE);

  /// Constructor
  DataBlock(const std::vector<DTYPE>& data, const std::vector<SizeType>& shape)
      : data(data)
      , shape(shape)
  {
  }

  /**
   * \brief Transform the data to a boost multi-dimensional array for convenient
   * access
   *
   * The function uses boost::const_multi_array_ref to avoid copying of the data
   *
   * @tparam NDIMS The number of dimensions of the array. Same as shape.size()
   */
  template<std::size_t NDIMS>
  boost::const_multi_array_ref<DTYPE, NDIMS> as_multi_array() const
  {
    if (shape.size() != NDIMS) {
      throw std::invalid_argument(
          "Shape size does not match the number of dimensions.");
    }

    // Calculate the total number of elements expected
    SizeType expected_size = 1;
    for (SizeType dim : shape) {
      expected_size *= dim;
    }

    if (data.size() != expected_size) {
      throw std::invalid_argument("Data size does not match the shape.");
    }

    // Convert the shape vector to a boost::array
    boost::array<std::size_t, NDIMS> boost_shape;
    std::copy(shape.begin(), shape.end(), boost_shape.begin());

    // Construct and return the boost::const_multi_array_ref
    return boost::const_multi_array_ref<DTYPE, NDIMS>(data.data(), boost_shape);
  }

  /**
   * @brief Factory method to create an DataBlock from a DataBlockGeneric.
   *
   * The function using std::any_cast to avoid copying the data
   *
   * @param genericData The DataBlockGeneric structure containing the data and
   * shape.
   *
   * @return A DataBlock structure containing the data and shape.
   */
  inline static DataBlock<DTYPE> fromGeneric(
      const DataBlockGeneric& genericData)
  {
    auto result = DataBlock<DTYPE>(
        std::any_cast<std::vector<DTYPE>>(genericData.data), genericData.shape);
    return result;
  }
};



/**
 * @brief The base class for wrapping data objects for reading data from a file
 */
template<typename VTYPE>
class ReadDataWrapperBase
{
public:
  /**
   * @brief Default constructor.
   */
  ReadDataWrapperBase(const std::shared_ptr<IO::BaseIO> io, std::string dataPath)
      : io(io)
      , dataPath(dataPath)
  {
  }

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  ReadDataWrapperBase(const ReadDataWrapperBase&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  ReadDataWrapperBase& operator=(const ReadDataWrapperBase&) = delete;

  /**
   * @brief Destructor.
   */
  virtual ~ReadDataWrapperBase() {}

  /**
   * @brief Reads an attribute or dataset and determines the data type to
   * allocate the memory for all data values
   *
   * @return An DataBlockGeneric structure containing the full data and shape.
   */
  virtual DataBlockGeneric valuesGeneric() = 0;

protected:
  /**
   * @brief Pointer to the I/O object to use for reading.
   */
  const std::shared_ptr<IO::BaseIO> io;  // BaseIO* io;
  /**
   * @brief Path to the dataset or attribute to read
   */
  std::string dataPath;
};

/**
 * @brief Wrapper class for lazily reading data from a dataset in a file
 */
template<typename VTYPE>
class ReadDatasetWrapper : public ReadDataWrapperBase<VTYPE>
{
public:
  /**
   * @brief Default constructor.
   */
  ReadDatasetWrapper(const std::shared_ptr<IO::BaseIO> io, std::string dataPath)
      : ReadDataWrapperBase<VTYPE>(io, dataPath)
  {
  }

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  ReadDatasetWrapper(const ReadDatasetWrapper&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  ReadDatasetWrapper& operator=(const ReadDatasetWrapper&) = delete;

  /**
   * @brief Destructor.
   */
  virtual ~ReadDatasetWrapper() {}

  /**
   * @brief Reads a dataset and determines the data type.
   *
   * This functions calls the overloaded valuesGeneric({}, {}, {}, {}) variant
   *
   * @return An DataBlockGeneric structure containing the data and shape.
   */
  DataBlockGeneric valuesGeneric() override
  {
    return this->valuesGeneric({}, {}, {}, {});
  }

  /**
   * @brief Reads a dataset and determines the data type.
   *
   * @param start The starting indices for the slice (required).
   * @param count The number of elements to read for each dimension (optional).
   * @param stride The stride for each dimension (optional).
   * @param block The block size for each dimension (optional).
   *
   * @return An DataBlockGeneric structure containing the data and shape.
   */
  DataBlockGeneric valuesGeneric(const std::vector<SizeType>& start,
                                 const std::vector<SizeType>& count = {},
                                 const std::vector<SizeType>& stride = {},
                                 const std::vector<SizeType>& block = {})
  {
    return this->io->readDataset(this->dataPath, start, count, stride, block);
  }

  /**
   * @brief Reads an dataset with a specified data type.
   *
   * This convenience function uses valuesGeneric to read the data and then
   * convert the DataBlockGeneric to a specific DataBlock
   *
   * @param start The starting indices for the slice (optional).
   * @param count The number of elements to read for each dimension (optional).
   * @param stride The stride for each dimension (optional).
   * @param block The block size for each dimension (optional).
   *
   * @return A DataBlock structure containing the data and shape.
   */
  DataBlock<VTYPE> values(const std::vector<SizeType>& start = {},
                          const std::vector<SizeType>& count = {},
                          const std::vector<SizeType>& stride = {},
                          const std::vector<SizeType>& block = {})
  {
    return DataBlock<VTYPE>::fromGeneric(
        this->valuesGeneric(start, count, stride, block));
  }

};  // ReadDatasetWrapper

/**
 * @brief Wrapper class for lazily reading data from an attribute in a file
 */
template<typename VTYPE>
class ReadAttributeWrapper : public ReadDataWrapperBase<VTYPE>
{
public:
  /**
   * @brief Default constructor.
   */
  ReadAttributeWrapper(const std::shared_ptr<IO::BaseIO> io, std::string dataPath)
      : ReadDataWrapperBase<VTYPE>(io, dataPath)
  {
  }

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  ReadAttributeWrapper(const ReadAttributeWrapper&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  ReadAttributeWrapper& operator=(const ReadAttributeWrapper&) = delete;

  /**
   * @brief Destructor.
   */
  virtual ~ReadAttributeWrapper() {}

  /**
   * @brief Reads an attribute and determines the data type.
   *
   * @return An DataBlockGeneric structure containing the data and shape.
   */
  DataBlockGeneric valuesGeneric() override
  {
    return this->io->readAttribute(this->dataPath);
  }

  /**
   * @brief Reads an attribute with a specified data type.
   *
   * This convenience function uses valuesGeneric to read the data and then
   * convert the DataBlockGeneric to a specific DataBlock
   *
   * @return A DataBlock structure containing the data and shape.
   */
  DataBlock<VTYPE> values()
  {
    return DataBlock<VTYPE>::fromGeneric(this->valuesGeneric());
  }

};  // ReadAttributeWrapper

}  // namespace AQNWB::IO
