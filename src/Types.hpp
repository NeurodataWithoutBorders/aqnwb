#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace AQNWB
{

// Forward declaration of Channel
class Channel;

/**
 * @brief Type trait to check if a type is a std::vector.
 * Used to enable/disable constructors in CellValue based on whether the input
 * is a scalar or a vector.
 */
template<typename T>
struct is_vector : std::false_type
{
};
template<typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type
{
};

/**
 * @brief Provides definitions for various types used in the project.
 */
namespace Types
{
/**
 * @brief Represents the status of an operation.
 */
enum Status
{
  Success = 1,
  Failure = -1
};

/**
 * @brief Overloaded && operator for Status enum 
 * @param lhs Left-hand side Status
 * @param rhs Right-hand side Status
 * @return Success if both statuses are Success, Failure otherwise
 */
inline Status operator&&(Status lhs, Status rhs)
{
  return (lhs == Success && rhs == Success) ? Success : Failure;
}

/**
 * @brief Overloaded || operator for Status enum
 * @param lhs Left-hand side Status
 * @param rhs Right-hand side Status
 * @return Success if either status is Success, Failure otherwise
 */
inline Status operator||(Status lhs, Status rhs)
{
  return (lhs == Success || rhs == Success) ? Success : Failure;
}

/**
 * @brief Types of object used in the NWB schema
 */
enum StorageObjectType
{
  Group = 0,
  Dataset = 1,
  Attribute = 2,
  Undefined = -1
};

/**
 * @brief Convert StorageObjectType enum value to string
 * @param type The StorageObjectType value to convert
 * @return String representation of the StorageObjectType
 */
inline std::string storageObjectTypeToString(StorageObjectType type)
{
  switch (type) {
    case Group:
      return "Group";
    case Dataset:
      return "Dataset";
    case Attribute:
      return "Attribute";
    case Undefined:
      return "Undefined";
    default:
      return "Unknown";
  }
}

/**
 *  \brief Helper struct to check if a value is a data field, i.e.,
 * Dataset or Attribute
 *
 * This function is used to enforce constraints on templated functions that
 * should only be callable for valid StorageObjectType values
 */
template<StorageObjectType T>
struct IsDataStorageObjectType
    : std::integral_constant<bool, (T == Dataset || T == Attribute)>
{
};

/**
 * @brief Alias for the size type used in the project.
 */
using SizeType = size_t;

/**
 * @brief Value to use to indicate that a SizeType index is not set.
 */
static constexpr SizeType SizeTypeNotSet =
    (std::numeric_limits<SizeType>::max)();

/**
 * @brief Alias for an array of size types used in the project.
 */
using SizeArray = std::vector<SizeType>;

/**
 * @brief Alias for a vector of channels.
 */
using ChannelVector = std::vector<Channel>;

/**
 * @brief Variant data type for representing a single scalar value.
 */
using ScalarDataVariant = std::variant<uint8_t,
                                       uint16_t,
                                       uint32_t,
                                       uint64_t,
                                       int8_t,
                                       int16_t,
                                       int32_t,
                                       int64_t,
                                       float,
                                       double,
                                       std::string>;

/**
 * @brief Variant data type for representing any 1D vector of scalar values.
 */
using VectorDataVariant = std::variant<std::monostate,
                                       std::vector<uint8_t>,
                                       std::vector<uint16_t>,
                                       std::vector<uint32_t>,
                                       std::vector<uint64_t>,
                                       std::vector<int8_t>,
                                       std::vector<int16_t>,
                                       std::vector<int32_t>,
                                       std::vector<int64_t>,
                                       std::vector<float>,
                                       std::vector<double>,
                                       std::vector<std::string>>;

/**
 * @brief Struct to hold namespace information.
 */
struct NamespaceInfo
{
  std::string name;  ///< The name of the namespace.
  std::string version;  ///< The version of the namespace.

  /** @brief The specVariables of the namespace.
   *
   * This is a vector of pairs, where each pair consists of 1) the
   * name of the specification filed (e.g., "nwb.base") and 2) the
   * string with the JSON specification of the format schema.
   **/
  std::vector<std::pair<std::string_view, std::string_view>>
      specVariables;  ///< The specVariables of the namespace.
};

/**
 * @brief Represents a single cell value in a DynamicTable row.
 *
 * A cell can hold either a scalar value (for regular columns) or a vector of
 * values (for ragged array columns). This struct wraps a std::variant of both
 * types and provides templated constructors to allow implicit conversion from
 * both scalar and vector types. This enables clean syntax like `RowData row =
 * {{"col1", 5}, {"col2", std::vector<int>{1, 2}}}`.
 *
 * Note: The constructors for this struct are intentionally implicit (not marked
 * `explicit`) to allow for this clean initializer list syntax. The cppcheck
 * warnings for `noExplicitConstructor` are suppressed because this implicit
 * conversion is a deliberate design choice for the API. Otherwise, users would
 * have to explicitly wrap values in `CellValue` when constructing rows, via
 * `RowData row = {{"col1", CellValue(5)}, {"col2",
 * CellValue(std::vector<int>{1, 2}))}`, which would be cumbersome.
 */
struct CellValue
{
  std::variant<ScalarDataVariant, VectorDataVariant> value;

  CellValue() = default;

  /**
   * @brief Implicit constructor for scalar values.
   *
   * This constructor is enabled only if the input type T is NOT a std::vector
   * and is NOT a CellValue itself (to prevent hiding the copy/move
   * constructors). It forwards the value to the ScalarDataVariant.
   */
  template<typename T,
           typename =
               std::enable_if_t<!is_vector<std::decay_t<T>>::value
                                && !std::is_same_v<std::decay_t<T>, CellValue>>>
  // cppcheck-suppress noExplicitConstructor
  CellValue(T&& val)
      : value(ScalarDataVariant(std::forward<T>(val)))
  {
  }

  /**
   * @brief Implicit constructor for vector values.
   *
   * This constructor is enabled only if the input type T IS a std::vector.
   * It forwards the vector to the VectorDataVariant.
   */
  template<typename T,
           typename = std::enable_if_t<is_vector<std::decay_t<T>>::value>,
           typename = void>
  // cppcheck-suppress noExplicitConstructor
  CellValue(T&& val)
      : value(VectorDataVariant(std::forward<T>(val)))
  {
  }

  /**
   * @brief Implicit constructor for string literals.
   *
   * Required because string literals (const char*) would otherwise match the
   * scalar template but fail to implicitly convert to std::string inside the
   * variant.
   */
  // cppcheck-suppress noExplicitConstructor
  CellValue(const char* val)
      : value(ScalarDataVariant(std::string(val)))
  {
  }

  /**
   * @brief Helper method to extract the underlying value.
   *
   * @tparam T The expected type of the value.
   * @return The value of type T.
   * @throws std::bad_variant_access if the requested type does not match the
   * stored type.
   */
  template<typename T>
  T get() const
  {
    if constexpr (is_vector<T>::value) {
      return std::get<T>(std::get<VectorDataVariant>(value));
    } else {
      return std::get<T>(std::get<ScalarDataVariant>(value));
    }
  }

  /**
   * @brief Implicit conversion operator to extract the underlying value.
   *
   * @tparam T The expected type of the value.
   * @return The value of type T.
   * @throws std::bad_variant_access if the requested type does not match the
   * stored type.
   */
  template<typename T>
  operator T() const
  {
    return get<T>();
  }

  /**
   * @brief Convert the cell value to a string representation.
   * @return A string representation of the cell value.
   */
  std::string toString() const
  {
    return std::visit(
        [](auto&& arg) -> std::string
        {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, ScalarDataVariant>) {
            return std::visit(
                [](auto&& scalarArg) -> std::string
                {
                  using ScalarT = std::decay_t<decltype(scalarArg)>;
                  if constexpr (std::is_same_v<ScalarT, std::string>) {
                    return scalarArg;
                  } else {
                    return std::to_string(scalarArg);
                  }
                },
                arg);
          } else if constexpr (std::is_same_v<T, VectorDataVariant>) {
            return std::visit(
                [](auto&& vectorArg) -> std::string
                {
                  using VectorT = std::decay_t<decltype(vectorArg)>;
                  if constexpr (std::is_same_v<VectorT, std::monostate>) {
                    return "[]";
                  } else {
                    std::string res = "[";
                    for (size_t i = 0; i < vectorArg.size(); ++i) {
                      if constexpr (std::is_same_v<VectorT,
                                                   std::vector<std::string>>) {
                        res += vectorArg[i];
                      } else {
                        res += std::to_string(vectorArg[i]);
                      }
                      if (i < vectorArg.size() - 1) {
                        res += ", ";
                      }
                    }
                    res += "]";
                    return res;
                  }
                },
                arg);
          } else {
            return "Unknown";
          }
        },
        value);
  }
};

/**
 * @brief Represents a row of data in a DynamicTable.
 *
 * A row is a map from column names to cell values.
 */
using RowData = std::unordered_map<std::string, CellValue>;

}  // namespace Types
}  // namespace AQNWB
