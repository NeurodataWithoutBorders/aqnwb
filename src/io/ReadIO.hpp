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

using StorageObjectType = AQNWB::Types::StorageObjectType;
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
  inline boost::const_multi_array_ref<DTYPE, NDIMS> as_multi_array() const
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
 * @brief Class for wrapping data objects (datasets or attributes) for reading
 * data from a file
 *
 * @tparam OTYPE The type of object being wrapped defined via \ref
 * AQNWB::Types::StorageObjectType
 * @tparam VTYPE The data type of the values stored in the data object
 */
template<StorageObjectType OTYPE, typename VTYPE>
class ReadDataWrapper
{
  // Embedded traits for compile time checking of allowed OTYPE for the class
  // and methods
private:
  /**
   * Embedded Trait to Check the OTYPE Enum Value at compile time to
   * SFINAE (Substitution Failure Is Not An Error) approach to disable
   * select functions for attributes, to not support slicing.
   */
  template<StorageObjectType U>
  struct is_dataset
      : std::integral_constant<bool, (U == StorageObjectType::Dataset)>
  {
  };

  /// Helper struct to check if a StorageObjectType is allowed. Used in static
  /// assert.
  template<StorageObjectType T>
  struct is_allowed_storage_object_type : std::false_type
  {
  };

  /// Helper struct to check if a StorageObjectType is allowed. Used in static
  /// assert.
  template<>
  struct is_allowed_storage_object_type<StorageObjectType::Dataset>
      : std::true_type
  {
  };

  /// Helper struct to check if a StorageObjectType is allowed. Used in static
  /// assert.
  template<>
  struct is_allowed_storage_object_type<StorageObjectType::Attribute>
      : std::true_type
  {
  };

  /**
   *  Static assert to enforce the restriction that ReadDataWrapper can only be
   *  instantiated for OTYPE of StorageObjectType::Dataset and
   *  StorageObjectType::Attribute but not for other types, e.g., Group or
   * Undefined.
   */
  static_assert(is_allowed_storage_object_type<OTYPE>::value,
                "StorageObjectType not allowed for ReadDataWrapper");

  // Actual definition of the class
public:
  /**
   * @brief Default constructor.
   */
  ReadDataWrapper(const std::shared_ptr<IO::BaseIO> io, std::string dataPath)
      : io(io)
      , dataPath(dataPath)
  {
  }

  /**
   * @brief Function to return the \ref AQNWB::Types::StorageObjectType OTYPE of
   * the instance
   */
  inline StorageObjectType getStorageObjectType() const { return OTYPE; }

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  ReadDataWrapper(const ReadDataWrapper&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  ReadDataWrapper& operator=(const ReadDataWrapper&) = delete;

  /**
   * @brief Destructor.
   */
  virtual ~ReadDataWrapper() {}

  /**
   * @brief Reads a dataset and determines the data type.
   *
   * This functions calls the overloaded valuesGeneric({}, {}, {}, {}) variant
   *
   * @return An DataBlockGeneric structure containing the data and shape.
   */
  inline DataBlockGeneric valuesGeneric() const
  {
    switch (OTYPE) {
      case StorageObjectType::Dataset: {
        return this->io->readDataset(this->dataPath);
      }
      case StorageObjectType::Attribute: {
        return this->io->readAttribute(this->dataPath);
      }
      default: {
        throw std::runtime_error("Unsupported StorageObjectType");
      }
    }
  }

  /**
   * @brief Reads a dataset and determines the data type.
   *
   * We do not support slicing for attributes, so this function is disabled for
   * attributes. For attributes we should only use the valuesGeneric() method
   * without arguments.
   *
   * @param start The starting indices for the slice (required).
   * @param count The number of elements to read for each dimension (optional).
   * @param stride The stride for each dimension (optional).
   * @param block The block size for each dimension (optional).
   *
   * @return An DataBlockGeneric structure containing the data and shape.
   */
  template<StorageObjectType U = OTYPE,
           typename std::enable_if<is_dataset<U>::value, int>::type = 0>
  inline DataBlockGeneric valuesGeneric(
      const std::vector<SizeType>& start,
      const std::vector<SizeType>& count = {},
      const std::vector<SizeType>& stride = {},
      const std::vector<SizeType>& block = {}) const
  {
    // The function is only enabled for datasets so we don't need to check
    // for attributes here.
    return this->io->readDataset(this->dataPath, start, count, stride, block);
  }

  /**
   * @brief Reads an attribute with a specified data type.
   *
   * This convenience function uses valuesGeneric to read the data and then
   * convert the DataBlockGeneric to a specific DataBlock
   *
   * @tparam T the value type to use. By default this is set to the VTYPE
   *           of the object but is added here to allow the user to
   *           request a different type if approbriate, e.g., if the
   *           object uses VTYPE=std::any and the user knows the type
   *           VTYPE=float
   *
   * @return A DataBlock structure containing the data and shape.
   */
  template<typename T = VTYPE>
  inline DataBlock<VTYPE> values() const
  {
    return DataBlock<T>::fromGeneric(this->valuesGeneric());
  }

  /**
   * @brief Reads an dataset with a specified data type.
   *
   * This convenience function uses valuesGeneric to read the data and then
   * convert the DataBlockGeneric to a specific DataBlock/
   *
   * We do not support slicing for attributes, so this function is disabled for
   * attributes. For attributes we should only use the valuesGeneric() method
   * without arguments.
   *
   * @tparam T the value type to use. By default this is set to the VTYPE
   *           of the object but is added here to allow the user to
   *           request a different type if approbriate, e.g., if the
   *           object uses VTYPE=std::any and the user knows the type
   *           VTYPE=float
   *
   * @param start The starting indices for the slice (optional).
   * @param count The number of elements to read for each dimension (optional).
   * @param stride The stride for each dimension (optional).
   * @param block The block size for each dimension (optional).
   *
   * @return A DataBlock structure containing the data and shape.
   */
  template<typename T = VTYPE,
           StorageObjectType U = OTYPE,
           typename std::enable_if<is_dataset<U>::value, int>::type = 0>
  inline DataBlock<VTYPE> values(const std::vector<SizeType>& start,
                                 const std::vector<SizeType>& count = {},
                                 const std::vector<SizeType>& stride = {},
                                 const std::vector<SizeType>& block = {}) const
  {
    // The function is only enabled for datasets so we don't need to check
    // for attributes here.
    return DataBlock<VTYPE>::fromGeneric(
        this->valuesGeneric(start, count, stride, block));
  }

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

}  // namespace AQNWB::IO
