#pragma once

#include <any>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <variant>
#include <vector>

#include "BaseIO.hpp"
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
  SizeArray shape;

  /**
   * \brief Type index of the values stored in the data vector.
   *
   * I.e. if data is actually a ``std::vector<float>`` then this should be
   * set to ``typeid(float)`` and not ``typeid(std::vector<float>)``.
   * The default value is ``typeid(void)`` to indicate that the data type is
   * unknown. However, the data type should usually be determined by the
   * I/O backend when loading data so this should usually be set to the
   * correct type.
   *
   * \note
   * Using  ``std::type_index`` allows inspection of types at runtime, e.g.,
   * ``if(typeIndex == typeid(float))``, however, the std::type_index returned
   * by typeid is not guaranteed to be unique across different compilations
   * or executions of the program, so the typeIndex should not be used outside
   * of the scope of a specific execution, i.e., it should not be used for
   * serialization, hashing, or comparison across compilations or executions.
   */
  std::type_index typeIndex = typeid(void);

  /** \brief The base data type for the data block */
  IO::BaseDataType baseDataType;

  /**
   * \brief Default constructor
   */
  DataBlockGeneric() = default;

  /**
   * \brief Parameterized constructor
   */
  DataBlockGeneric(const std::any& inData,
                   const SizeArray& inShape,
                   const std::type_index& inTypeIndex,
                   const IO::BaseDataType inBaseDataType)
      : data(inData)
      , shape(inShape)
      , typeIndex(inTypeIndex)
      , baseDataType(inBaseDataType)
  {
  }

  /**
   * @brief Get the BaseDataType for the data
   *
   * @return The BaseDataType as stored in baseDataType member
   */
  inline BaseDataType getBaseDataType() const { return baseDataType; }

  /**
   * @brief Cast the data to an std::variant for convenient access.
   *
   * This function casts the std::any data to an std::variant via an
   * std::any_cast. This only works for data blocks that store data types
   * defined by BaseDataType. For other types an std::monostate will be returned
   * instead if the data cannot be cast to a BaseDataType::BaseDataVectorVariant
   *
   * @return An std::variant containing the data.
   */
  BaseDataType::BaseDataVectorVariant as_variant() const
  {
    try {
      switch (baseDataType.type) {
        case BaseDataType::T_U8:
          return std::any_cast<std::vector<uint8_t>>(data);
        case BaseDataType::T_U16:
          return std::any_cast<std::vector<uint16_t>>(data);
        case BaseDataType::T_U32:
          return std::any_cast<std::vector<uint32_t>>(data);
        case BaseDataType::T_U64:
          return std::any_cast<std::vector<uint64_t>>(data);
        case BaseDataType::T_I8:
          return std::any_cast<std::vector<int8_t>>(data);
        case BaseDataType::T_I16:
          return std::any_cast<std::vector<int16_t>>(data);
        case BaseDataType::T_I32:
          return std::any_cast<std::vector<int32_t>>(data);
        case BaseDataType::T_I64:
          return std::any_cast<std::vector<int64_t>>(data);
        case BaseDataType::T_F32:
          return std::any_cast<std::vector<float>>(data);
        case BaseDataType::T_F64:
          return std::any_cast<std::vector<double>>(data);
        case BaseDataType::T_STR:
          return std::any_cast<std::vector<std::string>>(data);
        default:
          return std::monostate {};
      }
    } catch (const std::bad_any_cast&) {
      // If the actual type stored in `data` does not match the expected type
      // based on `baseDataType`, a `std::bad_any_cast` exception will be
      // thrown. In this case, we catch the exception and return
      // `std::monostate` to indicate that the conversion failed.
      return std::monostate {};
    }
  }
};

/**
 * @brief Non-owning, multi-dimensional, read-only array view for contiguous
 * data.
 *
 * Provides multi-dimensional access to a flat data buffer using row-major
 * order. This class is used by DataBlock::as_multi_array for C++17/20
 * compatibility.
 *
 * @tparam DTYPE The data type of the array elements.
 * @tparam NDIMS The number of dimensions.
 *
 * @note For C++23 and later, prefer using std::mdspan for multi-dimensional
 * array access.
 * @note This class may be deprecated and removed in future releases in favor of
 * std::mdspan.
 */
template<typename DTYPE, std::size_t NDIMS>
class ConstMultiArrayView
{
public:
  using size_type = std::size_t;

  /**
   * @brief Construct a multi-dimensional view over a contiguous data buffer.
   * @param data Pointer to the data buffer (must remain valid for the lifetime
   * of the view).
   * @param shape Array specifying the size of each dimension.
   * @param strides Array specifying the stride (in elements) for each
   * dimension.
   */
  ConstMultiArrayView(const DTYPE* data,
                      const std::array<size_type, NDIMS>& shape,
                      const std::array<size_type, NDIMS>& strides)
      : m_data(data)
      , m_shape(shape)
      , m_strides(strides)
  {
  }

  /**
   * @brief Access element at the given index for 1D arrays.
   *
   * @param index Index of the element to access.
   * @return Reference to the element at the specified index.
   * @note Only enabled for 1D arrays (NDIMS == 1).
   */
  template<size_type N = NDIMS, typename std::enable_if_t<N == 1, int> = 0>
  const DTYPE& operator[](size_type index) const
  {
    assert(index < m_shape[0]);
    return m_data[index * m_strides[0]];
  }

  /**
   * @brief Access a sub-array view at the given index for multi-dimensional
   * arrays.
   *
   * @param index Index of the sub-array to access (along the first dimension).
   * @return A ConstMultiArrayView of one lower dimension.
   * @note Only enabled for NDIMS > 1.
   */
  template<size_type N = NDIMS, typename std::enable_if_t<(N > 1), int> = 0>
  ConstMultiArrayView<DTYPE, NDIMS - 1> operator[](size_type index) const
  {
    assert(index < m_shape[0]);
    std::array<size_type, NDIMS - 1> sub_shape {};
    std::array<size_type, NDIMS - 1> sub_strides {};
    for (size_type i = 1; i < NDIMS; ++i) {
      sub_shape[i - 1] = m_shape[i];
      sub_strides[i - 1] = m_strides[i];
    }
    return ConstMultiArrayView<DTYPE, NDIMS - 1>(
        m_data + index * m_strides[0], sub_shape, sub_strides);
  }

  /**
   * @brief Returns a pointer to the beginning of the data for 1D arrays.
   *
   * @return Pointer to the first element.
   * @note Only enabled for 1D arrays (NDIMS == 1).
   */
  template<size_type N = NDIMS, typename std::enable_if_t<N == 1, int> = 0>
  const DTYPE* begin() const
  {
    assert(m_shape[0] == 0 || m_data != nullptr);
    return m_data;
  }

  /**
   * @brief Returns a pointer to one past the last element for 1D arrays.
   *
   * @return Pointer to one past the last element.
   * @note Only enabled for 1D arrays (NDIMS == 1).
   */
  template<size_type N = NDIMS, typename std::enable_if_t<N == 1, int> = 0>
  const DTYPE* end() const
  {
    assert(m_shape[0] == 0 || m_data != nullptr);
    if (m_shape[0] == 0) {
      return m_data;
    }
    return m_data + (m_shape[0] * m_strides[0]);
  }

  /**
   * @brief Get the shape of the multi-dimensional array view.
   *
   * @return A const reference to the array specifying the size of each
   * dimension.
   */
  const std::array<size_type, NDIMS>& shape() const { return m_shape; }

private:
  const DTYPE* m_data;  ///< Pointer to the data buffer
  std::array<size_type, NDIMS> m_shape;  ///< Size of each dimension
  std::array<size_type, NDIMS> m_strides;  ///< Stride per dimension
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
  SizeArray shape;
  /**
   * \brief Type index of the values stored in the data vector.
   *        Here this is fixed to ``typeid(DTYPE)``
   */
  const std::type_index typeIndex = typeid(DTYPE);

  /**
   * Constructor
   */
  DataBlock(const std::vector<DTYPE>& inData, const SizeArray& inShape)
      : data(inData)
      , shape(inShape)
  {
  }

  /**
   * \brief Transform the data to a multi-dimensional array view for convenient
   * access.
   *
   * @note For C++23 and later, prefer using std::mdspan for multi-dimensional
   *       array access instead.
   * @note This function may be deprecated and removed in future releases in
   * favor of std::mdspan once C++23 is widely adopted.
   *
   * @tparam NDIMS The number of dimensions of the array. Same as shape.size()
   * @return A ConstMultiArrayView providing multi-dimensional access to the
   */
  template<std::size_t NDIMS>
  inline ConstMultiArrayView<DTYPE, NDIMS> as_multi_array() const
  {
    if (shape.size() != NDIMS) {
      throw std::invalid_argument(
          "Shape size does not match the number of dimensions.");
    }

    // Calculate the total number of elements expected
    SizeType expected_size = std::accumulate(
        shape.begin(), shape.end(), SizeType {1}, std::multiplies<SizeType> {});

    if (data.size() != expected_size) {
      throw std::invalid_argument("Data size does not match the shape.");
    }

    std::array<std::size_t, NDIMS> shape_array {};
    for (std::size_t i = 0; i < NDIMS; ++i) {
      shape_array[i] = static_cast<std::size_t>(shape[i]);
    }

    std::array<std::size_t, NDIMS> strides {};
    std::size_t stride = 1;
    for (std::size_t i = NDIMS; i-- > 0;) {
      strides[i] = stride;
      stride *= shape_array[i];
    }

    return ConstMultiArrayView<DTYPE, NDIMS>(data.data(), shape_array, strides);
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

  /**
   * @brief Get the BaseDataType for the data
   *
   * We here do not store the BaseDataType since it is determined by the
   * template parameter.
   *
   * @return The BaseDataType corresponding to the data type
   * @throws std::runtime_error if the typeIndex does not correspond to a
   * supported data type.
   */
  inline BaseDataType getBaseDataType() const
  {
    return BaseDataType::fromTypeId(typeIndex);
  }
};  // class DataBlock

/// Helper struct to check if a StorageObjectType is allowed. Used in static
/// assert.
template<StorageObjectType T>
struct isAllowedStorageObjectType : std::false_type
{
};

/// Helper struct to check if a StorageObjectType is allowed. Used in static
/// assert.
template<>
struct isAllowedStorageObjectType<StorageObjectType::Dataset> : std::true_type
{
};

/// Helper struct to check if a StorageObjectType is allowed. Used in static
/// assert.
template<>
struct isAllowedStorageObjectType<StorageObjectType::Attribute> : std::true_type
{
};

/**
 * @brief Class for wrapping data objects (datasets or attributes) for reading
 * data from a file
 *
 * @tparam OTYPE The type of object being wrapped defined via \ref
 * AQNWB::Types::StorageObjectType
 * @tparam VTYPE The data type of the values stored in the data object
 */
template<StorageObjectType OTYPE, typename VTYPE = std::any>
class ReadDataWrapper
{
  // Embedded traits for compile time checking of allowed OTYPE for the class
  // and methods
private:
  /**
   * \brief Internal embedded Trait to Check the OTYPE Enum Value at compile
   * time
   *
   * This is used to implement a SFINAE (Substitution Failure Is Not An Error)
   * approach to disable select functions for attributes, to not support
   * slicing.
   */
  template<StorageObjectType U>
  struct isDataset
      : std::integral_constant<bool, (U == StorageObjectType::Dataset)>
  {
  };

  /**
   *  Static assert to enforce the restriction that ReadDataWrapper can only be
   *  instantiated for OTYPE of StorageObjectType::Dataset and
   *  StorageObjectType::Attribute but not for other types, e.g., Group or
   * Undefined.
   */
  static_assert(isAllowedStorageObjectType<OTYPE>::value,
                "StorageObjectType not allowed for ReadDataWrapper");

  // Actual definition of the class
public:
  /**
   * @brief Default constructor.
   *
   * @param io The IO object to use for reading
   * @param path The path to the attribute or dataset to read
   */
  ReadDataWrapper(const std::shared_ptr<IO::BaseIO> io, const std::string& path)
      : m_io(io)
      , m_path(path)
  {
  }

  /**
   * @brief Function to return the \ref AQNWB::Types::StorageObjectType OTYPE of
   * the instance
   */
  static inline StorageObjectType getStorageObjectType() { return OTYPE; }

  /**
   * @brief Function to check at compile-time whether the object is of a
   * particular VTYPE, e.g., ``if constexpr (wrapper.isType<float>())``
   */
  template<typename T>
  static constexpr bool isType()
  {
    return std::is_same_v<VTYPE, T>;
  }

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
   * @brief Gets the path of the registered type.
   * @return The path of the registered type.
   */
  inline const std::string& getPath() const { return m_path; }

  /**
   * @brief Get a shared pointer to the IO object.
   * @return Shared pointer to the IO object.
   */
  inline std::shared_ptr<IO::BaseIO> getIO() const { return m_io; }

  /**
   * @brief Get the shape of the data object.
   * @return The shape of the data object.
   */
  inline SizeArray getShape() const
  {
    return m_io->getStorageObjectShape(m_path);
  }

  /**
   * @brief Get the number of dimensions of the data object
   * @return The number of dimensions of the data object
   */
  inline SizeType getNumDimensions() const { return this->getShape().size(); }

  /**
   * @brief Get the data type of the data object.
   * @return The BaseDataType of the data object.
   * @throws std::runtime_error if the data type cannot be determined.
   */
  inline IO::BaseDataType getDataType() const
  {
    return m_io->getStorageObjectDataType(m_path);
  }

  /**
   * @brief Get the chunking configuration of the data object.
   *
   * Attributes are not chunked, so this will return an empty SizeArray for
   * attributes. An empty SizeArray is also returned if the path does not exist
   * in the file or if the dataset is contiguous (not chunked).
   *
   * @return The chunking configuration of the dataset, or an empty SizeArray
   * if the dataset is not chunked, if this is an attribute, or if the object
   * does not exist.
   */
  inline SizeArray getChunking() const
  {
    return m_io->getStorageObjectChunking(m_path);
  }

  /**
   * @brief Constructs a \ref AQNWB::IO::LinkArrayDataSetConfig from this
   * wrapper.
   *
   * This is useful for creating soft-links to the dataset represented by
   * this wrapper and for querying the storage properties (shape, chunking,
   * data type) of the dataset via the returned config object.
   *
   * We do not support creating links to attributes, so this function is
   * disabled for attributes.
   *
   * @return A \ref AQNWB::IO::LinkArrayDataSetConfig with the path of this
   * wrapper as the link target.
   */
  template<StorageObjectType U = OTYPE,
           typename std::enable_if<isDataset<U>::value, int>::type = 0>
  inline IO::LinkArrayDataSetConfig toLinkArrayDataSetConfig() const
  {
    return IO::LinkArrayDataSetConfig(m_path);
  }

  /**
   * @brief Check that the object exists
   * @return Bool indicating whether the object exists in the file
   */
  inline bool exists() const
  {
    switch (OTYPE) {
      case StorageObjectType::Dataset: {
        return m_io->objectExists(m_path);
      }
      case StorageObjectType::Attribute: {
        return m_io->attributeExists(m_path);
      }
      default: {
        throw std::runtime_error("Unsupported StorageObjectType");
      }
    }
  }

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
        return m_io->readDataset(m_path);
      }
      case StorageObjectType::Attribute: {
        return m_io->readAttribute(m_path);
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
           typename std::enable_if<isDataset<U>::value, int>::type = 0>
  inline DataBlockGeneric valuesGeneric(const SizeArray& start,
                                        const SizeArray& count = {},
                                        const SizeArray& stride = {},
                                        const SizeArray& block = {}) const
  {
    // The function is only enabled for datasets so we don't need to check
    // for attributes here.
    return m_io->readDataset(m_path, start, count, stride, block);
  }

  /**
   * @brief Reads an attribute with a specified data type.
   *
   * This convenience function uses valuesGeneric to read the data and then
   * convert the DataBlockGeneric to a specific DataBlock
   *
   * @tparam T the value type to use. By default this is set to the VTYPE
   *           of the object but is added here to allow the user to
   *           request a different type if appropriate, e.g., if the
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
           typename std::enable_if<isDataset<U>::value, int>::type = 0>
  inline DataBlock<VTYPE> values(const SizeArray& start,
                                 const SizeArray& count = {},
                                 const SizeArray& stride = {},
                                 const SizeArray& block = {}) const
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
  const std::shared_ptr<IO::BaseIO> m_io;
  /**
   * @brief Path to the dataset or attribute to read
   */
  std::string m_path;
};  // ReadDataWrapper

}  // namespace AQNWB::IO
