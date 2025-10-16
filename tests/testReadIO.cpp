#include <any>
#include <numeric>
#include <stdexcept>
#include <typeindex>
#include <variant>
#include <vector>

#include <boost/multi_array.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/ReadIO.hpp"

using namespace AQNWB::IO;

// Helper function to compute the mean of a vector
template<typename T>
inline double test_compute_mean(const T& data)
{
  if (data.empty()) {
    throw std::runtime_error("Data vector is empty");
  }
  double sum = std::accumulate(data.begin(), data.end(), 0.0);
  return sum / static_cast<double>(data.size());
}

// Function to compute the mean using std::visit
inline double test_compute_mean(
    const BaseDataType::BaseDataVectorVariant& variant)
{
  return std::visit(
      [](auto&& arg) -> double
      {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          throw std::runtime_error("Invalid data type");
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
          throw std::runtime_error("Cannot compute mean of string data");
        } else {
          return test_compute_mean(arg);
        }
      },
      variant);
}

// Templated function to test variant conversion
template<typename T>
inline void test_variant_conversion(const std::vector<T>& data,
                                    BaseDataType::Type baseType)
{
  std::vector<SizeType> shape = {data.size()};
  BaseDataType baseDataType = {baseType};

  DataBlockGeneric genericBlock(std::any(data), shape, typeid(T), baseDataType);

  auto variant = genericBlock.as_variant();
  REQUIRE(std::holds_alternative<std::vector<T>>(variant));
  REQUIRE(std::get<std::vector<T>>(variant) == data);
}

TEST_CASE("DataBlock - Basic Functionality", "[DataBlock]")
{
  SECTION("Constructor and Accessors")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<SizeType> shape = {5};

    DataBlock<int> block(data, shape);

    REQUIRE(block.data == data);
    REQUIRE(block.shape == shape);
    REQUIRE(block.typeIndex == typeid(int));
  }

  SECTION("Modification")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<SizeType> shape = {5};

    DataBlock<int> block(data, shape);
    block.data[2] = 10;

    REQUIRE(block.data[2] == 10);
  }

  SECTION("From Generic")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<SizeType> shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlock<int> block(data, shape);
    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto newBlock = DataBlock<int>::fromGeneric(genericBlock);

    REQUIRE(newBlock.data == block.data);
    REQUIRE(newBlock.shape == block.shape);
  }
}

TEST_CASE("DataBlockGeneric - Basic Functionality", "[DataBlockGeneric]")
{
  SECTION("Constructor and Accessors")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<SizeType> shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlock<int> block(data, shape);
    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    REQUIRE(genericBlock.shape == shape);
    REQUIRE(genericBlock.typeIndex == typeid(int));
    REQUIRE(std::any_cast<std::vector<int>>(genericBlock.data) == data);

    auto newBlock = DataBlock<int>::fromGeneric(genericBlock);
    REQUIRE(newBlock.data == data);
    REQUIRE(newBlock.shape == shape);
  }
}

TEST_CASE("DataBlock - Edge Cases", "[DataBlock]")
{
  SECTION("Empty Data Block")
  {
    std::vector<int> data;
    std::vector<SizeType> shape;

    DataBlock<int> block(data, shape);

    REQUIRE(block.data.empty());
    REQUIRE(block.shape.empty());
  }

  SECTION("Multi-dimensional Data Block")
  {
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    std::vector<SizeType> shape = {2, 3};

    DataBlock<int> block(data, shape);

    REQUIRE(block.data == data);
    REQUIRE(block.shape == shape);
  }
}

TEST_CASE("DataBlockGeneric - as_variant Method", "[DataBlockGeneric]")
{
  SECTION("Variant Conversion")
  {
    test_variant_conversion<uint8_t>({1, 2, 3, 4, 5}, BaseDataType::T_U8);
    test_variant_conversion<uint16_t>({1, 2, 3, 4, 5}, BaseDataType::T_U16);
    test_variant_conversion<uint32_t>({1, 2, 3, 4, 5}, BaseDataType::T_U32);
    test_variant_conversion<uint64_t>({1, 2, 3, 4, 5}, BaseDataType::T_U64);
    test_variant_conversion<int8_t>({1, 2, 3, 4, 5}, BaseDataType::T_I8);
    test_variant_conversion<int16_t>({1, 2, 3, 4, 5}, BaseDataType::T_I16);
    test_variant_conversion<int32_t>({1, 2, 3, 4, 5}, BaseDataType::T_I32);
    test_variant_conversion<int64_t>({1, 2, 3, 4, 5}, BaseDataType::T_I64);
    test_variant_conversion<float>({1.0f, 2.0f, 3.0f, 4.0f, 5.0f},
                                   BaseDataType::T_F32);
    test_variant_conversion<double>({1.0, 2.0, 3.0, 4.0, 5.0},
                                    BaseDataType::T_F64);
    test_variant_conversion<std::string>({"a", "b", "c"}, BaseDataType::T_STR);
  }

  SECTION("Unsupported Type")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<SizeType> shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_STR};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto variant = genericBlock.as_variant();
    REQUIRE(std::holds_alternative<std::monostate>(variant));
  }
}

TEST_CASE("DataBlockGeneric - Compute Mean with std::visit",
          "[DataBlockGeneric]")
{
  SECTION("Compute Mean for Integer Data")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<SizeType> shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto variant = genericBlock.as_variant();

    double mean = test_compute_mean(variant);

    REQUIRE(mean == Catch::Approx(3.0));
  }

  SECTION("Compute Mean for Float Data")
  {
    std::vector<float> data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<SizeType> shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_F32};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(float), baseDataType);

    auto variant = genericBlock.as_variant();

    double mean = test_compute_mean(variant);

    REQUIRE(mean == Catch::Approx(3.0));
  }

  SECTION("Compute Mean for Empty Data")
  {
    std::vector<int> data;
    std::vector<SizeType> shape = {0};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto variant = genericBlock.as_variant();

    REQUIRE_THROWS_AS(test_compute_mean(variant), std::runtime_error);
  }

  SECTION("Compute Mean for String Data")
  {
    std::vector<std::string> data = {"a", "b", "c"};
    std::vector<SizeType> shape = {3};
    BaseDataType baseDataType = {BaseDataType::T_STR};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(std::string), baseDataType);

    auto variant = genericBlock.as_variant();

    REQUIRE_THROWS_AS(test_compute_mean(variant), std::runtime_error);
  }
}
